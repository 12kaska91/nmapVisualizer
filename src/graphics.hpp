#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "./utils.hpp"
#include "cairomm/fontface.h"
#include <gtkmm.h>
#include <sigc++/sigc++.h>
#include <future>
#include <sstream>
#include <memory>

#define M_PI 3.14159265358979323846

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
        set_size_request(600, 400);
        gesture_click = Gtk::GestureClick::create();
        add_controller(gesture_click);
        gesture_click->signal_pressed().connect(sigc::mem_fun(*this, &MapArea::on_click));
        signal_resize().connect([this](int width, int height){
            for (auto& network : networks) {
                auto& devices = network.devices;
                for (auto& d : devices) {
                    int num = devices.size();
                    double angle = (&d - &devices[0]) * (2 * M_PI / num);
                    d.x = width / 2.0 + (width / 3.0) * cos(angle);
                    d.y = height / 2.0 + (height / 3.0) * sin(angle);
                }
            }
            queue_draw();
        });


        set_draw_func(sigc::mem_fun(*this, &MapArea::draw_map));
    }
    
    // width 
    int get_width() {
        return get_allocated_width();
    }

    // height
    int get_height() {
        return get_allocated_height();
    }
    
    void update_networks() {
        std::lock_guard<std::mutex> lock(nmapVisualizerGlobals::networks_mutex);
        networks.clear();
        for (auto& net : nmapVisualizerGlobals::networks) {
            Network newNet;
            newNet.cidr = net.cidr;
            newNet.center_x = get_width() / 2.0;
            newNet.center_y = get_height() / 2.0;

            for (auto& d : net.devices) {
                newNet.devices.push_back(Device{
                    d,
                    0,
                    0
                });
            }
            
            networks.push_back(newNet);
        }
        for(auto& network : networks) {
            auto& devices = network.devices;
            for (auto& d : devices) {
                int num = devices.size();
                double angle = (&d - &devices[0]) * (2 * M_PI / num);
                d.x = get_width() / 2.0 + (get_width() / 3.0) * cos(angle);
                d.y = get_height() / 2.0 + (get_height() / 3.0) * sin(angle);
            }
        }
        for (const auto& net : nmapVisualizerGlobals::networks) {
            std::cout << "Network CIDR: " << net.cidr << ", Devices: " << net.devices.size() << std::endl;
            for (const auto& d : net.devices) {
                std::cout << " - Device IP: " << d.ipAddress << std::endl;
            }
        }
        queue_draw();
    }
    
    sigc::signal<void(DeviceInfo)> signal_device_selected_;
    sigc::signal<void(void)> signal_cleared_;

    sigc::signal<void(DeviceInfo)>& signal_device_selected() { return signal_device_selected_; }
    sigc::signal<void(void)>& signal_cleared() { return signal_cleared_; }

