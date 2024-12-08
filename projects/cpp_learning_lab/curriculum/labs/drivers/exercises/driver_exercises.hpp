#pragma once

#include "../include/driver_framework.hpp"
#include "../include/network_driver.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

namespace ghost::drivers::exercises {

/**
 * Week 3: Basic Driver Concepts
 */
namespace week3 {

// Exercise 1: Device Registration
void test_device_registration() {
    // Create a network card device
    auto card = std::make_shared<NetworkCard>(0x8086, 0x100E);
    
    // Create and register driver
    auto driver = std::make_shared<NetworkDriver>();
    DriverManager::instance().register_driver(driver);
    
    // Test driver matching
    auto matched_driver = DriverManager::instance().find_driver(*card);
    assert(matched_driver != nullptr);
    assert(matched_driver->get_name() == driver->get_name());
}

// Exercise 2: Basic I/O Operations
void test_basic_io() {
    NetworkCard card(0x8086, 0x100E);
    
    // Test register operations
    card.write_dword(NetworkCard::REG_CONTROL, 0x12345678);
    uint32_t value = card.read_dword(NetworkCard::REG_CONTROL);
    assert(value == 0x12345678);
    
    // Test byte operations
    card.write_byte(0, 0xAA);
    uint8_t byte = card.read_byte(0);
    assert(byte == 0xAA);
}

// Exercise 3: Interrupt Handling
void test_interrupt_handling() {
    NetworkCard card(0x8086, 0x100E);
    bool interrupt_received = false;
    
    // Set up interrupt handler
    card.set_interrupt_handler([&]() {
        interrupt_received = true;
    });
    
    // Enable interrupts
    card.enable_interrupts();
    
    // Simulate interrupt
    card.get_interrupt_handler()();
    assert(interrupt_received);
    
    // Disable interrupts
    card.disable_interrupts();
}

} // namespace week3

/**
 * Week 4: Advanced Driver Concepts
 */
namespace week4 {

// Exercise 1: Packet Transmission
void test_packet_transmission() {
    NetworkCard card(0x8086, 0x100E);
    
    // Create test packet
    NetworkPacket packet;
    packet.data = {0x01, 0x02, 0x03, 0x04};
    packet.size = 4;
    packet.flags = 0;
    
    // Send packet
    bool sent = card.send_packet(packet);
    assert(sent);
    
    // Receive packet
    NetworkPacket received;
    bool got_packet = card.receive_packet(received);
    assert(got_packet);
    assert(received.data == packet.data);
}

// Exercise 2: Multi-threaded Driver Operations
void test_multithreaded_operations() {
    NetworkCard card(0x8086, 0x100E);
    std::atomic<int> packets_sent{0};
    std::atomic<int> packets_received{0};
    
    // Create sender thread
    std::thread sender([&]() {
        for (int i = 0; i < 100; i++) {
            NetworkPacket packet;
            packet.data = {static_cast<uint8_t>(i)};
            packet.size = 1;
            if (card.send_packet(packet)) {
                packets_sent++;
            }
        }
    });
    
    // Create receiver thread
    std::thread receiver([&]() {
        while (packets_received < 100) {
            NetworkPacket packet;
            if (card.receive_packet(packet)) {
                packets_received++;
            }
        }
    });
    
    sender.join();
    receiver.join();
    
    assert(packets_sent == 100);
    assert(packets_received == 100);
}

// Exercise 3: Error Handling and Recovery
void test_error_handling() {
    NetworkCard card(0x8086, 0x100E);
    NetworkDriver driver;
    
    // Test device probe
    bool probed = driver.probe_device(card);
    assert(probed);
    
    // Test reset functionality
    bool reset = driver.reset();
    assert(reset);
    
    // Test shutdown and cleanup
    driver.shutdown();
}

// Exercise 4: Performance Testing
void test_performance() {
    NetworkCard card(0x8086, 0x100E);
    const int NUM_PACKETS = 10000;
    
    // Measure packet transmission time
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_PACKETS; i++) {
        NetworkPacket packet;
        packet.data.resize(1024);
        card.send_packet(packet);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    std::cout << "Transmitted " << NUM_PACKETS << " packets in "
              << duration << "ms\n";
    std::cout << "Average packet transmission time: "
              << (duration / NUM_PACKETS) << "ms\n";
}

} // namespace week4

// Run all exercises
void run_all_exercises() {
    std::cout << "Running Week 3 exercises...\n";
    week3::test_device_registration();
    std::cout << "✓ Device registration test passed\n";
    
    week3::test_basic_io();
    std::cout << "✓ Basic I/O test passed\n";
    
    week3::test_interrupt_handling();
    std::cout << "✓ Interrupt handling test passed\n";
    
    std::cout << "\nRunning Week 4 exercises...\n";
    week4::test_packet_transmission();
    std::cout << "✓ Packet transmission test passed\n";
    
    week4::test_multithreaded_operations();
    std::cout << "✓ Multi-threaded operations test passed\n";
    
    week4::test_error_handling();
    std::cout << "✓ Error handling test passed\n";
    
    week4::test_performance();
    std::cout << "✓ Performance test completed\n";
}

} // namespace ghost::drivers::exercises
