#include "comms/usb.hpp"
#include "comms/errors.hpp"
#include "comms/identifiers.hpp"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

namespace usb {

CDC::CDC() : _context(nullptr), _initialized(false), _running(false) {
    // Initialize device identifiers with default values
    _device_identifiers[DeviceType::TOP_PLATE] = {RP2040_VID, RP2040_PID};
    _device_identifiers[DeviceType::MIDDLE_PLATE] = {RP2040_VID, RP2040_PID};
    _device_identifiers[DeviceType::BOTTOM_PLATE] = {RP2040_VID, RP2040_PID};
}

CDC::~CDC() {
    _running = false;
    
    // Join all read threads
    for (auto& thread : _read_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Close all open devices
    for (auto& device : _connected_devices) {
        if (device.fileDescriptor >= 0) {
            close(device.fileDescriptor);
        }
    }
    
    if (_initialized) {
        libusb_exit(_context);
    }
}

bool CDC::init(void) {
    if (libusb_init(&_context) != 0) {
        return false;
    }
    
    _initialized = true;
    _running = true;
    return true;
}

std::vector<USBDevice> CDC::scan_devices() {
    std::vector<USBDevice> devices;
    
    if (!_initialized) {
        return devices;
    }
    
    libusb_device **dev_list;
    ssize_t cnt = libusb_get_device_list(_context, &dev_list);
    
    if (cnt < 0) {
        return devices;
    }
    
    for (ssize_t i = 0; i < cnt; i++) {
        libusb_device *device = dev_list[i];
        libusb_device_descriptor desc;
        
        if (libusb_get_device_descriptor(device, &desc) != 0) {
            continue;
        }
        
        // Check if this is a Raspberry Pi Pico device or matches any of our expected devices
        bool is_match = false;
        DeviceType device_type = DeviceType::UNKNOWN;
        
        for (const auto& pair : _device_identifiers) {
            if (desc.idVendor == pair.second.first && desc.idProduct == pair.second.second) {
                is_match = true;
                device_type = pair.first;
                break;
            }
        }
        
        if (!is_match) {
            continue;
        }
        
        USBDevice usb_device;
        usb_device.vid = desc.idVendor;
        usb_device.pid = desc.idProduct;
        usb_device.type = device_type;
        
        // Get additional device details
        libusb_device_handle *handle;
        if (libusb_open(device, &handle) == 0) {
            char string[256];
            
            if (desc.iSerialNumber) {
                if (libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, 
                    (unsigned char*)string, sizeof(string)) > 0) {
                    usb_device.serialNumber = string;
                }
            }
            
            if (desc.iManufacturer) {
                if (libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, 
                    (unsigned char*)string, sizeof(string)) > 0) {
                    usb_device.manufacturer = string;
                }
            }
            
            if (desc.iProduct) {
                if (libusb_get_string_descriptor_ascii(handle, desc.iProduct, 
                    (unsigned char*)string, sizeof(string)) > 0) {
                    usb_device.product = string;
                }
            }
            
            libusb_close(handle);
        }
        
        // Find the TTY device node
        usb_device.deviceNode = get_tty_device(usb_device.vid, usb_device.pid, usb_device.serialNumber);
        
        if (!usb_device.deviceNode.empty()) {
            devices.push_back(usb_device);
        }
    }
    
    libusb_free_device_list(dev_list, 1);
    return devices;
}

std::string CDC::get_tty_device(const types::u16 vid, const types::u16 pid, const std::string& serial) {
    // Check each USB device in /sys/bus/usb/devices/
    DIR *dir = opendir("/sys/bus/usb/devices/");
    if (!dir) {
        return "";
    }
    
    std::string result;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != nullptr) {
        std::string path = "/sys/bus/usb/devices/";
        path += entry->d_name;
        
        // Skip . and ..
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        // Check if this is a USB device
        std::string idVendorPath = path + "/idVendor";
        std::string idProductPath = path + "/idProduct";
        std::ifstream vendorFile(idVendorPath);
        std::ifstream productFile(idProductPath);
        
        if (!vendorFile.is_open() || !productFile.is_open()) {
            continue;
        }

        // Read the vendor and product IDs
        std::string vendorStr, productStr;
        vendorFile >> vendorStr;
        productFile >> productStr;
        
        // Convert hex strings to integers
        types::u16 deviceVid, devicePid;
        std::stringstream ss;
        ss << std::hex << vendorStr;
        ss >> deviceVid;
        ss.clear();
        ss << std::hex << productStr;
        ss >> devicePid;
        
        if (deviceVid == vid && devicePid == pid) {
            // Found matching VID/PID, now look for the tty device
            // First check if there's a matching serial number
            std::string serialPath = path + "/serial";
            std::ifstream serialFile(serialPath);
            std::string deviceSerial;
            
            if (serialFile.is_open()) {
                serialFile >> deviceSerial;
                if (!serial.empty() && deviceSerial != serial) {
                    continue; // Serial number doesn't match
                }
            }
            
            // Look for tty devices
            DIR *ttyDir = opendir(path.c_str());
            if (ttyDir) {
                struct dirent *ttyEntry;
                while ((ttyEntry = readdir(ttyDir)) != nullptr) {
                    std::string ttyName = ttyEntry->d_name;
                    if (ttyName.find("tty") != std::string::npos) {
                        // Check if this is a CDC ACM device
                        std::string ttyPath = path + "/" + ttyName;
                        if (ttyName.find("ttyACM") != std::string::npos) {
                            result = "/dev/" + ttyName;
                            break;
                        }
                    }
                }
                closedir(ttyDir);
            }
            
            if (!result.empty()) {
                break;
            }
        }
    }
    
    closedir(dir);
    return result;
}

