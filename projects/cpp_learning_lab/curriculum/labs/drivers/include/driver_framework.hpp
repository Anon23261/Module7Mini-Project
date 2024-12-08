#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ghost::drivers {

// Forward declarations
class Device;
class Driver;
class DriverManager;

/**
 * @brief Represents hardware device status
 */
enum class DeviceStatus {
    UNKNOWN,
    READY,
    BUSY,
    ERROR,
    OFFLINE
};

/**
 * @brief Base class for hardware device interfaces
 */
class Device {
public:
    virtual ~Device() = default;
    
    // Device identification
    virtual uint32_t get_vendor_id() const = 0;
    virtual uint32_t get_device_id() const = 0;
    virtual std::string get_name() const = 0;
    
    // Device status
    virtual DeviceStatus get_status() const = 0;
    virtual bool is_initialized() const = 0;
    
    // Memory-mapped I/O operations
    virtual uint8_t read_byte(uintptr_t offset) const = 0;
    virtual void write_byte(uintptr_t offset, uint8_t value) = 0;
    virtual uint32_t read_dword(uintptr_t offset) const = 0;
    virtual void write_dword(uintptr_t offset, uint32_t value) = 0;
    
    // Interrupt handling
    virtual void enable_interrupts() = 0;
    virtual void disable_interrupts() = 0;
    virtual void set_interrupt_handler(std::function<void()> handler) = 0;
};

/**
 * @brief Base class for device drivers
 */
class Driver {
public:
    virtual ~Driver() = default;
    
    // Driver lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool reset() = 0;
    
    // Device management
    virtual bool probe_device(Device& device) = 0;
    virtual void release_device(Device& device) = 0;
    
    // Driver information
    virtual std::string get_name() const = 0;
    virtual std::string get_version() const = 0;
    virtual std::vector<std::pair<uint32_t, uint32_t>> get_supported_devices() const = 0;
};

/**
 * @brief Manages driver registration and device matching
 */
class DriverManager {
public:
    static DriverManager& instance() {
        static DriverManager instance;
        return instance;
    }
    
    // Driver registration
    void register_driver(std::shared_ptr<Driver> driver) {
        drivers_.push_back(driver);
    }
    
    // Device matching
    std::shared_ptr<Driver> find_driver(const Device& device) {
        for (auto& driver : drivers_) {
            if (driver->probe_device(const_cast<Device&>(device))) {
                return driver;
            }
        }
        return nullptr;
    }
    
    // Driver enumeration
    const std::vector<std::shared_ptr<Driver>>& get_drivers() const {
        return drivers_;
    }

private:
    DriverManager() = default;
    std::vector<std::shared_ptr<Driver>> drivers_;
};

/**
 * @brief Base class for PCI devices
 */
class PCIDevice : public Device {
public:
    // PCI configuration space access
    virtual uint16_t get_command() const = 0;
    virtual void set_command(uint16_t command) = 0;
    virtual uint8_t get_interrupt_line() const = 0;
    virtual void set_interrupt_line(uint8_t line) = 0;
    virtual uint32_t get_bar(uint8_t index) const = 0;
    virtual void set_bar(uint8_t index, uint32_t value) = 0;
};

/**
 * @brief Base class for USB devices
 */
class USBDevice : public Device {
public:
    // USB-specific operations
    virtual uint8_t get_endpoint_count() const = 0;
    virtual uint16_t get_max_packet_size(uint8_t endpoint) const = 0;
    virtual bool transfer_data(uint8_t endpoint, const std::vector<uint8_t>& data) = 0;
    virtual bool receive_data(uint8_t endpoint, std::vector<uint8_t>& data) = 0;
};

/**
 * @brief Direct Memory Access (DMA) controller interface
 */
class DMAController {
public:
    virtual ~DMAController() = default;
    
    // DMA operations
    virtual bool setup_transfer(void* src, void* dst, size_t size) = 0;
    virtual bool start_transfer() = 0;
    virtual bool wait_for_completion() = 0;
    virtual void cancel_transfer() = 0;
    
    // Status
    virtual bool is_busy() const = 0;
    virtual size_t get_remaining_bytes() const = 0;
};

/**
 * @brief Interrupt controller interface
 */
class InterruptController {
public:
    virtual ~InterruptController() = default;
    
    // Interrupt management
    virtual void enable_irq(uint8_t irq) = 0;
    virtual void disable_irq(uint8_t irq) = 0;
    virtual bool is_irq_enabled(uint8_t irq) const = 0;
    virtual void acknowledge_irq(uint8_t irq) = 0;
    
    // Handler registration
    virtual void register_handler(uint8_t irq, std::function<void()> handler) = 0;
    virtual void unregister_handler(uint8_t irq) = 0;
};

} // namespace ghost::drivers
