#pragma once

#include "comms/default_usb_config.h"
#include "identifiers.hpp"
#include "types.hpp"
#include <cstring>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>

namespace usb {

/* ******** *
 * Settings *
 * ******** */

static const types::u32 DEVICE_SCAN_TIMEOUT = 1000; // in milliseconds
static const types::u32 CDC_CONNECTION_TIMEOUT = 1000;  // in milliseconds
static const types::u32 DEVICE_SCAN_INTERVAL = 2000;    // in milliseconds

static const types::u16 MAX_RX_BUF_SIZE = USB_RX_BUFSIZE;
static const types::u16 MAX_TX_BUF_SIZE = USB_TX_BUFSIZE;

static const types::u8 N_LENGTH_BYTES = 2;
static const types::u16 MAX_RX_PACKET_LENGTH = MAX_RX_BUF_SIZE;
static const types::u16 MAX_TX_PACKET_LENGTH = MAX_TX_BUF_SIZE;

// Device type identifiers
enum class DeviceType {
    UNKNOWN,
    TOP_PLATE,
    MIDDLE_PLATE,
    BOTTOM_PLATE
};

/* ******* *
 * Structs *
 * ******* */

struct USBDevice {
    std::string path;
    std::string deviceNode;
    int fileDescriptor;
    DeviceType type;
    
    USBDevice() : fileDescriptor(-1), type(DeviceType::UNKNOWN) {}
};

struct CurrentRXState {
    bool length_bytes_received = false;
    types::u16 expected_length = 0;
    types::u8 *data_buffer = nullptr;
    
    inline void reset(void) {
        length_bytes_received = false;
        expected_length = 0;
        if (data_buffer) {
            std::memset(data_buffer, 0, MAX_RX_BUF_SIZE);
        }
    }
};

/* ********** *
 * Main class *
 * ********** */

class CDC {
public:
    CDC();
    ~CDC();

    /**
     * @brief Initializes communication and starts background device scanning
     * @returns true if successfully initialized, false if not
     */
    bool init(void);

    /**
     * @brief Scans for connected ttyACM devices
     * @returns vector of detected devices
     */
    std::vector<USBDevice> scan_devices();

    /**
     * @brief Writes data, formatted correctly, will flush buffer.
     * @param type The type of device to write to
     * @param identifier Identifier for the sent data
     * @param data u8 array
     * @param data_len Length of data
     * @returns true if successfully sent, false if not
     */
    bool write(DeviceType type, const types::u8 identifier,
            const types::u8 *data, const types::u16 data_len);

    /**
     * @brief Register a callback function for receiving data from a specific device type
     * @param type The type of device to listen to
     * @param identifier Command identifier to listen for
     * @param callback Function to call when data with this identifier is received
     */
    void register_callback(DeviceType type, types::u8 identifier,
        std::function<void(const types::u8*, types::u16)> callback);
    
    /**
     * @brief Check if a specific device type is connected
     * @param type The device type to check
     * @returns true if connected, false if not
     */
    bool is_connected(DeviceType type);

private:
    /* **************** *
     * Private functions *
     * ***************** */
    
    /**
     * @brief Thread function for reading from device
     * @param device Device to read from
     */
    void read_thread(USBDevice device);
    
    /**
     * @brief Thread function for continuously scanning for devices
     */
    void scan_thread();
    
    /**
     * @brief Handles a BOARD_ID response from a device
     * @param board_id The board ID that was received
     */
    void handle_board_id(comms::BoardIdentifiers board_id);
    
    /**
     * @brief Checks if a device has already been identified
     * @param device_node Path to the device node (e.g. "/dev/ttyACM0")
     * @return true if identified, false otherwise
     */
    bool is_device_identified(const std::string &device_node);
    
    /**
     * @brief Attempts to identify a device by sending a BOARD_ID request
     * @param device Reference to the device to identify
     */
    void identify_device(USBDevice &device);
    
    /**
     * @brief Writes data to a specific device
     * @param device The device to write to
     * @param identifier Command identifier
     * @param data Data buffer
     * @param data_len Length of data
     * @return true if write successful, false otherwise
     */
    bool write_to_device(const USBDevice &device, types::u8 identifier, 
                         const types::u8 *data, types::u16 data_len);
    
    /**
     * @brief Attempts to connect to a device
     * @param device Reference to the device to connect to
     * @returns true if connection successful, false otherwise
     */
    bool connect(USBDevice& device);
    
    /**
     * @brief Closes connection to a device
     * @param device Reference to the device to disconnect
     */
    void disconnect(USBDevice& device);
    
    /**
     * @brief Process received data
     * @param device Source device
     * @param data Received data
     * @param length Data length
     */
    void process_data(const USBDevice& device, const types::u8* data, types::u16 length);
    
    /* *************************************************************** *
     * Private buffers, synchronization primitives and other variables *
     * *************************************************************** */
    
    // Main device references for each type
    std::map<DeviceType, USBDevice> _device_map;
    std::mutex _device_map_mutex;
    
    // Record of which device was last pinged for identification
    USBDevice _last_pinged_device;
    
    std::vector<std::thread> _read_threads;
    std::thread _scan_thread;
    
    // Callbacks for received data, mapped by device type and identifier
    std::map<DeviceType, 
             std::map<types::u8,
                     std::function<void(const types::u8*, types::u16)>>> _callbacks;
    std::mutex _callbacks_mutex;
    
    std::atomic<bool> _initialized;
    std::atomic<bool> _running;
};

} // namespace usb