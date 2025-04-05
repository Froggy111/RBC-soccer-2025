#include "comms/usb.hpp"
#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "debug.hpp"
#include <chrono>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace usb {

CDC::CDC() : _initialized(false) {}

CDC::~CDC() {
    // Clean up: close all open devices and stop threads
    std::lock_guard<std::mutex> lock(_devices_mutex);
    for (auto &device_pair : _devices) {
        if (device_pair.second) {
            device_pair.second->running = false;
            if (device_pair.second->rx_thread.joinable()) {
                device_pair.second->rx_thread.join();
            }
            if (device_pair.second->fd >= 0) {
                close(device_pair.second->fd);
            }
        }
    }
}

bool CDC::init() {
    if (_initialized) {
        return true;
    }

    addDebugCallbacks();

    debug::info("Initializing scan...");
    // Scan for Pico devices
    scanDevices();

    _initialized = !_devices.empty();
    return _initialized;
}

void CDC::addDebugCallbacks() {
    // Register debug message handlers for each board type
    registerBottomPicoHandler(comms::RecvBottomPicoIdentifiers::COMMS_DEBUG,
                              handle_debug);
    registerMiddlePicoHandler(comms::RecvMiddlePicoIdentifiers::COMMS_DEBUG,
                              handle_debug);
    registerTopPicoHandler(comms::RecvTopPicoIdentifiers::COMMS_DEBUG,
                           handle_debug);
    registerUnknownPicoHandler(
        (types::u8)comms::RecvBottomPicoIdentifiers::COMMS_DEBUG, handle_debug);

    registerBottomPicoHandler(comms::RecvBottomPicoIdentifiers::COMMS_WARN,
                              handle_debug);
    registerMiddlePicoHandler(comms::RecvMiddlePicoIdentifiers::COMMS_WARN,
                              handle_debug);
    registerTopPicoHandler(comms::RecvTopPicoIdentifiers::COMMS_WARN,
                           handle_debug);
    registerUnknownPicoHandler(
        (types::u8)comms::RecvBottomPicoIdentifiers::COMMS_WARN, handle_debug);

    registerBottomPicoHandler(comms::RecvBottomPicoIdentifiers::COMMS_ERROR,
                              handle_debug);
    registerMiddlePicoHandler(comms::RecvMiddlePicoIdentifiers::COMMS_ERROR,
                              handle_debug);
    registerTopPicoHandler(comms::RecvTopPicoIdentifiers::COMMS_ERROR,
                           handle_debug);
    registerUnknownPicoHandler(
        (types::u8)comms::RecvBottomPicoIdentifiers::COMMS_ERROR, handle_debug);
}

void CDC::handle_debug(const types::u8 *data, types::u16 data_len) {
    printf("%.*s", data_len, data);
}

