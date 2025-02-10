#define led_pin 16
#define led_count 1
#define brightness 50

// SPI Protocol
#define SPI_ADDRESS_MASK 0x3F00 // Mask for SPI register address bits
#define SPI_ADDRESS_POS 8       // Position for SPI register address bits
#define SPI_DATA_MASK 0x00FF    // Mask for SPI register data bits
#define SPI_DATA_POS 0          // Position for SPI register data bits
#define SPI_RW_BIT_MASK 0x4000  // Mask for SPI register read write indication bit

const uint16_t A_IPROPI = 4750; // VQFN-HR package
const uint32_t IPROPI_resistance = 680;
const uint32_t ipropi_moving_average_size = 16384; // change this if uw, if its too big it wont fit

