#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace ghost {
namespace os {

struct SystemInfo {
    std::string architecture;
    std::string os_type;
    uint64_t memory_total;
    uint32_t cpu_cores;
    std::vector<std::string> cpu_features;
};

struct ProcessInfo {
    uint32_t pid;
    std::string name;
    uint64_t memory_usage;
    float cpu_usage;
};

class OsToolkitError : public std::runtime_error {
public:
    explicit OsToolkitError(const std::string& message) : std::runtime_error(message) {}
};

class PlatformOps {
public:
    virtual ~PlatformOps() = default;
    virtual SystemInfo getSystemInfo() = 0;
    virtual void modifyMemoryProtection(uintptr_t addr, size_t size, bool protect) = 0;
    virtual std::vector<ProcessInfo> enumerateProcesses() = 0;
};

class OsToolkit {
public:
    struct Config {
        bool enable_hardware_access = false;
        bool enable_process_manipulation = false;
        bool enable_memory_manipulation = false;
    };

    explicit OsToolkit(const Config& config = Config{});
    ~OsToolkit();

    SystemInfo getSystemInfo();
    void modifyMemoryProtection(uintptr_t addr, size_t size, bool protect);
    std::vector<ProcessInfo> enumerateProcesses();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace os
} // namespace ghost
