#include "comms/default_usb_config.h"
#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

// RHPort number used for device
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE
// TinyUSB OS as FreeRTOS so tud_task() blocks
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_FREERTOS
#endif

// Device mode configuration
#define CFG_TUD_CDC 1
#define CFG_TUD_CDC_RX_BUFSIZE USB_RX_BUFSIZE
#define CFG_TUD_CDC_TX_BUFSIZE USB_TX_BUFSIZE

// Enable vendor interface for reset functionality
#define CFG_TUD_VENDOR 1
#define CFG_TUD_VENDOR_RX_BUFSIZE USB_RX_BUFSIZE
#define CFG_TUD_VENDOR_TX_BUFSIZE USB_TX_BUFSIZE

#endif /* _TUSB_CONFIG_H_ */
