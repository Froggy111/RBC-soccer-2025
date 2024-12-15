#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <time.h>

#define VENDOR_ID  0x2E8A// Replace with your device's Vendor ID
#define PRODUCT_ID 0x000A// Replace with your device's Product ID
#define ENDPOINT_IN  0x81 // IN endpoint address (device to host)
#define ENDPOINT_OUT 0x01 // OUT endpoint address (host to device)
#define TIMEOUT_MS 10

// Measure current time in microseconds
unsigned long long current_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}

int main() {
    libusb_context *ctx;
    libusb_device_handle *handle;
    unsigned char data_out[64] = {0}; // Example data to send
    unsigned char data_in[64];       // Buffer for response
    int transferred;
    int res;

    // Initialize libusb
    res = libusb_init(&ctx);
    if (res < 0) {
        fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(res));
        return 1;
    }

    // Open the USB device
    handle = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    if (!handle) {
        fprintf(stderr, "Failed to open USB device\n");
        libusb_exit(ctx);
        return 1;
    }

    // Claim the interface
    res = libusb_claim_interface(handle, 0);
    if (res < 0) {
        fprintf(stderr, "Failed to claim interface: %s\n", libusb_error_name(res));
        libusb_close(handle);
        libusb_exit(ctx);
        return 1;
    }

    // Perform latency test
    unsigned long long start_time, end_time;
    for (int i = 0; i < 10; i++) {
        start_time = current_time_us();

        // Send data to the device
        res = libusb_bulk_transfer(handle, ENDPOINT_OUT, data_out, sizeof(data_out), &transferred, TIMEOUT_MS);
        if (res < 0) {
            fprintf(stderr, "Error in OUT transfer: %s\n", libusb_error_name(res));
            break;
        }

        // Receive response from the device
        res = libusb_bulk_transfer(handle, ENDPOINT_IN, data_in, sizeof(data_in), &transferred, TIMEOUT_MS);
        if (res < 0) {
            fprintf(stderr, "Error in IN transfer: %s\n", libusb_error_name(res));
            break;
        }

        end_time = current_time_us();
        printf("Round-trip latency: %llu microseconds\n", end_time - start_time);
    }

    // Release the interface and close the device
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);

    return 0;
}
