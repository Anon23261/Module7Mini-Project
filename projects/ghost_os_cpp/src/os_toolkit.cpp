#include "os_toolkit.hpp"

#ifdef WINDOWS_SUPPORT
#include "windows_ops.hpp"
#endif

#ifdef ARM_SUPPORT
#include "arm_ops.hpp"
#endif

namespace ghost {
namespace os {

class OsToolkit::Impl {
public:
    explicit Impl(const Config& config) : config_(config) {
        #ifdef WINDOWS_SUPPORT
        windows_ops_ = std::make_unique<WindowsOps>();
        #endif

        #ifdef ARM_SUPPORT
        arm_ops_ = std::make_unique<ArmOps>();
        #endif
    }

    SystemInfo getSystemInfo() {
        #ifdef WINDOWS_SUPPORT
        if (windows_ops_) {
            return windows_ops_->getSystemInfo();
        }
        #endif

        #ifdef ARM_SUPPORT
        if (arm_ops_) {
            return arm_ops_->getSystemInfo();
        }
        #endif

        throw OsToolkitError("No supported platform implementation available");
    }

    void modifyMemoryProtection(uintptr_t addr, size_t size, bool protect) {
        if (!config_.enable_memory_manipulation) {
            throw OsToolkitError("Memory manipulation not enabled");
        }

        #ifdef WINDOWS_SUPPORT
        if (windows_ops_) {
            return windows_ops_->modifyMemoryProtection(addr, size, protect);
        }
        #endif

        #ifdef ARM_SUPPORT
        if (arm_ops_) {
            return arm_ops_->modifyMemoryProtection(addr, size, protect);
        }
        #endif

        throw OsToolkitError("No supported platform implementation available");
    }

    std::vector<ProcessInfo> enumerateProcesses() {
        if (!config_.enable_process_manipulation) {
            throw OsToolkitError("Process manipulation not enabled");
        }

        #ifdef WINDOWS_SUPPORT
        if (windows_ops_) {
            return windows_ops_->enumerateProcesses();
        }
        #endif

        #ifdef ARM_SUPPORT
        if (arm_ops_) {
            return arm_ops_->enumerateProcesses();
        }
        #endif

        throw OsToolkitError("No supported platform implementation available");
    }

private:
    Config config_;
    #ifdef WINDOWS_SUPPORT
    std::unique_ptr<WindowsOps> windows_ops_;
    #endif
    #ifdef ARM_SUPPORT
    std::unique_ptr<ArmOps> arm_ops_;
    #endif
};

OsToolkit::OsToolkit(const Config& config)
    : pimpl(std::make_unique<Impl>(config)) {}

OsToolkit::~OsToolkit() = default;

SystemInfo OsToolkit::getSystemInfo() {
    return pimpl->getSystemInfo();
}

void OsToolkit::modifyMemoryProtection(uintptr_t addr, size_t size, bool protect) {
    pimpl->modifyMemoryProtection(addr, size, protect);
}

std::vector<ProcessInfo> OsToolkit::enumerateProcesses() {
    return pimpl->enumerateProcesses();
}

} // namespace os
} // namespace ghost
