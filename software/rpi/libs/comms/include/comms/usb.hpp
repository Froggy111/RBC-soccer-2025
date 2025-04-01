#pragma once

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

#include "types.hpp"
#include "identifiers.hpp"

namespace usb {

class CDC {
public:
    // Maximum buffer sizes for receiving and sending data
    static const types::u16 MAX_RX_BUF_SIZE = 1024;
    static const types::u16 MAX_TX_BUF_SIZE = 1024;

    // Callback type for message handlers
    using MessageCallback = std::function<void(const types::u8*, types::u16)>;

    CDC();
    ~CDC();
    
    // Initialize the communication system
    bool init();
    
    // Send messages to specific boards
    bool writeToBottomPico(comms::SendBottomPicoIdentifiers identifier, const types::u8* data, types::u16 data_len);
    bool writeToMiddlePico(comms::SendMiddlePicoIdentifiers identifier, const types::u8* data, types::u16 data_len);
    bool writeToTopPico(comms::SendTopPicoIdentifiers identifier, const types::u8* data, types::u16 data_len);
    
    // Register message handlers for specific message types
    void registerBottomPicoHandler(comms::RecvBottomPicoIdentifiers identifier, MessageCallback callback);
    void registerMiddlePicoHandler(comms::RecvMiddlePicoIdentifiers identifier, MessageCallback callback);
    void registerTopPicoHandler(comms::RecvTopPicoIdentifiers identifier, MessageCallback callback);

private:
    // Struct to store detected Pico devices
    struct PicoDevice {
        std::string port;
        comms::BoardIdentifiers board_id;
        int fd;  // File descriptor for the serial port
        bool identified;
        std::thread rx_thread;
        std::atomic<bool> running;
        std::mutex tx_mutex;
    };

    // Function to scan for Pico devices on /dev/ttyACM*
    void scanDevices();
    
    // Function to identify a Pico board
    void identifyBoard(PicoDevice& device);
    
    // Thread function to handle receiving data from a Pico
    void rxThreadFunc(PicoDevice& device);
    
    // Helper function to write data to a Pico
    bool writeToPico(PicoDevice& device, const types::u8* identifier_ptr, const types::u8* data, types::u16 data_len);
    
    // Process a received message
    void processMessage(comms::BoardIdentifiers board, types::u8 identifier, const types::u8* data, types::u16 data_len);

    // Store detected Pico devices
    std::map<comms::BoardIdentifiers, std::shared_ptr<PicoDevice>> _devices;
    
    // Store message handlers
    std::map<types::u8, MessageCallback> _bottom_pico_handlers;
    std::map<types::u8, MessageCallback> _middle_pico_handlers;
    std::map<types::u8, MessageCallback> _top_pico_handlers;
    
    // Mutex for thread safety
    std::mutex _devices_mutex;
    std::mutex _handlers_mutex;
    
    // Flag to indicate if the communication system is initialized
    bool _initialized;
};

} // namespace comms