#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "./utils.hpp"
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/box.h>

class MyWindow : public Gtk::Window {
public:
    MyWindow() {
        set_title("nmap Visualizer");
        set_default_size(300, 100);

        // Create a box layout
        auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
        set_child(*vbox);

        // Create a MenuButton
        auto menubutton = Gtk::make_managed<Gtk::MenuButton>();
        menubutton->set_label("Options");

        // Create a Gio::Menu
        auto menu = Gio::Menu::create();
        menu->append("Say Hello", "app.hello");
        menu->append("Quit", "app.quit");

        // Attach the menu to the button
        menubutton->set_menu_model(menu);

        // Add the button to the layout
        vbox->append(*menubutton);
    }
};

class MyApplication : public Gtk::Application {
protected:
    MyApplication() : Gtk::Application("org.kaska.nmapVisualizer") {}

    void on_startup() override {
        Gtk::Application::on_startup();

        // Define actions
        add_action("hello", sigc::mem_fun(*this, &MyApplication::on_hello));
        add_action("quit", sigc::mem_fun(*this, &MyApplication::on_quit));
    }

    void on_activate() override {
        auto win = create_window();
        win->present();
    }

    Glib::RefPtr<Gtk::Window> create_window() {
        auto window = new MyWindow();
        add_window(*window);
        return Glib::RefPtr<Gtk::Window>(window);
    }

    // Action handlers
    void on_hello() {
        std::cout << "Hello from the menu!" << std::endl;
    }

    void on_quit() {
        std::cout << "Quitting..." << std::endl;
        quit();
    }

public:
    static Glib::RefPtr<MyApplication> create() {
        return Glib::RefPtr<MyApplication>(new MyApplication());
    }
};

#endif // GRAPHICS_HPP