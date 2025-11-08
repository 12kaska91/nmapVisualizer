#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "./utils.hpp"
#include <giomm.h>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/notebook.h>
#include <gtkmm/paned.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/gesturedrag.h>
#include <gtkmm/gesturezoom.h>
#include <gtkmm/gestureclick.h>
#include <gdk/gdk.h>

/* 
LAYOUT
FILE | SCAN OPTIONS     | GOBUTTON  | ENTER IP TEXT FIELD
------------------------------------------------------------
LIST OF NETWORK TABS                | ATTRIBUTES
------------------------------------|
DEVICE MAP                          |
------------------------------------------------------------
*/

class MapArea : public Gtk::DrawingArea {
public:
    MapArea() {
        // Enable gestures
        add_controller(drag_gesture_);
        add_controller(zoom_gesture_);
        add_controller(click_gesture_);
    }

    private:
        Glib::RefPtr<Gtk::GestureDrag> drag_gesture_ = Gtk::GestureDrag::create();
        Glib::RefPtr<Gtk::GestureZoom> zoom_gesture_ = Gtk::GestureZoom::create();
        Glib::RefPtr<Gtk::GestureClick> click_gesture_ = Gtk::GestureClick::create();

    void on_drag_updated(double offset_x, double offset_y) {
        // Handle drag updates
    }

    void on_zoom_updated(double scale) {
        // Handle zoom updates
    }

    void on_clicked(double x, double y) {
        // Handle clicks
    }
};

class MainWindow : public Gtk::Window {
public:
    MainWindow() {
        set_title("nmap Visualizer");
        set_default_size(300, 100);

        // create top bar elements
        auto file = Gtk::make_managed<Gtk::MenuButton>();
        auto fileMenu = Gio::Menu::create();
        auto scanOptions = Gtk::make_managed<Gtk::MenuButton>();
        auto scanOptionsMenu = Gio::Menu::create();
        auto go_button = Gtk::make_managed<Gtk::Button>("Go");
        ip_entry_ = Gtk::make_managed<Gtk::Entry>();

        // create main layout
        auto main_paned = Gtk::make_managed<Gtk::Paned>(Gtk::Orientation::HORIZONTAL);
        auto notebook = Gtk::make_managed<Gtk::Notebook>();
        auto device_map_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5); // placeholder for device map
        auto map_area = Gtk::make_managed<MapArea>();
        // create boxes
        auto top_hbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);  
        auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);

        // attributes panel
        auto attrs_frame = Gtk::make_managed<Gtk::Frame>("Attributes");
        auto attrs_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);


        /*
        #########################
        ##       TOP BAR       ##
        #########################
        */

        // initialize file menu
        file->set_label("files");

        fileMenu->append("Say Hello", "app.hello");
        fileMenu->append("Quit", "app.quit");

        file->set_menu_model(fileMenu);

        // initialize scan options menu
        scanOptions->set_label("Scan Options");

        scanOptionsMenu->append("Say Hello", "app.hello");
        scanOptionsMenu->append("Quit", "app.quit");

        scanOptions->set_menu_model(scanOptionsMenu);

        // initialize go button
        go_button->set_tooltip_text("Start nmap scan");
        
        go_button->signal_clicked().connect([this]() {
            if (auto app = get_application()) {
                app->activate_action("go_button");
            }
        });

        // ip entry initialization
        
        ip_entry_->set_placeholder_text("Enter IP address");

        // set up top_hbox
        top_hbox->append(*file);
        top_hbox->append(*scanOptions);
        top_hbox->append(*go_button);
        top_hbox->append(*ip_entry_);

        /*
        ###########################
        ##       MAIN AREA       ##
        ###########################
        */

        // devices
        device_map_box->append(*map_area);
        notebook->append_page(*map_area, "Network 1");
        main_paned->set_start_child(*device_map_box);
        main_paned->set_start_child(*notebook);

        // attributes
        attrs_box->append(*Gtk::make_managed<Gtk::Label>("No selection"));
        attrs_frame->set_child(*attrs_box);
        main_paned->set_end_child(*attrs_frame);

        /*
        #########################
        ##      SET SIZES      ##
        #########################
        */

        main_paned->set_vexpand(true);
        main_paned->set_valign(Gtk::Align::FILL);

        notebook->set_vexpand(true);
        attrs_frame->set_vexpand(true);
        attrs_frame->set_valign(Gtk::Align::FILL);

        ip_entry_->set_halign(Gtk::Align::FILL);
        ip_entry_->set_hexpand(true);

        /*
        ########################
        ##      FINALIZE      ##
        ########################     
        */
        set_child(*vbox);
        vbox->append(*top_hbox);
        vbox->append(*main_paned);

        show();
    }

    // getters
    std::string get_ip_entry_text() const {
        return ip_entry_ ? ip_entry_->get_text() : "";
    }

private:
    Gtk::Entry* ip_entry_ = nullptr;
};

class nmapVisualizer : public Gtk::Application {
protected:
    nmapVisualizer() : Gtk::Application("org.kaska.nmapVisualizer") {}

    void on_startup() override {
        Gtk::Application::on_startup();

        // Define actions
        add_action("hello", sigc::mem_fun(*this, &nmapVisualizer::on_hello));
        add_action("quit", sigc::mem_fun(*this, &nmapVisualizer::on_quit));
        add_action("go_button", sigc::mem_fun(*this, &nmapVisualizer::on_go_button_clicked));
    }

    void on_activate() override {
        // create_window now returns a raw pointer and the app owns the window
        MainWindow* win = create_window();
        if (win) win->present();
    }

    // return raw pointer; application keeps ownership via add_window()
    MainWindow* create_window() {
        MainWindow* window = new MainWindow();
        window->set_resizable(true);
        add_window(*window);
        return window;
    }

    // Action handlers
    void on_hello() {
        std::cout << "Hello from the menu!" << std::endl;
    }

    void on_quit() {
        std::cout << "Quitting..." << std::endl;
        quit();
    }

    void on_go_button_clicked() {
        std::string target;
        if (auto win = dynamic_cast<MainWindow*>(get_active_window())) {
            target = win->get_ip_entry_text();
        }

        std::cout << "Running nmap scan on target: " << target << std::endl;

        std::string nmapOutput;
        nmapOutput = run_nmap(target, "");

        auto devices = parse_nmap_xml(nmapOutput);
        for (const auto& device : devices) {
            std::cout << "Device IP: " << device.ipAddress << ", MAC: " << device.macAddress << ", OS: " << device.operatingSystem << std::endl;
        }
    }

public:
    static Glib::RefPtr<nmapVisualizer> create() {
        return Glib::RefPtr<nmapVisualizer>(new nmapVisualizer());
    }
};

#endif // GRAPHICS_HPP