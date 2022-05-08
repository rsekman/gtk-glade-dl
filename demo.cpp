#include <gtkmm.h>

extern "C"
void on_quit_btn_clicked(){
    Gtk::Main::quit();
}

Glib::RefPtr<Gtk::Builder> builder;

typedef struct {
    GModule* gmodule;
    gpointer data;
} connect_args;

static void
gtk_builder_connect_signals_default (GtkBuilder    *builder,
        GObject       *object,
        const gchar   *signal_name,
        const gchar   *handler_name,
        GObject       *connect_object,
        GConnectFlags  flags,
        gpointer       user_data)
{
    GCallback func;
    connect_args *args = (connect_args*)user_data;

    if (!g_module_symbol (args->gmodule, handler_name, (gpointer*)&func) )
    {
        g_warning ("Could not find signal handler '%s'", handler_name);
        return;
    }

    if (connect_object)
        g_signal_connect_object (object, signal_name, func, connect_object, flags);
    else
        g_signal_connect_data (object, signal_name, func, args->data, NULL, flags);
}

extern "C"
void on_app_activate(int argc, char** argv, char* gmodule){
    Gtk::Main app = Gtk::Main(argc, argv, false);
    builder = Gtk::Builder::create();
    builder->add_from_file("demo.ui");
    connect_args* args;
    args = g_slice_new0 (connect_args);
    args->gmodule = g_module_open (gmodule, G_MODULE_BIND_LAZY);
    args->data = NULL;

    gtk_builder_connect_signals_full (builder->gobj(),
            gtk_builder_connect_signals_default,
            args);
    g_module_close (args->gmodule);

    g_slice_free (connect_args, args);

    Gtk::Window* main_win;
    builder->get_widget<Gtk::Window>("main_win", main_win);

    app.run(*main_win);
}

int main(int argc, char** argv ) {
    on_app_activate(argc, argv, NULL);
}
