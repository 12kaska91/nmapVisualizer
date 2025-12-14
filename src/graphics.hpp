#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "./utils.hpp"
#include "cairomm/fontface.h"
#include <gtkmm.h>

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
        set_content_width(600);
        set_content_height(400);

        gesture_click = Gtk::GestureClick::create();
        add_controller(gesture_click);
        gesture_click->signal_pressed().connect(sigc::mem_fun(*this, &MapArea::on_click));
        signal_resize().connect([this](int width, int height){
            for (auto& d : devices) {
                int num = devices.size();
                double angle = (&d - &devices[0]) * (2 * M_PI / num);
                d.x = width / 2.0 + (width / 3.0) * cos(angle);
                d.y = height / 2.0 + (height / 3.0) * sin(angle);
            }
        });

        set_draw_func(sigc::mem_fun(*this, &MapArea::draw_map));
    }

private:
    Glib::RefPtr<Gtk::GestureClick> gesture_click;

    struct Device {
        std::string name;
        double x, y;
        Gdk::RGBA color;
    };

    struct Network {
        std::string cidr;
        std::vector<Device> devices;
    };

    std::vector<Device> devices;
    std::vector<std::pair<int,int>> connections;

    void on_click(int /*n_press*/, double x, double y) {
        for (auto& d : devices) {
            double dx = x - d.x;
            double dy = y - d.y;
            if (dx*dx + dy*dy <= 20*20) {
                std::cout << "Clicked device: " << d.name << std::endl;
            }
        }
    }

    void draw_map(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
        // Clear background
        cr->set_source_rgb(0.1, 0.1, 0.1);
        cr->rectangle(0, 0, width, height);
        cr->fill();

        if(devices.empty()) {
            Gdk::RGBA color;
            color.set_rgba(0.0, 0.6, 1.0, 1.0);
            devices.push_back({"Test Device", width / 2.0, height / 2.0, color});
            // Add more devices for demonstration
            devices.push_back({"Device 2", width / 4.0, height / 4.0, Gdk::RGBA(1.0, 0.0, 0.0, 1.0)});
            devices.push_back({"Device 3", 3 * width / 4.0, height / 4.0, Gdk::RGBA(0.0, 1.0, 0.0, 1.0)});
            devices.push_back({"Device 4", width / 4.0, 3 * height / 4.0, Gdk::RGBA(0.0, 0.0, 1.0, 1.0)});
            devices.push_back({"Device 5", 3 * width / 4.0, 3 * height / 4.0, Gdk::RGBA(1.0, 1.0, 0.0, 1.0)});
            devices.push_back({"Device 6", width / 2.0, height / 4.0, Gdk::RGBA(1.0, 0.0, 1.0, 1.0)});
            devices.push_back({"Device 7", width / 2.0, 3 * height / 4.0, Gdk::RGBA(0.0, 1.0, 1.0, 1.0)});
        }

        for (auto& d : devices) {
            int num = devices.size();
            double angle = (&d - &devices[0]) * (2 * M_PI / num);
            d.x = width / 2.0 + (width / 3.0) * cos(angle);
            d.y = height / 2.0 + (height / 3.0) * sin(angle);

        }

        // Draw connections
        cr->set_line_width(2.0);
        cr->set_source_rgb(0.7, 0.7, 0.7);
        for (auto& d : devices) {
            cr->move_to(d.x, d.y);
            cr->line_to(width/2, height/2);
            cr->stroke();
        }

        // Draw devices
        for (auto& d : devices) {
            // circle
            cr->set_source_rgb(255, 255, 255);
            cr->arc(d.x, d.y, 20.0, 0, 2*M_PI);
            cr->fill();

            // font
            cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::NORMAL);
            cr->set_font_size(10.0);

            // extents
            Cairo::TextExtents extents;
            cr->get_text_extents(d.name, extents);

            // label shadow
            cr->set_source_rgba(0, 0, 0, 0.5);
            cr->move_to(d.x - extents.width / 2 + 1, d.y + 31);
            cr->show_text(d.name);

            // text
            cr->set_source_rgb(1.0, 1.0, 1.0);
            cr->move_to(d.x - extents.width / 2, d.y + 30);
            cr->show_text(d.name);
        }


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
            auto go_button = Gtk::make_managed<Gtk::Button>("Scan");
            ip_entry_ = Gtk::make_managed<Gtk::Entry>();

            // create main layout
            auto main_paned = Gtk::make_managed<Gtk::Paned>(Gtk::Orientation::HORIZONTAL);
            auto map_area = Gtk::make_managed<MapArea>();
            // create boxes
            auto top_hbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10); 
            auto top_hbox_left = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
            auto top_hbox_right = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
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
            file->set_label("File");

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
            top_hbox_left->append(*file);
            top_hbox_left->append(*scanOptions);
            top_hbox_right->set_halign(Gtk::Align::END);
            top_hbox_right->append(*go_button);
            top_hbox_right->append(*ip_entry_);
            top_hbox->append(*top_hbox_left);
            top_hbox->append(*top_hbox_right);

            /*
            ###########################
            ##       MAIN AREA       ##
            ###########################
            */

            // devices
            // device_map_box->append(*map_area);
            main_paned->set_start_child(*map_area);

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

            attrs_frame->set_vexpand(true);
            attrs_frame->set_valign(Gtk::Align::FILL);

            map_area->set_hexpand(true);
            map_area->set_vexpand(true);

            ip_entry_->set_halign(Gtk::Align::FILL);
            ip_entry_->set_hexpand(true);
            
            // make default size of ip_entry larger
            ip_entry_->set_size_request(300, -1);

            // make space between top bar and main area 0 
            vbox->set_spacing(0);
            top_hbox_left->set_spacing(0);
            top_hbox_right->set_spacing(0);

            /*
            ########################
            ##      FINALIZE      ##
            ########################     
            */
            set_child(*vbox);
            vbox->append(*top_hbox);
            vbox->append(*main_paned);

            // make edges not rounded
            

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
