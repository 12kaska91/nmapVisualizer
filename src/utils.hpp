#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

std::string win_run_nmap_xml(const std::string &targets, const std::string &nmap_path = "C:\\Program Files (x86)\\Nmap\\nmap.exe") {
    // Note: " -oX - " -> XML to stdout
    std::string cmd = "\"" + nmap_path + "\" -oX - " + targets + " 2>nul";
    std::array<char, 4096> buffer;
    std::string result;

    // _popen is available in MSVC; returns FILE* you can read
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to run nmap (is it installed and in PATH?)");
    }
    while (fgets(buffer.data(), (int)buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    _pclose(pipe);
    return result;
}

#endif // UTILS_HPP