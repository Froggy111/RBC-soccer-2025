#include "pico/unique_id.h"
#include "tusb.h"
#include <pico/usb_reset_interface.h>

// Private constants for USB descriptors
#define USB_VID 0x2E8A // Raspberry Pi
#define USB_PID 0x000a // Pico SDK CDC

// Interface numbers
#define USB_CDC_INTERFACE 0    // CDC interfaces start at 0
#define USB_VENDOR_INTERFACE 2 // Vendor interface comes after CDC

// Endpoint numbers
#define EP_CDC_NOTIF 0x81
#define EP_CDC_OUT 0x02
#define EP_CDC_IN 0x82
#define EP_VENDOR_OUT 0x03 // Added vendor endpoints
#define EP_VENDOR_IN 0x83

// String index constants
#define USB_STR_ZERO 0
#define USB_STR_MANUF 1
#define USB_STR_PRODUCT 2
#define USB_STR_SERIAL 3
#define USB_STR_CDC 4
#define USB_STR_VENDOR 5

// Device Descriptor
static const tusb_desc_device_t device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = USB_STR_MANUF,
    .iProduct = USB_STR_PRODUCT,
    .iSerialNumber = USB_STR_SERIAL,
    .bNumConfigurations = 1};

// Configuration Descriptor
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + 9)

static const uint8_t configuration_descriptor[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, 3, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // CDC with Interface Association Descriptor (IAD)
    TUD_CDC_DESCRIPTOR(USB_CDC_INTERFACE, USB_STR_CDC, EP_CDC_NOTIF,
                       8, // Notification endpoint & size
                       EP_CDC_OUT, EP_CDC_IN, 64), // Data endpoints & size

    // Simple vendor interface descriptor
    9,                          // bLength
    TUSB_DESC_INTERFACE,        // bDescriptorType
    USB_VENDOR_INTERFACE,       // bInterfaceNumber
    0,                          // bAlternateSetting
    0,                          // bNumEndpoints
    TUSB_CLASS_VENDOR_SPECIFIC, // bInterfaceClass
    RESET_INTERFACE_SUBCLASS,   // bInterfaceSubClass
    RESET_INTERFACE_PROTOCOL,   // bInterfaceProtocol
    USB_STR_VENDOR              // iInterface
};

// String Descriptors
static char serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

static const char *string_descriptors[] = {[USB_STR_MANUF] = "Raspberry Pi",
                                           [USB_STR_PRODUCT] = "Pico",
                                           [USB_STR_SERIAL] = serial_str,
                                           [USB_STR_CDC] = "Board CDC",
                                           [USB_STR_VENDOR] =
                                               "Reset Interface"};

// Callbacks remain the same
uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *)&device_descriptor;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;
  return configuration_descriptor;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  static uint16_t desc_str[32];
  uint8_t len;

  if (!serial_str[0]) {
    pico_get_unique_board_id_string(serial_str, sizeof(serial_str));
  }

  if (index == 0) {
    desc_str[1] = 0x0409; // English
    len = 1;
  } else {
    if (index >= sizeof(string_descriptors) / sizeof(string_descriptors[0])) {
      return NULL;
    }
    const char *str = string_descriptors[index];
    for (len = 0; len < 31 && str[len]; ++len) {
      desc_str[1 + len] = str[len];
    }
  }

  desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);
  return desc_str;
}