void CDC::scanDevices() {
    DIR *dir;
    struct dirent *entry;

    dir = opendir("/dev");
    if (!dir) {
        debug::error("Failed to open /dev directory");
        return;
    }

    // Find all ttyACM devices
    std::vector<std::string> ttyACM_devices;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name(entry->d_name);
        if (name.find("ttyACM") != std::string::npos) {
            ttyACM_devices.push_back("/dev/" + name);
            debug::info("Found device: %s", name.c_str());
        }
    }
    closedir(dir);

    // Try to connect to each device and identify it
    for (const auto &port : ttyACM_devices) {
        int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

        if (fd < 0) {
            debug::error("Failed to open %s", port.c_str());
            continue;
        }

        // Set up serial port
        struct termios tty;
        memset(&tty, 0, sizeof(tty));

        if (tcgetattr(fd, &tty) != 0) {
            debug::error("Error from tcgetattr for %s", port.c_str());
            close(fd);
            continue;
        }

        // Set baud rate and other settings (8N1, no flow control)
        cfsetospeed(&tty, B115200);
        cfsetispeed(&tty, B115200);

        tty.c_cflag &= ~PARENB; // No parity
        tty.c_cflag &= ~CSTOPB; // 1 stop bit
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;      // 8 data bits
        tty.c_cflag &= ~CRTSCTS; // No hardware flow control
        tty.c_cflag |= CREAD | CLOCAL;

        tty.c_lflag &= ~ICANON; // No canonical mode
        tty.c_lflag &= ~ECHO;   // No echo
        tty.c_lflag &= ~ECHOE;  // No echo erase
        tty.c_lflag &= ~ECHONL; // No echo new line
        tty.c_lflag &= ~ISIG;   // No interpretation of INTR, QUIT, SUSP

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                         ICRNL); // No special handling of received bytes

        tty.c_oflag &= ~OPOST; // No output processing
        tty.c_oflag &= ~ONLCR; // No conversion of newline to CR/LF

        tty.c_cc[VTIME] = 0; // No timeout
        tty.c_cc[VMIN]  = 1; // Read at least 1 character

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            debug::error("Error from tcsetattr for %s", port.c_str());
            close(fd);
            continue;
        }

        // <<<--- START: Add this block for low latency mode --->>>
        struct serial_struct serial_info;
        if (ioctl(fd, TIOCGSERIAL, &serial_info) < 0) {
            // Error getting serial info - maybe not supported?
            // You might want to log this as a warning or just continue
            debug::warn("Could not get serial info for %s to set low_latency",
                        port.c_str());
        } else {
            serial_info.flags |= ASYNC_LOW_LATENCY; // Set the low latency flag
            if (ioctl(fd, TIOCSSERIAL, &serial_info) < 0) {
                // Error setting serial info - maybe not supported?
                debug::warn("Could not set low_latency mode for %s",
                            port.c_str());
            } else {
                debug::info("Enabled low latency mode for %s", port.c_str());
            }
        }
        // <<<--- END: Add this block for low latency mode --->>>

        // Create device object
        auto device        = std::make_shared<PicoDevice>();
        device->port       = port;
        device->fd         = fd;
        device->identified = false;
        device->running    = true;
        device->board_id   = comms::BoardIdentifiers::UNKNOWN;

        // Start RX thread for this device
        device->rx_thread =
            std::thread(&CDC::rxThreadFunc, this, std::ref(*device));

        // Send board ID request to identify the board
        types::u8 id_cmd =
            static_cast<types::u8>(comms::SendBottomPicoIdentifiers::BOARD_ID);
        debug::info("Sending BOARD_ID to %s", port.c_str());
        writeToPico(*device, &id_cmd, nullptr, 0);

        // If the board is identified, add it to our devices map
        uint16_t timeout = 1000;
        while (!device->identified && timeout > 0) {
            // Wait for identification
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            timeout -= 10;
        }

        if (device->identified) {
            // Add device to the map
            std::lock_guard<std::mutex> lock(_devices_mutex);
            _devices[device->board_id] = device;
            std::string identifier;
            switch (device->board_id) {
                case comms::BoardIdentifiers::BOTTOM_PICO:
                    identifier = "Bottom Pico";
                    break;
                case comms::BoardIdentifiers::MIDDLE_PICO:
                    identifier = "Middle Pico";
                    break;
                case comms::BoardIdentifiers::TOP_PICO:
                    identifier = "Top Pico";
                    break;
                default: identifier = "Unknown board"; break;
            }
            debug::info("Found %s on %s", identifier.c_str(), port.c_str());
        }
    }
}

