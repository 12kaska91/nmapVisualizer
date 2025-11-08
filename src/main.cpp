#include <iostream>
#include <thread>
#include <clocale>
#include <locale>
#include <cstdlib>
#include "graphics.hpp"
#include "utils.hpp"

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

    std::string targets = "scanme.nmap.org";
    std::string nmapOutput = win_run_nmap_xml(targets);
    auto devices = parse_nmap_xml(nmapOutput);
    for (const auto& device : devices) {
        std::cout << "Device IP: " << device.ipAddress << ", MAC: " << device.macAddress << ", OS: " << device.operatingSystem << std::endl;
    }

    auto app = MyApplication::create();
    return app->run(argc, argv);
}