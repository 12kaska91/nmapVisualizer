#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <libxml++-3.0/libxml++/libxml++.h>
#include <vector>

class Port {
public:
    int portNumber;
    std::string protocol;
    std::string state;
    std::string service;

    Port(int number, const std::string &proto, const std::string &st, const std::string &serv)
        : portNumber(number), protocol(proto), state(st), service(serv) {}
};

class DeviceInfo {
public:
    std::string ipAddress;
    std::string macAddress;
    std::string vendor;
    std::string deviceType;
    std::vector<Port> ports;
    std::string operatingSystem;

    DeviceInfo(
        const std::string &ip,
        const std::string &mac,
        const std::string &ven,
        const std::string &devType,
        const std::vector<Port> &prt,
        const std::string &os
    )
        : ipAddress(ip), macAddress(mac), vendor(ven), deviceType(devType), ports(prt), operatingSystem(os) {}
};

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