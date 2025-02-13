#ifndef STDIO_CONFIG_H
#define STDIO_CONFIG_H

#ifndef PICO_STDIO_USB_DEFAULT_CRLF
#define PICO_STDIO_USB_DEFAULT_CRLF PICO_STDIO_DEFAULT_CRLF
#endif

// PICO_CONFIG: PICO_STDIO_USB_STDOUT_TIMEOUT_US, Number of microseconds to be
// blocked trying to write USB output before assuming the host has disappeared
// and discarding data, default=500000, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_STDOUT_TIMEOUT_US
#define PICO_STDIO_USB_STDOUT_TIMEOUT_US 500000
#endif

// todo perhaps unnecessarily frequent?
// PICO_CONFIG: PICO_STDIO_USB_TASK_INTERVAL_US, Period of microseconds between
// calling tud_task in the background, default=1000, advanced=true,
// group=pico_stdio_usb
#ifndef PICO_STDIO_USB_TASK_INTERVAL_US
#define PICO_STDIO_USB_TASK_INTERVAL_US 1000
#endif

// PICO_CONFIG: PICO_STDIO_USB_LOW_PRIORITY_IRQ, Explicit User IRQ number to
// claim for tud_task() background execution instead of letting the
// implementation pick a free one dynamically (deprecated), advanced=true,
// group=pico_stdio_usb
#ifndef PICO_STDIO_USB_LOW_PRIORITY_IRQ
// this variable is no longer set by default (one is claimed dynamically), but
// will be respected if specified
#endif

// PICO_CONFIG: PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE, Enable/disable
// resetting into BOOTSEL mode if the host sets the baud rate to a magic value
// (PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE), type=bool, default=1 if application
// is not using TinyUSB directly, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE
#define PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE 1
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE, Baud rate that if selected
// causes a reset into BOOTSEL mode (if
// PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE is set), default=1200,
// group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE
#define PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE 1200
#endif

// PICO_CONFIG: PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS, Maximum number of
// milliseconds to wait during initialization for a CDC connection from the host
// (negative means indefinite) during initialization, default=0,
// group=pico_stdio_usb
#ifndef PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS
#define PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 0
#endif

// PICO_CONFIG: PICO_STDIO_USB_POST_CONNECT_WAIT_DELAY_MS, Number of extra
// milliseconds to wait when using PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS after
// a host CDC connection is detected (some host terminals seem to sometimes lose
// transmissions sent right after connection), default=50, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_POST_CONNECT_WAIT_DELAY_MS
#define PICO_STDIO_USB_POST_CONNECT_WAIT_DELAY_MS 50
#endif

// PICO_CONFIG: PICO_STDIO_USB_DEINIT_DELAY_MS, Number of milliseconds to wait
// before deinitializing stdio_usb, default=110, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_DEINIT_DELAY_MS
#define PICO_STDIO_USB_DEINIT_DELAY_MS 110
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED, Optionally define a
// pin to use as bootloader activity LED when BOOTSEL mode is entered via USB
// (either VIA_BAUD_RATE or VIA_VENDOR_INTERFACE), type=int, min=0, max=47 on
// RP2350B, 29 otherwise, group=pico_stdio_usb

// PICO_CONFIG: PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED_ACTIVE_LOW, Whether
// pin to use as bootloader activity LED when BOOTSEL mode is entered via USB
// (either VIA_BAUD_RATE or VIA_VENDOR_INTERFACE) is active low, type=bool,
// default=0, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED_ACTIVE_LOW
#define PICO_STDIO_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED_ACTIVE_LOW 0
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED, Whether the pin
// specified by PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED is fixed or can be
// modified by picotool over the VENDOR USB interface, type=bool, default=0,
// group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED
#define PICO_STDIO_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED 0
#endif

// Any modes disabled here can't be re-enabled by picotool via VENDOR_INTERFACE.
// PICO_CONFIG: PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK, Optionally
// disable either the mass storage interface (bit 0) or the PICOBOOT interface
// (bit 1) when entering BOOTSEL mode via USB (either VIA_BAUD_RATE or
// VIA_VENDOR_INTERFACE), type=int, min=0, max=3, default=0,
// group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK
#define PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u
#endif

// PICO_CONFIG: PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE, Enable/disable
// resetting into BOOTSEL mode via an additional VENDOR USB interface - enables
// picotool based reset, type=bool, default=1 if application is not using
// TinyUSB directly, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE
#define PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE 1
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_BOOTSEL, If
// vendor reset interface is included allow rebooting to BOOTSEL mode,
// type=bool, default=1, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_BOOTSEL
#define PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_BOOTSEL 1
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_FLASH_BOOT, If
// vendor reset interface is included allow rebooting with regular flash boot,
// type=bool, default=1, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_FLASH_BOOT
#define PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_FLASH_BOOT 1
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_MS_OS_20_DESCRIPTOR, If
// vendor reset interface is included add support for Microsoft OS 2.0
// Descriptor, type=bool, default=1, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_MS_OS_20_DESCRIPTOR
#define PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_MS_OS_20_DESCRIPTOR 1
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS, Delay in ms before
// rebooting via regular flash boot, default=100, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS
#define PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS 100
#endif

// PICO_CONFIG: PICO_STDIO_USB_CONNECTION_WITHOUT_DTR, Disable use of DTR for
// connection checking meaning connection is assumed to be valid, type=bool,
// default=0, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_CONNECTION_WITHOUT_DTR
#define PICO_STDIO_USB_CONNECTION_WITHOUT_DTR 0
#endif

// PICO_CONFIG: PICO_STDIO_USB_DEVICE_SELF_POWERED, Set USB device as self
// powered device, type=bool, default=0, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_DEVICE_SELF_POWERED
#define PICO_STDIO_USB_DEVICE_SELF_POWERED 0
#endif

// PICO_CONFIG: PICO_STDIO_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK, Enable USB
// STDIO support for stdio_set_chars_available_callback. Can be disabled to make
// use of USB CDC RX callback elsewhere, type=bool, default=1,
// group=pico_stdio_usb
#ifndef PICO_STDIO_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK
#define PICO_STDIO_USB_SUPPORT_CHARS_AVAILABLE_CALLBACK 1
#endif

#endif
