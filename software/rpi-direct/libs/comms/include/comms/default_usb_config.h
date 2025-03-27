#ifndef DEFAULT_USB_CONFIG_H
#define DEFAULT_USB_CONFIG_H

#ifndef USB_MAX_PACKET_SIZE
#define USB_MAX_PACKET_SIZE 256
#endif

#ifndef USB_TX_BUFSIZE
#define USB_TX_BUFSIZE USB_MAX_PACKET_SIZE
#endif
#ifndef USB_RX_BUFSIZE
#define USB_RX_BUFSIZE USB_MAX_PACKET_SIZE
#endif

#ifndef USB_DEBUG_ABSTRACTED
#define USB_DEBUG_ABSTRACTED // default to the abstracted usb debug instead of sending raw text. undefine this to change.
#endif

#endif // DEFAULT_USB_CONFIG_H