#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <vector>
#include <string>
#include <array>
#include <iostream>

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

namespace nmapVisualizerGlobals {
    extern std::vector<DeviceInfo> devices;
}

#endif // GLOBALS_HPP