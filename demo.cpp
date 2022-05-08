#include <gtkmm.h>

extern "C"
void on_quit_btn_clicked(){
    Gtk::Main::quit();
}

Glib::RefPtr<Gtk::Builder> builder;

extern "C"
void on_app_activate(int argc, char** argv, char* gmodule){
    Gtk::Main app = Gtk::Main(argc, argv, false);

    builder = Gtk::Builder::create();
    builder->add_from_file("demo.ui");

    gtk_builder_connect_signals(builder->gobj(), NULL);

    Gtk::Window* main_win;
    builder->get_widget<Gtk::Window>("main_win", main_win);

    app.run(*main_win);
}

int main(int argc, char** argv ) {
    on_app_activate(argc, argv, NULL);
}