bool CDC::connect(USBDevice& device) {
    if (device.deviceNode.empty()) {
        return false;
    }
    
    // Check if already connected
    std::lock_guard<std::mutex> lock(_devices_mutex);
    for (const auto& connected : _connected_devices) {
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
    tty.c_cflag &= ~PARENB;          // No parity
    tty.c_cflag &= ~CSTOPB;          // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;              // 8 bits
    tty.c_cflag &= ~CRTSCTS;         // No hardware flow control
    tty.c_cflag |= CREAD | CLOCAL;   // Enable receiver, ignore modem control lines
    
    // Raw input
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    // Raw output
    tty.c_oflag &= ~OPOST;
    
    // No software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    // No special handling of bytes
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    
    // Non-blocking read with timeout
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1; // 0.1 seconds
    
    // Apply the settings
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        return false;
    }
    
ccc    device.fileDescriptor = fd;
    _connected_devices.push_back(device);
    
    // Start the read thread
    _read_threads.emplace_back(&CDC::read_thread, this, device);
    
    return true;
}

void CDC::disconnect(USBDevice& device) {
    std::lock_guard<std::mutex> lock(_devices_mutex);
    
    // Find the device
    auto it = std::find_if(_connected_devices.begin(), _connected_devices.end(),
        [&device](const USBDevice& d) {
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

void CDC::set_device_identifiers(DeviceType type, types::u16 vid, types::u16 pid) {
    _device_identifiers[type] = {vid, pid};
}

bool CDC::write(const USBDevice& device, const comms::SendIdentifiers identifier,
                const types::u8 *data, const types::u16 data_len) {
    if (device.fileDescriptor < 0) {
        return false;
    }
    
    types::u16 reported_len = data_len + sizeof(identifier);
    types::u16 packet_len = sizeof(reported_len) + reported_len;
    
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

void CDC::register_callback(comms::RecvIdentifiers identifier, 
                          std::function<void(const USBDevice&, const types::u8*, types::u16)> callback) {
    std::lock_guard<std::mutex> lock(_callbacks_mutex);
    _callbacks[identifier] = callback;
}

void CDC::read_thread(USBDevice device) {
    types::u8 buffer[MAX_RX_BUF_SIZE];
    CurrentRXState state;
    state.data_buffer = new types::u8[MAX_RX_BUF_SIZE];
    
    while (_running) {
        // Check if the device is still in our list
        {
            std::lock_guard<std::mutex> lock(_devices_mutex);
            auto it = std::find_if(_connected_devices.begin(), _connected_devices.end(),
                [&device](const USBDevice& d) {
                    return d.deviceNode == device.deviceNode;
                });
            
            if (it == _connected_devices.end()) {
                break;
            }
        }
        
        // Read from device
        ssize_t bytes_read = read(device.fileDescriptor, buffer, sizeof(buffer));
        
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

void CDC::process_data(const USBDevice& device, const types::u8* data, types::u16 length) {
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
            types::u16 remaining = state.expected_length - (state.data_buffer - data_buffer);
            types::u16 available = length - offset;
            types::u16 to_read = std::min(remaining, available);
            
            // Copy data to buffer
            memcpy(state.data_buffer, data + offset, to_read);
            state.data_buffer += to_read;
            offset += to_read;
            
            // Check if we've read all the data
            if (state.data_buffer - data_buffer == state.expected_length) {
                // Complete packet received
                types::u8 identifier = data_buffer[0];
                
                // Find and call the appropriate callback
                std::lock_guard<std::mutex> lock(_callbacks_mutex);
                auto it = _callbacks.find(static_cast<comms::RecvIdentifiers>(identifier));
                if (it != _callbacks.end()) {
                    it->second(device, data_buffer + 1, state.expected_length - 1);
                }
                
                // Reset state for next packet
                state.reset();
                state.data_buffer = data_buffer;
            }
        }
    }
}

} // namespace usb