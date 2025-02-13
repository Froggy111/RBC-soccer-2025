#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

// RHPort number used for device
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE

// Device mode configuration
#define CFG_TUD_CDC                 1
#define CFG_TUD_CDC_RX_BUFSIZE     256
#define CFG_TUD_CDC_TX_BUFSIZE     256

// Enable vendor interface for reset functionality
#define CFG_TUD_VENDOR             1
#define CFG_TUD_VENDOR_RX_BUFSIZE  256
#define CFG_TUD_VENDOR_TX_BUFSIZE  256

#endif /* _TUSB_CONFIG_H_ */