void CDC::rxThreadFunc(PicoDevice &device) {
    types::u8 rx_buffer[MAX_RX_BUF_SIZE];
    types::u8 temp_buffer[MAX_RX_BUF_SIZE];
    size_t temp_buffer_pos = 0;

    while (device.running) {
        // Try to read data
        int bytes_avail;
        ioctl(device.fd, FIONREAD, &bytes_avail);

        if (bytes_avail > 0) {
            int n =
                read(device.fd, temp_buffer + temp_buffer_pos,
                     std::min(bytes_avail, static_cast<int>(MAX_RX_BUF_SIZE -
                                                            temp_buffer_pos)));
            if (n > 0) {
                temp_buffer_pos += n;

                // Process complete messages
                size_t processed_pos = 0;
                while (
                    processed_pos + 3 <=
                    temp_buffer_pos) { // At least length (2 bytes) + identifier (1 byte)
                    // Extract message length (little endian)
                    types::u16 msg_len = temp_buffer[processed_pos] |
                                         (temp_buffer[processed_pos + 1] << 8);

                    // Check if we have a complete message
                    if (processed_pos + 2 + msg_len <= temp_buffer_pos) {
                        // Extract identifier
                        types::u8 identifier = temp_buffer[processed_pos + 2];

                        // Handle board identification
                        if (identifier == static_cast<types::u8>(
                                              comms::RecvBottomPicoIdentifiers::
                                                  BOARD_ID) &&
                            msg_len == 2) { // 1 for identifier + 1 for board ID
                            comms::BoardIdentifiers board_id =
                                static_cast<comms::BoardIdentifiers>(
                                    temp_buffer[processed_pos + 3]);
                            device.board_id   = board_id;
                            device.identified = true;
                        }

                        processMessage(
                            device.board_id, identifier,
                            temp_buffer + processed_pos +
                                3, // Data starts after length and identifier
                            msg_len -
                                1); // Length includes identifier, so subtract 1

                        // Move to next message
                        processed_pos += 2 + msg_len;
                    } else {
                        // Incomplete message, wait for more data
                        break;
                    }
                }

                // Move any remaining data to the beginning of the buffer
                if (processed_pos < temp_buffer_pos) {
                    memmove(temp_buffer, temp_buffer + processed_pos,
                            temp_buffer_pos - processed_pos);
                    temp_buffer_pos -= processed_pos;
                } else {
                    temp_buffer_pos = 0;
                }
            }
        }

        // Small delay to avoid hogging the CPU
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void CDC::processMessage(comms::BoardIdentifiers board, types::u8 identifier,
                         const types::u8 *data, types::u16 data_len) {
    std::lock_guard<std::mutex> lock(_handlers_mutex);

    switch (board) {
        case comms::BoardIdentifiers::BOTTOM_PICO: {
            auto it = _bottom_pico_handlers.find(identifier);
            if (it != _bottom_pico_handlers.end()) {
                it->second(data, data_len);
            }
            break;
        }
        case comms::BoardIdentifiers::MIDDLE_PICO: {
            auto it = _middle_pico_handlers.find(identifier);
            if (it != _middle_pico_handlers.end()) {
                it->second(data, data_len);
            }
            break;
        }
        case comms::BoardIdentifiers::TOP_PICO: {
            auto it = _top_pico_handlers.find(identifier);
            if (it != _top_pico_handlers.end()) {
                it->second(data, data_len);
            }
            break;
        }
        case comms::BoardIdentifiers::UNKNOWN: {
            // Handle messages from unknown board types
            auto it = _unknown_pico_handlers.find(identifier);
            if (it != _unknown_pico_handlers.end()) {
                it->second(data, data_len);
            }
            break;
        }
    }
}

bool CDC::writeToPico(PicoDevice &device, const types::u8 *identifier_ptr,
                      const types::u8 *data, types::u16 data_len) {
    types::u16 reported_len =
        data_len + sizeof(*identifier_ptr); // +1 for identifier
    types::u16 packet_len = sizeof(reported_len) + reported_len;

    if (packet_len > MAX_TX_BUF_SIZE) {
        return false;
    }

    // Prepare packet
    types::u8 tx_buffer[MAX_TX_BUF_SIZE] = {0};

    // Write length (little endian)
    memcpy(tx_buffer, &reported_len, sizeof(reported_len));

    // Write identifier
    tx_buffer[sizeof(reported_len)] = *identifier_ptr;

    memcpy(&tx_buffer[3], data, data_len);

    // Send packet
    std::lock_guard<std::mutex> lock(device.tx_mutex);
    return write(device.fd, tx_buffer, packet_len) == packet_len;
}

bool CDC::writeToBottomPico(comms::SendBottomPicoIdentifiers identifier,
                            const types::u8 *data, types::u16 data_len) {
    std::lock_guard<std::mutex> lock(_devices_mutex);
    auto it = _devices.find(comms::BoardIdentifiers::BOTTOM_PICO);
    if (it == _devices.end()) {
        return false;
    }

    types::u8 id = static_cast<types::u8>(identifier);
    return writeToPico(*(it->second), &id, data, data_len);
}

bool CDC::writeToMiddlePico(comms::SendMiddlePicoIdentifiers identifier,
                            const types::u8 *data, types::u16 data_len) {
    std::lock_guard<std::mutex> lock(_devices_mutex);
    auto it = _devices.find(comms::BoardIdentifiers::MIDDLE_PICO);
    if (it == _devices.end()) {
        return false;
    }

    types::u8 id = static_cast<types::u8>(identifier);
    return writeToPico(*(it->second), &id, data, data_len);
}

bool CDC::writeToTopPico(comms::SendTopPicoIdentifiers identifier,
                         const types::u8 *data, types::u16 data_len) {
    std::lock_guard<std::mutex> lock(_devices_mutex);
    auto it = _devices.find(comms::BoardIdentifiers::TOP_PICO);
    if (it == _devices.end()) {
        return false;
    }

    types::u8 id = static_cast<types::u8>(identifier);
    return writeToPico(*(it->second), &id, data, data_len);
}

void CDC::registerBottomPicoHandler(comms::RecvBottomPicoIdentifiers identifier,
                                    MessageCallback callback) {
    std::lock_guard<std::mutex> lock(_handlers_mutex);
    _bottom_pico_handlers[static_cast<types::u8>(identifier)] = callback;
}

void CDC::registerMiddlePicoHandler(comms::RecvMiddlePicoIdentifiers identifier,
                                    MessageCallback callback) {
    std::lock_guard<std::mutex> lock(_handlers_mutex);
    _middle_pico_handlers[static_cast<types::u8>(identifier)] = callback;
}

void CDC::registerTopPicoHandler(comms::RecvTopPicoIdentifiers identifier,
                                 MessageCallback callback) {
    std::lock_guard<std::mutex> lock(_handlers_mutex);
    _top_pico_handlers[static_cast<types::u8>(identifier)] = callback;
}

void CDC::registerUnknownPicoHandler(types::u8 identifier,
                                     MessageCallback callback) {
    std::lock_guard<std::mutex> lock(_handlers_mutex);
    _unknown_pico_handlers[identifier] = callback;
}

} // namespace usb
