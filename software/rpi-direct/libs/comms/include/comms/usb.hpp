#pragma once

#include "comms/default_usb_config.h"
#include "identifiers.hpp"
#include "types.hpp"
#include <cstring>
#include <libusb-1.0/libusb.h>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <map>

namespace usb {

/* ******** *
 * Settings *
 * ******** */

static const types::u32 DEVICE_SCAN_TIMEOUT = 1000; // in milliseconds
static const types::u32 CDC_CONNECTION_TIMEOUT = 1000;  // in milliseconds

static const types::u16 MAX_RX_BUF_SIZE = USB_RX_BUFSIZE;
static const types::u16 MAX_TX_BUF_SIZE = USB_TX_BUFSIZE;

static const types::u8 N_LENGTH_BYTES = 2;
static const types::u16 MAX_RX_PACKET_LENGTH = MAX_RX_BUF_SIZE;
static const types::u16 MAX_TX_PACKET_LENGTH = MAX_TX_BUF_SIZE;

// Raspberry Pi Pico identifiers
static const types::u16 RP2040_VID = 0x2E8A; // Raspberry Pi
static const types::u16 RP2040_PID = 0x000a; // Pico SDK CDC

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
    types::u16 vid;
    types::u16 pid;
    std::string serialNumber;
    std::string manufacturer;
    std::string product;
    int fileDescriptor;
    DeviceType type;
    
    USBDevice() : vid(0), pid(0), fileDescriptor(-1), type(DeviceType::UNKNOWN) {}
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
     * @brief Initializes libusb and scanning thread
     * @returns true if successfully initialized, false if not
     */
    bool init(void);

    /**
     * @brief Scans for connected Pico devices
     * @returns vector of detected devices
     */
    std::vector<USBDevice> scan_devices();
    
    /**
     * @brief Opens a connection to a specific device
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
     * @brief Sets the expected VID/PID for a specific plate type
     * @param type The plate type
     * @param vid Vendor ID
     * @param pid Product ID
     */
    void set_device_identifiers(DeviceType type, types::u16 vid, types::u16 pid);

    /**
     * @brief Writes data, formatted correctly, will flush buffer.
     * @param device Device to write to
     * @param identifier Identifier for the sent data
     * @param data u8 array
     * @param data_len Length of data
     * @returns true if successfully sent, false if not
     */
    bool write(const USBDevice& device, const comms::SendIdentifiers identifier,
               const types::u8 *data, const types::u16 data_len);

    /**
     * @brief Just a regular printf to the specified device
     * @returns false if device not connected
     */
    bool printf(const USBDevice& device, const char *format, ...);

    /**
     * @brief Register a callback function for receiving data
     * @param identifier Command identifier to listen for
     * @param callback Function to call when data with this identifier is received
     */
    void register_callback(comms::RecvIdentifiers identifier, 
                          std::function<void(const USBDevice&, const types::u8*, types::u16)> callback);

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
     * @brief Process received data
     * @param device Source device
     * @param data Received data
     * @param length Data length
     */
    void process_data(const USBDevice& device, const types::u8* data, types::u16 length);

    /**
     * @brief Gets TTY device node for a USB device
     */
    std::string get_tty_device(const types::u16 vid, const types::u16 pid, const std::string& serial);
    
    /* *************************************************************** *
     * Private buffers, synchronization primitives and other variables *
     * *************************************************************** */
    
    libusb_context* _context;
    std::vector<USBDevice> _connected_devices;
    std::vector<std::thread> _read_threads;
    std::mutex _devices_mutex;
    
    // Device type to VID/PID mapping
    std::map<DeviceType, std::pair<types::u16, types::u16>> _device_identifiers;
    
    // Callbacks for received data
    std::map<comms::RecvIdentifiers, 
             std::function<void(const USBDevice&, const types::u8*, types::u16)>> _callbacks;
    std::mutex _callbacks_mutex;
    
    bool _initialized;
    bool _running;
};

} // namespace usb