private:
    Glib::RefPtr<Gtk::GestureClick> gesture_click;

    struct Device {
        DeviceInfo info;
        double x, y;
    };

    struct Network {
        std::string cidr;
        std::vector<Device> devices;
        double center_x, center_y;
    };

    std::vector<Network> networks;

    void on_click(int, double x, double y) {
        for (auto& net : networks) for (auto& d : net.devices) {
            double dx = x - d.x, dy = y - d.y;
            if (dx*dx + dy*dy <= 20*20) {
                nmapVisualizerGlobals::selected = d.info.ipAddress;
                signal_device_selected_.emit(d.info);
                queue_draw();
                return;
            }
        }
        nmapVisualizerGlobals::selected.clear();
        signal_cleared_.emit();
        queue_draw();
    }
    
    void draw_map(const Cairo::RefPtr<Cairo::Context>& cr, int /*width*/, int /*height*/) {
        int width = get_width();
        int height = get_height();

        // Clear background
        cr->set_source_rgb(0.1, 0.1, 0.1);
        cr->rectangle(0, 0, width, height);
        cr->fill();

        for (MapArea::Network network : networks) {
            std::vector<MapArea::Device> devices = network.devices;
            for (MapArea::Device d : devices) {
                int num = devices.size();
                double angle = (&d - &devices[0]) * (2 * M_PI / num);
                d.x = width / 2.0 + (width / 3.0) * cos(angle);
                d.y = height / 2.0 + (height / 3.0) * sin(angle);

            }

            // Draw connections
            cr->set_line_width(2.0);
            cr->set_source_rgb(0.7, 0.7, 0.7);
            for (MapArea::Device d : devices) {
                cr->move_to(d.x, d.y);
                cr->line_to(width/2, height/2);
                cr->stroke();
            }

            // Draw devices
            for (MapArea::Device d : devices) {
                const bool is_selected = (nmapVisualizerGlobals::selected == d.info.ipAddress);

                // circle
                if (is_selected) {
                    cr->set_source_rgb(0.2, 0.8, 1.0);
                } else {
                    cr->set_source_rgb(1.0, 1.0, 1.0);
                }
                cr->arc(d.x, d.y, 20.0, 0, 2*M_PI);
                cr->fill();

                // font
                cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::NORMAL);
                cr->set_font_size(10.0);

                // extents
                Cairo::TextExtents extents;
                cr->get_text_extents(d.info.ipAddress, extents);

                // label shadow
                cr->set_source_rgba(0, 0, 0, 0.5);
                cr->move_to(d.x - extents.width / 2 + 1, d.y + 31);
                cr->show_text(d.info.ipAddress);

                // text
                cr->set_source_rgb(1.0, 1.0, 1.0);
                cr->move_to(d.x - extents.width / 2, d.y + 30);
                cr->show_text(d.info.ipAddress);
            }

            // Draw network center
            cr->set_source_rgb(1.0, 1.0, 1.0);
            cr->arc(width/2, height/2, 20.0, 0, 2*M_PI);
            cr->fill();

            // Draw network label
            cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::BOLD);
            cr->set_font_size(10.0);
            Cairo::TextExtents extents;
            cr->get_text_extents(network.cidr, extents);

            cr->set_source_rgba(0, 0, 0, 0.5);
            cr->move_to(width/2 - extents.width / 2 + 1, height/2 + 31);
            cr->show_text(network.cidr);

            cr->set_source_rgb(1.0, 1.0, 1.0);
            cr->move_to(width/2 - extents.width / 2, height/2 + 30);
            cr->show_text(network.cidr);
        }
    }
};

class MainWindow : public Gtk::Window {
    public:
        MainWindow() {
            set_title("nmap Visualizer");
            set_default_size(1024, 768);

            // create top bar elements
            auto file = Gtk::make_managed<Gtk::MenuButton>();
            auto fileMenu = Gio::Menu::create();
            auto scanOptions = Gtk::make_managed<Gtk::MenuButton>();
            auto scanOptionsMenu = Gio::Menu::create();
            auto go_button = Gtk::make_managed<Gtk::Button>("Scan");
            ip_entry_ = Gtk::make_managed<Gtk::Entry>();

            // create main layout
            auto main_paned = Gtk::make_managed<Gtk::Paned>(Gtk::Orientation::HORIZONTAL);
            map_area_ = Gtk::make_managed<MapArea>();
            // create boxes
            auto top_hbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10); 
            auto top_hbox_left = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
            auto top_hbox_right = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);

            auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);

            // attributes panel
            auto attrs_frame = Gtk::make_managed<Gtk::Frame>("Attributes");
            auto attrs_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 8);
            auto attrs_grid = Gtk::make_managed<Gtk::Grid>();
            attrs_grid->set_column_spacing(8);
            attrs_grid->set_row_spacing(6);

            auto add_row = [&](const std::string& label, Gtk::Label*& value_label, int row){
                auto key = Gtk::make_managed<Gtk::Label>(label);
                key->set_xalign(0);
                value_label = Gtk::make_managed<Gtk::Label>("-");
                value_label->set_xalign(0);
                attrs_grid->attach(*key, 0, row, 1, 1);
                attrs_grid->attach(*value_label, 1, row, 1, 1);
            };

            add_row("IP", ip_label_, 0);
            add_row("MAC", mac_label_, 1);
            add_row("Vendor", vendor_label_, 2);
            add_row("OS", os_label_, 3);
            add_row("Ports", ports_label_, 4);

            attrs_box->append(*attrs_grid);
            show_empty_attrs();

            // Add status label
            status_label_ = Gtk::make_managed<Gtk::Label>("");
            status_label_->set_xalign(0);
            attrs_box->append(*status_label_);

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
            main_paned->set_start_child(*map_area_);

            // attributes
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

            map_area_->set_hexpand(true);
            map_area_->set_vexpand(true);

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

            map_area_->signal_device_selected().connect(sigc::mem_fun(*this, &MainWindow::update_attrs));
            map_area_->signal_cleared().connect([this]{ show_empty_attrs(); });
                    
            show();
        }

        // getters
        std::string get_ip_entry_text() const {
            return ip_entry_ ? ip_entry_->get_text() : "";
        }

        MapArea* get_map_area() const { return map_area_; }

        void update_attrs(const DeviceInfo& d) {
            ip_label_->set_text(d.ipAddress);
            mac_label_->set_text(d.macAddress);
            vendor_label_->set_text(d.vendor);
            os_label_->set_text(d.operatingSystem);
            std::string ports;
            for (const auto& p : d.ports) {
                ports += std::to_string(p.portNumber) + "/" + p.protocol + " " + p.state;
                if (!p.service.empty()) ports += " (" + p.service + ")";
                ports += "\n";
            }
            if (ports.empty()) ports = "-";
            ports_label_->set_text(ports);
        }

        void show_empty_attrs() {
            if (ip_label_) ip_label_->set_text("-");
            if (mac_label_) mac_label_->set_text("-");
            if (vendor_label_) vendor_label_->set_text("-");
            if (os_label_) os_label_->set_text("-");
            if (ports_label_) ports_label_->set_text("-");
        }

        void set_status(const std::string& status) {
            if (status_label_) {
                status_label_->set_text(status);
            }
        }

    private:
        Gtk::Entry* ip_entry_ = nullptr;
        MapArea* map_area_ = nullptr;
        Gtk::Label* ip_label_ = nullptr;
        Gtk::Label* mac_label_ = nullptr;
        Gtk::Label* vendor_label_ = nullptr;
        Gtk::Label* os_label_ = nullptr;
        Gtk::Label* ports_label_ = nullptr;
        Gtk::Label* status_label_ = nullptr;
};

