#include "comms/usb.hpp"
#include "comms.hpp"
#include "comms/errors.hpp"
#include "comms/identifiers.hpp"
#include "debug.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <regex>
#include <termios.h>
#include <unistd.h>

namespace usb {

// Map to store the device nodes by board ID
static std::map<comms::BoardIdentifiers, std::string> boardDeviceNodeMap;

CDC::CDC() : _initialized(false), _running(false) {}

CDC::~CDC() {
    _running = false;

    // Join scan thread
    if (_scan_thread.joinable()) {
        _scan_thread.join();
    }

    // Join all read threads
    for (auto &thread : _read_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Close all open devices
    std::lock_guard<std::mutex> lock(_device_map_mutex);
    for (auto &pair : _device_map) {
        if (pair.second.fileDescriptor >= 0) {
            close(pair.second.fileDescriptor);
        }
    }
}

bool CDC::init(void) {
    _initialized = true;
    _running     = true;

    // Register a callback for BOARD_ID responses
    register_callback(
        DeviceType::UNKNOWN,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::BOARD_ID),
        [this](const types::u8 *data, types::u16 length) {
            debug::info("Received BOARD_ID response, %d bytes", length);
            if (length >= 1) {
                // Process received board ID
                handle_board_id(static_cast<comms::BoardIdentifiers>(data[0]));
            }
        });

    // Start the background scanning thread
    _scan_thread = std::thread(&CDC::scan_thread, this);
    attach_debug_listeners();

    return true;
}

void CDC::attach_debug_listeners() {
    // ^ Very scuffed way of attaching debug listeners, but it works

    register_callback(
        DeviceType::UNKNOWN,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_DEBUG),
        log_pico);

    register_callback(
        DeviceType::UNKNOWN,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_ERROR),
        log_pico);

    register_callback(
        DeviceType::UNKNOWN,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_WARN),
        log_pico);

    register_callback(
        DeviceType::BOTTOM_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_DEBUG),
        log_pico);

    register_callback(
        DeviceType::BOTTOM_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_ERROR),
        log_pico);

    register_callback(
        DeviceType::BOTTOM_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_WARN),
        log_pico);

    register_callback(
        DeviceType::MIDDLE_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_DEBUG),
        log_pico);

    register_callback(
        DeviceType::MIDDLE_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_ERROR),
        log_pico);

    register_callback(
        DeviceType::MIDDLE_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_WARN),
        log_pico);

    register_callback(
        DeviceType::TOP_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_DEBUG),
        log_pico);

    register_callback(
        DeviceType::TOP_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_ERROR),
        log_pico);

    register_callback(
        DeviceType::TOP_PLATE,
        static_cast<types::u8>(comms::RecvBottomPicoIdentifiers::COMMS_WARN),
        log_pico);
}

void CDC::log_pico(const types::u8 *data, types::u16 length) {
    if (length > 0) {
        std::string message(reinterpret_cast<const char *>(data), length);
        printf("%s", message.c_str());
    }
}

void CDC::handle_board_id(comms::BoardIdentifiers board_id) {
    // This will be called when a device responds to the BOARD_ID request

    // Safely access last pinged device
    std::lock_guard<std::mutex> lock(_device_map_mutex);

    if (_last_pinged_device.deviceNode.empty()) {
        return;
    }

    DeviceType device_type;

    // Map board ID to device type
    switch (board_id) {
        case comms::BoardIdentifiers::BOTTOM_PICO:
            device_type = DeviceType::BOTTOM_PLATE;
            break;
        case comms::BoardIdentifiers::MIDDLE_PICO:
            device_type = DeviceType::MIDDLE_PLATE;
            break;
        case comms::BoardIdentifiers::TOP_PICO:
            device_type = DeviceType::TOP_PLATE;
            break;
        default: return; // Unknown board ID
    }

    // Store the board device node association
    boardDeviceNodeMap[board_id] = _last_pinged_device.deviceNode;

    // Update device information
    _last_pinged_device.type = device_type;

    // Add or update device in our map (already have mutex locked)
    _device_map[device_type] = _last_pinged_device;

    debug::info("Device %s identified as %d",
                _last_pinged_device.deviceNode.c_str(),
                static_cast<int>(device_type));
}

void CDC::scan_thread() {
    while (_running) {
        // Get all ttyACM devices
        auto devices = scan_devices();

        // Try to identify each device by pinging it
        for (auto &device : devices) {
            // Skip devices already identified
            if (!is_device_identified(device.deviceNode)) {
                identify_device(device);
            }
        }

        // Sleep for a bit before scanning again
        usleep(DEVICE_SCAN_INTERVAL * 1000);
    }
}

