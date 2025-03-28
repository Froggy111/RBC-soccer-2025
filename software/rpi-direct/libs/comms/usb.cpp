#include "comms/usb.hpp"
#include "comms/errors.hpp"
#include "comms/identifiers.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <termios.h>
#include <unistd.h>

namespace usb {

CDC::CDC() : _initialized(false), _running(false) {
    // Initialize device identifiers with default values
    _device_identifiers[DeviceType::TOP_PLATE]    = {RP2040_VID, RP2040_PID};
    _device_identifiers[DeviceType::MIDDLE_PLATE] = {RP2040_VID, RP2040_PID};
    _device_identifiers[DeviceType::BOTTOM_PLATE] = {RP2040_VID, RP2040_PID};
}

CDC::~CDC() {
    _running = false;

    // Join all read threads
    for (auto &thread : _read_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Close all open devices
    for (auto &device : _connected_devices) {
        if (device.fileDescriptor >= 0) {
            close(device.fileDescriptor);
        }
    }
}

bool CDC::init(void) {
    _initialized = true;
    _running     = true;
    return true;
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
            USBDevice device       = get_device_info(devicePath);

            // Check if device matches any of our expected devices
            bool is_match = false;
            for (const auto &pair : _device_identifiers) {
                if (device.vid == pair.second.first &&
                    device.pid == pair.second.second) {
                    device.type = pair.first;
                    is_match    = true;
                    break;
                }
            }

            if (is_match && device.vid != 0) {
                devices.push_back(device);
            }
        }
    }

    closedir(dir);
    return devices;
}

std::string CDC::read_sysfs_value(const std::string &path) {
    std::ifstream file(path);
    std::string value;

    if (file.is_open()) {
        std::getline(file, value);
        file.close();
    }

    return value;
}

USBDevice CDC::get_device_info(const std::string &ttyDevice) {
    USBDevice device;
    device.deviceNode = ttyDevice;

    // Extract the ACM number
    std::string acmNum = ttyDevice.substr(ttyDevice.find("ttyACM") + 6);

    // Find the USB device by looking at the symbolic link
    std::string devPath = "/sys/class/tty/ttyACM" + acmNum + "/device";

    // Follow symlinks to get to the USB device
    try {
        std::filesystem::path symlinkPath(devPath);
        std::filesystem::path realPath =
            std::filesystem::canonical(symlinkPath);

        // Find USB path by going up the directory tree
        std::filesystem::path usbPath = realPath;
        while (!usbPath.empty()) {
            std::string idVendorPath  = (usbPath / "idVendor").string();
            std::string idProductPath = (usbPath / "idProduct").string();

            // Check if this is a USB device with VID/PID
            if (std::filesystem::exists(idVendorPath) &&
                std::filesystem::exists(idProductPath)) {
                // Read the VID and PID
                std::string vidStr = read_sysfs_value(idVendorPath);
                std::string pidStr = read_sysfs_value(idProductPath);

                if (!vidStr.empty() && !pidStr.empty()) {
                    // Convert hex string to integer
                    device.vid = std::stoi(vidStr, nullptr, 16);
                    device.pid = std::stoi(pidStr, nullptr, 16);

                    // Read additional properties
                    device.path = usbPath.string();
                    device.manufacturer =
                        read_sysfs_value((usbPath / "manufacturer").string());
                    device.product =
                        read_sysfs_value((usbPath / "product").string());
                    device.serialNumber =
                        read_sysfs_value((usbPath / "serial").string());

                    break;
                }
            }

            // Go up one directory
            if (usbPath.has_parent_path()) {
                usbPath = usbPath.parent_path();
            } else {
                break;
            }
        }
    } catch (const std::exception &e) {
        // Failed to follow symlink, leave device with default values
    }

    return device;
}

