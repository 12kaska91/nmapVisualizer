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
    css->load_from_data(
        "/* Global styles */\n"
        "* {\n"
        "    color: #E6E6E6;\n"
        "    background-color: #121212;\n"
        "    background-image: none;\n"
        "    border-color: #2b2b2b;\n"
        "    border-radius: 0px;\n"
        "}\n"
        "hovered {\n"
        "    background-color: #1E1E1E;\n"
        "}\n"
    );

    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    return app->run(argc, argv);
}
