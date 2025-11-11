#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <gtkmm.h>
#include <cmath>
#include <algorithm>
#include "utils.hpp"

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
            // allow the drawing area to expand and update its content size dynamically
            set_hexpand(true);
            set_vexpand(true);

            set_halign(Gtk::Align::FILL);
            set_valign(Gtk::Align::FILL);

            try {
                icon_pixbuf = Gdk::Pixbuf::create_from_file("../assets/pc.svg");
            } catch (const Glib::FileError& ex) {
                std::cerr << "FileError: " << ex.what() << std::endl;
            } catch (const Gdk::PixbufError& ex) {
                std::cerr << "PixbufError: " << ex.what() << std::endl;
            }

            auto click = Gtk::GestureClick::create();
            click->signal_pressed().connect(sigc::mem_fun(*this, &MapArea::on_click));
            add_controller(click);

            auto drag = Gtk::GestureDrag::create();
            drag->signal_drag_update().connect(sigc::mem_fun(*this, &MapArea::on_drag_update));
            add_controller(drag);
        }

        void add_device(const DeviceInfo& device) {
            devices.push_back(device);
        }

        void clear_devices() {
            devices.clear();
        }

        std::vector<DeviceInfo> get_devices() const {
            return devices;
        }

        // Request a redraw; actual rendering happens in on_draw()
        void draw() {
            std::cout << "Redrawing MapArea with " << devices.size() << " devices." << std::endl;
            queue_draw();
        }

        // public forwarding draw function for set_draw_func (calls the protected on_draw)
        void draw_func(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
            on_draw(cr, width, height);
        }

        void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
            const int w = width;
            const int h = height;
            if (w <= 0 || h <= 0) return;

            // clear background (white)
            cr->set_source_rgb(1.0, 1.0, 1.0);
            cr->paint();

            // draw base image centered and scaled to fit (if loaded)
            if (icon_pixbuf) {
                cr->save();
                Gdk::Cairo::set_source_pixbuf(cr, icon_pixbuf, 0, 0);
                cr->paint();
                cr->restore();
            }

            // draw a vertical center guide line
            cr->set_source_rgba(0.0, 0.0, 0.0, 0.25);
            cr->set_line_width(2.0);
            cr->move_to(w / 2.0, 0.0);
            cr->line_to(w / 2.0, static_cast<double>(h));
            cr->stroke();

            // draw devices arranged around the center in a circle
            const size_t n = devices.size();
            if (n > 0) {
                const double cx = w / 2.0;
                const double cy = h / 2.0;
                const double radius = std::max(40.0, std::min(w, h) / 3.0);

                // icon size to render for each device
                const double icon_w = 32.0;
                const double icon_h = 32.0;

                // simple label settings
                cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::NORMAL);
                cr->set_font_size(12.0);

                constexpr double PI = 3.14159265358979323846;
                for (size_t i = 0; i < n; ++i) {
                    double angle = (2.0 * PI * static_cast<double>(i)) / static_cast<double>(n);
                    double dx = cx + radius * std::cos(angle);
                    double dy = cy + radius * std::sin(angle);

                    // draw device icon (use loaded icon_pixbuf if available, otherwise draw a circle)
                    if (icon_pixbuf) {
                        cr->save();
                        // translate to top-left of where we want the icon
                        cr->translate(dx - icon_w / 2.0, dy - icon_h / 2.0);
                        // scale the pixbuf to the desired icon size
                        const double sx_icon = icon_w / static_cast<double>(icon_pixbuf->get_width());
                        const double sy_icon = icon_h / static_cast<double>(icon_pixbuf->get_height());
                        cr->scale(sx_icon, sy_icon);
                        Gdk::Cairo::set_source_pixbuf(cr, icon_pixbuf, 0, 0);
                        cr->paint();
                        cr->restore();
                    } else {
                        cr->save();
                        cr->arc(dx, dy, 10.0, 0.0, 2.0 * PI);
                        cr->set_source_rgb(0.2, 0.6, 0.8);
                        cr->fill_preserve();
                        cr->set_source_rgb(0.0, 0.0, 0.0);
                        cr->stroke();
                        cr->restore();
                    }

                    std::string label;
                    try {
                        label = devices.at(i).ipAddress;
                    } catch (...) {
                        label = {};
                    }
                    if (!label.empty()) {
                        Cairo::TextExtents ext;
                        cr->get_text_extents(label, ext);
                        cr->set_source_rgb(0.0, 0.0, 0.0);
                        cr->move_to(dx - ext.width / 2.0, dy + icon_h / 2.0 + 14.0);
                        cr->show_text(label);
                    }
                }
            }
        }

    protected:
        void on_click(int n_press, double x, double y) {
            std::cout << "Clicked at (" << x << ", " << y << ")" << std::endl;
            // TODO: check if click falls within a defined region
        }

        void on_drag_update(double offset_x, double offset_y) {
            std::cout << "Dragged to offset (" << offset_x << ", " << offset_y << ")" << std::endl;
        }

    private:
    // pixbuf used for icons / base image loaded from assets
        Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf;
        std::vector<DeviceInfo> devices;
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
            auto go_button = Gtk::make_managed<Gtk::Button>("Go");
            ip_entry_ = Gtk::make_managed<Gtk::Entry>();

            // create main layout
            auto main_paned = Gtk::make_managed<Gtk::Paned>(Gtk::Orientation::HORIZONTAL);
            auto notebook = Gtk::make_managed<Gtk::Notebook>();
            auto device_map_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5); // placeholder for device map
            auto map_area = Gtk::make_managed<MapArea>();
            // ensure the DrawingArea has its draw callback connected (gtkmm4 uses set_draw_func)
            map_area->set_draw_func(sigc::mem_fun(*map_area, &MapArea::draw_func));
            // store pointer for later access
            map_area_ = map_area;
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
            notebook->append_page(*device_map_box, "Device Map");
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

        // get map area
        MapArea* get_map_area() {
            // return the stored pointer; it is managed by the widget hierarchy so raw pointer is safe to use
            return map_area_;
        }

    private:
        Gtk::Entry* ip_entry_ = nullptr;
        // stored pointer to the MapArea instance created in the constructor
        MapArea* map_area_ = nullptr;
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

            save_devices(devices);
            if (auto win = dynamic_cast<MainWindow*>(get_active_window())) {
                if (auto map_area = win->get_map_area()) {
                    map_area->clear_devices();
                    for (const auto& device : devices) {
                        map_area->add_device(device);
                    }
                    map_area->draw();
                }
            }
        }

    public:
        static Glib::RefPtr<nmapVisualizer> create() {
            return Glib::RefPtr<nmapVisualizer>(new nmapVisualizer());
        }
};

#endif // GRAPHICS_HPP