bool CDC::connect(USBDevice &device) {
    if (device.deviceNode.empty()) {
        return false;
    }

    // Check if already connected
    std::lock_guard<std::mutex> lock(_devices_mutex);
    for (const auto &connected : _connected_devices) {
        if (connected.deviceNode == device.deviceNode) {
            // Already connected
            device = connected;
            return true;
        }
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
    _connected_devices.push_back(device);

    // Start the read thread
    _read_threads.emplace_back(&CDC::read_thread, this, device);

    return true;
}

void CDC::disconnect(USBDevice &device) {
    std::lock_guard<std::mutex> lock(_devices_mutex);

    // Find the device
    auto it = std::find_if(_connected_devices.begin(), _connected_devices.end(),
                           [&device](const USBDevice &d) {
                               return d.deviceNode == device.deviceNode;
                           });

    if (it != _connected_devices.end()) {
        // Close the file descriptor
        if (it->fileDescriptor >= 0) {
            close(it->fileDescriptor);
        }

        // Remove from the list
        _connected_devices.erase(it);
    }

    // Reset the device
    device.fileDescriptor = -1;
}

void CDC::set_device_identifiers(DeviceType type, types::u16 vid,
                                 types::u16 pid) {
    _device_identifiers[type] = {vid, pid};
}

bool CDC::write(const USBDevice &device,
                types::u8 identifier, const types::u8 *data,
                const types::u16 data_len) {
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

void CDC::register_callback(
    const USBDevice &device, types::u8 identifier,
    std::function<void(const types::u8 *, types::u16)> callback) {
    std::lock_guard<std::mutex> lock(_callbacks_mutex);
    _device_callbacks[device.deviceNode][identifier] = callback;
}

void CDC::read_thread(USBDevice device) {
    types::u8 buffer[MAX_RX_BUF_SIZE];
    CurrentRXState state;
    state.data_buffer = new types::u8[MAX_RX_BUF_SIZE];

    while (_running) {
        // Check if the device is still in our list
        {
            std::lock_guard<std::mutex> lock(_devices_mutex);
            auto it = std::find_if(_connected_devices.begin(),
                                   _connected_devices.end(),
                                   [&device](const USBDevice &d) {
                                       return d.deviceNode == device.deviceNode;
                                   });

            if (it == _connected_devices.end()) {
                break;
            }
        }

        // Read from device
        ssize_t bytes_read =
            read(device.fileDescriptor, buffer, sizeof(buffer));

        if (bytes_read <= 0) {
            // No data or error
            usleep(10000); // Wait 10ms before trying again
            continue;
        }

        // Process the received data
        process_data(device, buffer, bytes_read);
    }

    delete[] state.data_buffer;
}

void CDC::process_data(const USBDevice &device, const types::u8 *data,
                       types::u16 length) {
    static CurrentRXState state;
    static types::u8 data_buffer[MAX_RX_BUF_SIZE];

    if (state.data_buffer == nullptr) {
        state.data_buffer = data_buffer;
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
            types::u16 remaining =
                state.expected_length - (state.data_buffer - data_buffer);
            types::u16 available = length - offset;
            types::u16 to_read   = std::min(remaining, available);

            // Copy data to buffer
            memcpy(state.data_buffer, data + offset, to_read);
            state.data_buffer += to_read;
            offset += to_read;

            // Check if we've read all the data
            if (state.data_buffer - data_buffer == state.expected_length) {
                // Complete packet received
                types::u8 recv_id = data_buffer[0];

                // Find and call the appropriate callback
                std::lock_guard<std::mutex> lock(_callbacks_mutex);

                // Check for device-specific callback first
                auto device_it = _device_callbacks.find(device.deviceNode);
                if (device_it != _device_callbacks.end()) {
                    auto callback_it = device_it->second.find(recv_id);
                    if (callback_it != device_it->second.end()) {
                        // We found a device-specific callback for this identifier
                        callback_it->second(data_buffer + 1,
                                            state.expected_length - 1);
                    }
                }

                // Reset state for next packet
                state.reset();
                state.data_buffer = data_buffer;
            }
        }
    }
}

} // namespace usb