# gtk-glade-dl

Consider the following two programs, glossing over what compiler flags we need to get them running. (See the `Makefile`).
https://github.com/rsekman/gtk-glade-dl/blob/c53be75926d6505898d95a794e2fa46530cdddf3/demo.cpp#L1-L27
https://github.com/rsekman/gtk-glade-dl/blob/c53be75926d6505898d95a794e2fa46530cdddf3/dl.c#L6-L19

On their face, the programs should have the same behaviour: both the entry point of `demo` and `dl` call `on_app_activate`.
What we expect to happen is a window to open, containing a button we can click to quit the program. `demo` works as expected.
On running `dl`, however, we are faced with a warning from Gtk
```
(dl:2875124): Gtk-WARNING **: 14:19:58.729: Could not find signal handler 'on_quit_btn_clicked'
```
and indeed clicking the button has no effect.

On the internet, many people have had issues compiling and linking against Gtk+.
We have to use the right compiler flags (from `pkg-config` and `-rdynamic -export-dynamic`).
We have to flag the callback with `extern "C"` to avoid name mangling.
The issue here is not one of these.

To understand what's wrong, let's look the [documentation for `gtk_builder_connect_signals`](https://www.csparks.com/gtk2-html-2.24.33/GtkBuilder.html#gtk-builder-connect-signals):
> This method is a simpler variation of `gtk_builder_connect_signals_full()`. It uses GModule's introspective features (by opening the module `NULL`) to look at the application's symbol table. From here it tries to match the signal handler names given in the interface description with symbols in the application and connects the signals.

The issue is that when called from `dl`, GModule tries to find `on_quit_btn_clicked` in the symbol table of `dl`.
But `on_quit_button_clicked` is in `demo.so`, and we have to tell GModule to look there.
Can we do that? The [source for `gtk_builder_connect_signals()`](https://gitlab.gnome.org/GNOME/gtk/-/blob/gtk-2-24/gtk/gtkbuilder.c#L1019) contains these important lines
```C
  args = g_slice_new0 (connect_args);
  args->module = g_module_open (NULL, G_MODULE_BIND_LAZY);
  args->data = user_data;
  
  gtk_builder_connect_signals_full (builder,
                                    gtk_builder_connect_signals_default,
                                    args);
```
It looks like we just need to change out the `NULL` in the call to `g_module_open`.
The [documentation](https://docs.gtk.org/gmodule/type_func.Module.open.html) tells us we can just pass the file name as a C string.

We change the signature of `on_app_activate` so that we can pass in argument that tells us if we're in the main program, or being dynamically loaded.
If this argument is a `char*` it can be `NULL` in the former case and double as the name of the `.so` in the latter.
https://github.com/rsekman/gtk-glade-dl/blob/0f5df9b8e94742ed68fda21141e7d60317bef41f/demo.cpp#L40-L51
Now, `gtk_builder_connect_signals_default` is not exported in `gtkbuilder.h`, so we have to provide the implementation, i.e. [copy-paste from source](https://gitlab.gnome.org/GNOME/gtk/-/blob/gtk-2-24/gtk/gtkbuilder.c#L972).
https://github.com/rsekman/gtk-glade-dl/blob/0f5df9b8e94742ed68fda21141e7d60317bef41f/demo.cpp#L15-L16

All that remains now is modifying `dl.c` to account for the new signature.
https://github.com/rsekman/gtk-glade-dl/blob/0f5df9b8e94742ed68fda21141e7d60317bef41f/dl.c#L6
https://github.com/rsekman/gtk-glade-dl/blob/0f5df9b8e94742ed68fda21141e7d60317bef41f/dl.c#L17

After recompiling, `dl` now works: clicking the quit button does, in fact, quit.

Instead of passing the filename as a string, we can deduce it through introspection.
With the `backtrace()` and `backtrace_symbols()` functions from `execinfo.h` (see [backtrace(3) man page](https://man7.org/linux/man-pages/man3/backtrace.3.html)), we can obtain backtraces that look something like
```
./demo.so(on_app_activate+0x202) [0x7fc6c66c1752]
```
The first stack frame from `backtrace()` must be in the file containing `on_app_activate`.
We can then just cut out the file name, and pass that to `gmodule`
https://github.com/rsekman/gtk-glade-dl/blob/0e73a2db7278369ed6cc4752d693d38cf472f6d6/demo.cpp#L45-L50
(Of course, we should really use something more sophisticated, like a regular expression, for this in case the file name could contain `(`.)

For some reason that I do not understand, when loading dynamically the leading `./` must be present;
but when running as the main program, it must be *absent*.
Therefore the third argument to `on_app_activate` is not dropped, but changed to a flag
https://github.com/rsekman/gtk-glade-dl/blob/0e73a2db7278369ed6cc4752d693d38cf472f6d6/demo.cpp#L41-L44
so that we can still pass `NULL` if running as the main program.
