#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>
#include <array>
#include <iostream>
#include <string>
#include <stdexcept>
#include <libxml/parser.h>

#include "globals.hpp"

#if defined(_WIN32) || defined(_WIN64)
std::string win_run_nmap_xml(const std::string &targets, const std::string &nmap_path = "C:\\Program Files (x86)\\Nmap\\nmap.exe") {
    // Note: " -oX - " -> XML to stdout
    std::string cmd = "\"" + nmap_path + "\" -oX - " + targets + " 2>nul";
    std::array<char, 4096> buffer;
    std::string result;

    // _popen is available on Windows; returns FILE* you can read
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to run nmap (is it installed in the default path?)");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
    _pclose(pipe);
    return result;
}
#endif

#if defined(__linux__)
std::string linux_run_nmap_xml(const std::string &targets, const std::string &nmap_path = "/usr/bin/nmap") {
    // Note: " -oX - " -> XML to stdout
    std::string cmd = nmap_path + " -oX - " + targets + " 2>/dev/null";
    std::array<char, 4096> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to run nmap (is it installed and in PATH?)");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}
#endif

std::string run_nmap(const std::string &targets, std::string nmap_path = "") {
    // Cross-platform nmap runner (synchronous) — return result directly
    #if defined(_WIN32) || defined(_WIN64)
        if (nmap_path.empty()) { nmap_path = "C:\\Program Files (x86)\\Nmap\\nmap.exe"; }
        return win_run_nmap_xml(targets, nmap_path);
    #elif defined(__linux__)
        if (nmap_path.empty()) { nmap_path = "/usr/bin/nmap"; }
        return linux_run_nmap_xml(targets, nmap_path);
    #else
        throw std::runtime_error("Unsupported platform for running nmap");
    #endif
}

void save_devices(const std::vector<DeviceInfo> &devices, const std::string &cidr = "default") {
    std::cout << "Saving " << devices.size() << " devices for network: " << cidr << std::endl;
    nmapVisualizerGlobals::networks.push_back(Network(cidr, devices));
}

std::vector<DeviceInfo> get_devices(const std::string &cidr = "default") {
    for (const auto& network : nmapVisualizerGlobals::networks) {
        if (network.cidr == cidr) {
            return network.devices;
        }
    }
    return {};
}

std::vector<DeviceInfo> parse_nmap_xml(const std::string &xmlData) {
    std::vector<DeviceInfo> devices;
    try {
        xmlDocPtr doc = xmlParseMemory(xmlData.c_str(), xmlData.size());
        if (!doc) throw std::runtime_error("Failed to parse XML");

        xmlNodePtr root = xmlDocGetRootElement(doc);
        for (xmlNodePtr hostNode = root->children; hostNode; hostNode = hostNode->next) {
            if (hostNode->type == XML_ELEMENT_NODE && xmlStrcmp(hostNode->name, BAD_CAST "host") == 0) {
            std::string ipAddress;
            std::string macAddress;
            std::string vendor;
            std::string deviceType;
            std::vector<Port> ports;
            std::string operatingSystem;

            // Use libxml2 (xml2) APIs to walk the hostNode children and fill fields
            for (xmlNodePtr child = hostNode->children; child; child = child->next) {
                if (child->type != XML_ELEMENT_NODE) continue;

                if (xmlStrcmp(child->name, BAD_CAST "address") == 0) {
                    xmlChar* addrType = xmlGetProp(child, BAD_CAST "addrtype");
                    xmlChar* addr     = xmlGetProp(child, BAD_CAST "addr");
                    xmlChar* vend     = xmlGetProp(child, BAD_CAST "vendor");

                    if (addrType) {
                        const std::string at(reinterpret_cast<const char*>(addrType));
                        if ((at == "ipv4" || at == "ipv6") && addr) {
                            ipAddress = reinterpret_cast<const char*>(addr);
                        } else if (at == "mac" && addr) {
                            macAddress = reinterpret_cast<const char*>(addr);
                            if (vend) vendor = reinterpret_cast<const char*>(vend);
                        }
                    }

                    if (addrType) xmlFree(addrType);
                    if (addr)     xmlFree(addr);
                    if (vend)     xmlFree(vend);
                } else if (xmlStrcmp(child->name, BAD_CAST "hostnames") == 0) {
                    for (xmlNodePtr hn = child->children; hn; hn = hn->next) {
                        if (hn->type != XML_ELEMENT_NODE) continue;
                        if (xmlStrcmp(hn->name, BAD_CAST "hostname") == 0) {
                            xmlChar* nameAttr = xmlGetProp(hn, BAD_CAST "name");
                            if (nameAttr) {
                                deviceType = reinterpret_cast<const char*>(nameAttr);
                                xmlFree(nameAttr);
                                break; // use first hostname
                            }
                        }
                    }
                } else if (xmlStrcmp(child->name, BAD_CAST "ports") == 0) {
                    for (xmlNodePtr portNode = child->children; portNode; portNode = portNode->next) {
                        if (portNode->type != XML_ELEMENT_NODE) continue;
                        if (xmlStrcmp(portNode->name, BAD_CAST "port") != 0) continue;

                        int portNumber = 0;
                        xmlChar* portid = xmlGetProp(portNode, BAD_CAST "portid");
                        xmlChar* proto  = xmlGetProp(portNode, BAD_CAST "protocol");
                        if (portid) {
                            try { portNumber = std::stoi(reinterpret_cast<const char*>(portid)); } catch (...) { portNumber = 0; }
                            xmlFree(portid);
                        }
                        std::string protocol = proto ? reinterpret_cast<const char*>(proto) : std::string();
                        if (proto) xmlFree(proto);

                        std::string state;
                        std::string service;

                        for (xmlNodePtr pchild = portNode->children; pchild; pchild = pchild->next) {
                            if (pchild->type != XML_ELEMENT_NODE) continue;
                            if (xmlStrcmp(pchild->name, BAD_CAST "state") == 0) {
                                xmlChar* stateAttr = xmlGetProp(pchild, BAD_CAST "state");
                                if (stateAttr) { state = reinterpret_cast<const char*>(stateAttr); xmlFree(stateAttr); }
                            } else if (xmlStrcmp(pchild->name, BAD_CAST "service") == 0) {
                                xmlChar* sname     = xmlGetProp(pchild, BAD_CAST "name");
                                xmlChar* sproduct  = xmlGetProp(pchild, BAD_CAST "product");
                                xmlChar* sversion  = xmlGetProp(pchild, BAD_CAST "version");
                                xmlChar* sextrainfo= xmlGetProp(pchild, BAD_CAST "extrainfo");
                                xmlChar* sostype   = xmlGetProp(pchild, BAD_CAST "ostype");

                                if (sname) {
                                    service = reinterpret_cast<const char*>(sname);
                                }
                                if (sproduct && service.empty() == false) {
                                    service += " (" + std::string(reinterpret_cast<const char*>(sproduct));
                                    if (sversion) service += " " + std::string(reinterpret_cast<const char*>(sversion));
                                    service += ")";
                                } else if (sproduct && service.empty()) {
                                    service = reinterpret_cast<const char*>(sproduct);
                                    if (sversion) service += " " + std::string(reinterpret_cast<const char*>(sversion));
                                }
                                if (sextrainfo) service += " " + std::string(reinterpret_cast<const char*>(sextrainfo));
                                if (sostype) service += " [os:" + std::string(reinterpret_cast<const char*>(sostype)) + "]";

                                if (sname)      xmlFree(sname);
                                if (sproduct)   xmlFree(sproduct);
                                if (sversion)   xmlFree(sversion);
                                if (sextrainfo) xmlFree(sextrainfo);
                                if (sostype)    xmlFree(sostype);
                            }
                        }

                        ports.emplace_back(portNumber, protocol, state, service);
                    }
                } else if (xmlStrcmp(child->name, BAD_CAST "os") == 0) {
                    for (xmlNodePtr osChild = child->children; osChild; osChild = osChild->next) {
                        if (osChild->type != XML_ELEMENT_NODE) continue;
                        if (xmlStrcmp(osChild->name, BAD_CAST "osmatch") == 0) {
                            xmlChar* nameAttr = xmlGetProp(osChild, BAD_CAST "name");
                            if (nameAttr) {
                                operatingSystem = reinterpret_cast<const char*>(nameAttr);
                                xmlFree(nameAttr);
                                break;
                            }
                        }
                    }
                }
            }

            if (ipAddress.empty())        ipAddress = "Unknown";
            if (macAddress.empty())       macAddress = "Unknown";
            if (vendor.empty())           vendor = "Unknown";
            if (deviceType.empty())       deviceType = "Unknown";
            if (operatingSystem.empty())  operatingSystem = "Unknown";

            devices.emplace_back(ipAddress, macAddress, vendor, deviceType, ports, operatingSystem);
        }
        }
        xmlFreeDoc(doc);
    } catch (const std::exception e) {
        std::cerr << "Error parsing Nmap XML: " << e.what() << std::endl;
    }
    return devices;
}

#endif // UTILS_HPP
