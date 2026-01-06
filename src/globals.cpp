// globals.cpp
#include "globals.hpp"

namespace nmapVisualizerGlobals {
	std::vector<Network> networks;
	std::string selected = "";
	std::mutex networks_mutex;
}
