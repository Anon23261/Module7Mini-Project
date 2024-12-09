#pragma once

#include <string>
#include <vector>

namespace ghost_os {

class ArmOps {
public:
    static bool isArmArchitecture();
    static std::string getArmVersion();
    static std::vector<std::string> getSupportedFeatures();
};

} // namespace ghost_os
