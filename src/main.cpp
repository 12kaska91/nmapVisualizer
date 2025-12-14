#include "graphics.hpp"

int main (int argc, char *argv[]) {
    try {
        std::locale::global(std::locale(""));
    } catch (const std::exception &e) {
        std::cerr << "Warning: failed to set global locale from environment: " << e.what() << "\n";
        std::cerr << "Falling back to C locale.\n";
        std::setlocale(LC_ALL, "C");
    }

    #ifdef _WIN32
        _putenv_s("LANG", "C");
    #else
        setenv("LANG", "C", 1);
    #endif

    auto app = nmapVisualizer::create();

    auto css = Gtk::CssProvider::create();
    css->load_from_data(R"(
      * {
        border-radius: 0;
      }
    )");

    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    return app->run(argc, argv);
}
