#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <vector>
#include <string>
#include <mutex>

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

class Network {
public:
    std::string cidr;
    std::vector<DeviceInfo> devices;
    Network(
        const std::string &c,
        const std::vector<DeviceInfo> &d
    )
        : cidr(c), devices(d) {}
};

namespace nmapVisualizerGlobals {
    extern std::string selected;
    extern std::vector<Network> networks;
    extern std::mutex networks_mutex;
}

#endif // GLOBALS_HPP