bool CDC::is_device_identified(const std::string &device_node) {
    std::lock_guard<std::mutex> lock(_device_map_mutex);
    for (const auto &pair : _device_map) {
        if (pair.second.deviceNode == device_node &&
            pair.second.type != DeviceType::UNKNOWN) {
            return true;
        }
    }
    return false;
}

void CDC::identify_device(USBDevice &device) {
    // First try to connect to the device
    if (!connect(device)) {
        return;
    }

    // Store as the last pinged device with mutex protection
    {
        std::lock_guard<std::mutex> lock(_device_map_mutex);
        _last_pinged_device = device;
    }

    // Send BOARD_ID request
    types::u8 ping_data = 0;
    debug::info("Sending BOARD_ID request to %s", device.deviceNode.c_str());
    write_to_device(
        device,
        static_cast<types::u8>(comms::SendBottomPicoIdentifiers::BOARD_ID),
        &ping_data, 1);

    // Wait for response (handled by callback registered in init)
    usleep(100000); // Wait 100ms for response
}

bool CDC::write_to_device(const USBDevice &device, types::u8 identifier,
                          const types::u8 *data, types::u16 data_len) {
    if (device.fileDescriptor < 0) {
        return false;
    }

    types::u16 reported_len = data_len + sizeof(identifier);
    types::u16 packet_len   = sizeof(reported_len) + reported_len;

    if (packet_len > MAX_TX_BUF_SIZE) {
        return false;
    }

    // Create the packet buffer
    types::u8 buffer[MAX_TX_BUF_SIZE];
    size_t offset = 0;

    // Copy length
    memcpy(buffer + offset, &reported_len, sizeof(reported_len));
    offset += sizeof(reported_len);

    // Copy identifier
    memcpy(buffer + offset, &identifier, sizeof(identifier));
    offset += sizeof(identifier);

    // Copy data
    memcpy(buffer + offset, data, data_len);
    offset += data_len;

    // Write to device
    ssize_t written = ::write(device.fileDescriptor, buffer, offset);
    return written == offset;
}

std::vector<USBDevice> CDC::scan_devices() {
    std::vector<USBDevice> devices;

    if (!_initialized) {
        return devices;
    }

    // Scan /dev for ttyACM devices
    DIR *dir = opendir("/dev");
    if (!dir) {
        return devices;
    }

    struct dirent *entry;
    std::regex ttyACMPattern("ttyACM[0-9]+");

    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (std::regex_match(name, ttyACMPattern)) {
            std::string devicePath = "/dev/" + name;

            // Create a basic device structure - we'll identify it later
            USBDevice device;
            device.deviceNode = devicePath;
            device.type       = DeviceType::UNKNOWN;

            devices.push_back(device);
        }
    }

    closedir(dir);
    return devices;
}

bool CDC::connect(USBDevice &device) {
    if (device.deviceNode.empty()) {
        return false;
    }

    // Open the device
    int fd = open(device.deviceNode.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        return false;
    }

    // Configure the terminal
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    // Get current terminal attributes
    if (tcgetattr(fd, &tty) != 0) {
        close(fd);
        return false;
    }

    // Set baud rate (115200)
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // 8N1 mode, no flow control
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;      // 8 bits
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control
    tty.c_cflag |=
        CREAD | CLOCAL; // Enable receiver, ignore modem control lines

    // Raw input
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Raw output
    tty.c_oflag &= ~OPOST;

    // No software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // No special handling of bytes
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // Non-blocking read with timeout
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1; // 0.1 seconds

    // Apply the settings
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        return false;
    }

    device.fileDescriptor = fd;

    // Start the read thread
    _read_threads.emplace_back(&CDC::read_thread, this, device);

    return true;
}

void CDC::disconnect(USBDevice &device) {
    // Close the file descriptor
    if (device.fileDescriptor >= 0) {
        close(device.fileDescriptor);
        device.fileDescriptor = -1;
    }
}
bool CDC::write(DeviceType type, types::u8 identifier, const types::u8 *data,
                const types::u16 data_len) {
    std::lock_guard<std::mutex> lock(_device_map_mutex);

    // Find the device of this type
    auto it = _device_map.find(type);
    if (it == _device_map.end() || it->second.fileDescriptor < 0) {
        return false; // Device not found or not connected
    }

    const USBDevice &device = it->second;
    return write_to_device(device, identifier, data, data_len);
}

void CDC::register_callback(
    DeviceType type, types::u8 identifier,
    std::function<void(const types::u8 *, types::u16)> callback) {
    std::lock_guard<std::mutex> lock(_callbacks_mutex);
    _callbacks[type][identifier] = callback;
}