class nmapVisualizer : public Gtk::Application {
    protected:
        nmapVisualizer() : Gtk::Application("org.kaska.nmapVisualizer") {
            scanner_ = std::make_unique<ParallelScanner>();
        }

        void on_startup() override {
            Gtk::Application::on_startup();

            // Define actions
            add_action("hello", sigc::mem_fun(*this, &nmapVisualizer::on_hello));
            add_action("quit", sigc::mem_fun(*this, &nmapVisualizer::on_quit));
            add_action("go_button", sigc::mem_fun(*this, &nmapVisualizer::on_go_button_clicked));
            
            // Start periodic timer to check for scan completion
            Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &nmapVisualizer::on_timer_check_scans),
                500  // Check every 500ms
            );
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
                
                if (target.empty()) {
                    win->set_status("Error: Please enter a target IP or network");
                    return;
                }
                
                std::cout << "Starting parallel nmap scan on target: " << target << std::endl;
                win->set_status("Scanning " + target + "...");
                
                // Parse multiple targets (comma or space separated)
                std::vector<std::string> targets;
                std::stringstream ss(target);
                std::string item;
                
                // Try comma first
                while (std::getline(ss, item, ',')) {
                    // Trim whitespace
                    item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
                    item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
                    if (!item.empty()) {
                        targets.push_back(item);
                    }
                }
                
                // If no comma found, try space
                if (targets.size() <= 1) {
                    targets.clear();
                    ss.clear();
                    ss.str(target);
                    while (ss >> item) {
                        targets.push_back(item);
                    }
                }
                
                // Launch parallel scans
                for (const auto& t : targets) {
                    scanner_->add_scan(t, t);
                }
                
                win->set_status("Scanning " + std::to_string(targets.size()) + " target(s)...");
            }
        }
        
        // Timer callback to check scan progress
        bool on_timer_check_scans() {
            if (scanner_->check_progress()) {
                // A scan completed, update the UI
                if (auto win = dynamic_cast<MainWindow*>(get_active_window())) {
                    if (auto map = win->get_map_area()) {
                        // Use Glib dispatcher for thread-safe UI update
                        Glib::signal_idle().connect_once([win, map]() {
                            map->update_networks();
                            map->queue_draw();
                            
                            int active = 0;
                            // Update status would require scanner access, simplified here
                            win->set_status("Scan completed. Ready.");
                        });
                    }
                }
                scanner_->clear_completed();
            }
            
            // Update status with active scan count
            int active_scans = scanner_->active_count();
            if (active_scans > 0) {
                if (auto win = dynamic_cast<MainWindow*>(get_active_window())) {
                    Glib::signal_idle().connect_once([win, active_scans]() {
                        win->set_status("Scanning... (" + std::to_string(active_scans) + " active)");
                    });
                }
            }
            
            return true;  // Keep timer running
        }

    private:
        std::unique_ptr<ParallelScanner> scanner_;

    public:
        static Glib::RefPtr<nmapVisualizer> create() {
            return Glib::RefPtr<nmapVisualizer>(new nmapVisualizer());
        }
};

#endif // GRAPHICS_HPP