bool CDC::is_connected(DeviceType type) {
    std::lock_guard<std::mutex> lock(_device_map_mutex);
    auto it = _device_map.find(type);
    return (it != _device_map.end() && it->second.fileDescriptor >= 0);
}

void CDC::read_thread(USBDevice device) {
    types::u8 buffer[MAX_RX_BUF_SIZE];

    device.rxState.data_buffer = new types::u8[MAX_RX_BUF_SIZE];

    while (_running) {
        bool deviceValid = false;

        // Check if the device is still valid
        {
            std::lock_guard<std::mutex> lock(_device_map_mutex);

            // First check if this is the last pinged device
            if (_last_pinged_device.deviceNode == device.deviceNode &&
                _last_pinged_device.fileDescriptor >= 0) {
                deviceValid = true;
            }
            // For UNKNOWN devices, check if they're in any slot in the map
            else if (device.type == DeviceType::UNKNOWN) {
                for (const auto &pair : _device_map) {
                    if (pair.second.deviceNode == device.deviceNode &&
                        pair.second.fileDescriptor >= 0) {
                        deviceValid = true;
                        break;
                    }
                }
            } else {
                // For identified devices
                auto it = _device_map.find(device.type);
                if (it != _device_map.end() &&
                    it->second.deviceNode == device.deviceNode &&
                    it->second.fileDescriptor >= 0) {
                    deviceValid = true;
                }
            }

            if (!deviceValid) {
                break;
            }
        }

        // Rest of the function stays the same
        ssize_t bytes_read =
            read(device.fileDescriptor, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            // Process the received data
            process_data(device, buffer, bytes_read);
        } else if (bytes_read < 0) {
            usleep(10000); // Wait 10ms before trying again
        } else {
            // No data
            usleep(10000); // Wait 10ms before trying again
        }
    }

    delete[] device.rxState.data_buffer;
}

void CDC::process_data(USBDevice &device, const types::u8 *data,
                       types::u16 length) {
    // Use the device's own state instead of a static one
    CurrentRXState &state = device.rxState;

    if (state.data_buffer == nullptr) {
        state.data_buffer = new types::u8[MAX_RX_BUF_SIZE];
    }

    size_t offset = 0;
    while (offset < length) {
        // If we haven't received length bytes yet
        if (!state.length_bytes_received) {
            // Check if we have enough data for the length
            if (offset + N_LENGTH_BYTES <= length) {
                // Read the length bytes
                memcpy(&state.expected_length, data + offset, N_LENGTH_BYTES);
                offset += N_LENGTH_BYTES;
                state.length_bytes_received = true;
                state.bytes_received        = 0; // Reset the counter

                // Check if length is valid
                if (state.expected_length > MAX_RX_BUF_SIZE) {
                    // Invalid length, reset state
                    state.reset();
                    continue;
                }
            } else {
                // Not enough data for length, wait for more
                break;
            }
        } else {
            // We have the length, now read the data
            types::u16 remaining = state.expected_length - state.bytes_received;
            types::u16 available = length - offset;
            types::u16 to_read   = std::min(remaining, available);

            // Copy data to buffer
            memcpy(state.data_buffer + state.bytes_received, data + offset,
                   to_read);
            state.bytes_received += to_read;
            offset += to_read;

            // Check if we've read all the data
            if (state.bytes_received == state.expected_length) {
                // Complete packet received
                types::u8 recv_id = state.data_buffer[0];

                // Find and call the appropriate callback
                std::lock_guard<std::mutex> lock(_callbacks_mutex);

                // First check for UNKNOWN device callbacks (used for identification)
                auto unknown_callbacks = _callbacks.find(DeviceType::UNKNOWN);
                if (unknown_callbacks != _callbacks.end()) {
                    auto callback_it = unknown_callbacks->second.find(recv_id);
                    if (callback_it != unknown_callbacks->second.end()) {
                        // Found a callback for UNKNOWN device type, execute it
                        callback_it->second(state.data_buffer + 1,
                                            state.expected_length - 1);
                    }
                }

                // Then check for this specific device type callbacks
                auto device_callbacks = _callbacks.find(device.type);
                if (device_callbacks != _callbacks.end()) {
                    // Look for a callback for this identifier
                    auto callback_it = device_callbacks->second.find(recv_id);
                    if (callback_it != device_callbacks->second.end()) {
                        // Found a callback, execute it
                        callback_it->second(state.data_buffer + 1,
                                            state.expected_length - 1);
                    }
                }

                // Reset state for next packet
                state.reset();
            }
        }
    }
}

} // namespace usb