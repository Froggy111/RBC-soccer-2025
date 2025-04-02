#include "ICM20948.hpp"
#include "comms.hpp"
#include "pinmap.hpp"
// #include "registers.hpp" // Definitions now in .hpp
#include <cstddef>
#include <cstdint>
#include <cmath> // For round

extern "C" {
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <pico/time.h>
}

// --- Constants for Bit Manipulation ---
// GYRO_CONFIG_1 bits
#define GYRO_FCHOICE_BIT 0
#define GYRO_FS_SEL_LSB_BIT                                                    \
  1 // Not used in config function but here for completeness
#define GYRO_DLPFCFG_LSB_BIT 3
// ACCEL_CONFIG bits
#define ACCEL_FCHOICE_BIT 0
#define ACCEL_FS_SEL_LSB_BIT 1
#define ACCEL_DLPFCFG_LSB_BIT 3

// --- Bank Switching Helper ---
void icm20948::set_bank(config_t *config, uint8_t bank) {
  // Only banks 0, 1, 2, 3 are valid
  if (bank > 3)
    return;
  uint8_t reg_val[2] = {REG_BANK_SEL, static_cast<uint8_t>(bank << 4)};
  spi_write(config, reg_val, 2);
}

// --- SPI Communication (Keep your existing functions) ---
void icm20948::spi_configure(config_t *config) {
  // init spi
  gpio_set_function((uint)pinmap::Pico::SPI0_SCLK, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MISO, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MOSI, GPIO_FUNC_SPI);
  spi_set_format(config->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void icm20948::spi_write(config_t *config, const uint8_t *data, size_t len) {
  spi_configure(config);

  uint8_t buf[len];
  for (uint8_t i = 0; i < len; i++)
    buf[i] = data[i];
  buf[0] = buf[0] & 0x7F; // Clear R/W bit for writing

  gpio_put(
      (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
      0);
  spi_write_blocking(config->spi, buf, len);
  gpio_put(
      (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
      1);

  return;
}

void icm20948::spi_read(config_t *config, uint8_t addr, uint8_t *buffer,
                        size_t len_buffer) {
  spi_configure(config);

  uint8_t total_length = len_buffer + 1;

  // prepare buffer to send
  uint8_t buf_send[total_length];
  buf_send[0] = addr | 0x80; // Set R/W bit for reading

  // prepare buffer for receiving
  volatile uint8_t buf_recv[total_length];

  gpio_put(
      (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
      0);
  spi_write_read_blocking(config->spi, buf_send, (uint8_t *)buf_recv,
                          total_length);
  gpio_put(
      (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
      1);

  // copy received data to buffer
  for (uint8_t i = 0; i < len_buffer; i++) {
    buffer[i] = buf_recv[i + 1];
  }
}

// --- NEW: Configuration Functions Implementation ---

int8_t icm20948::set_gyro_config(config_t *config, float odr_hz,
                                 uint8_t dlpf_cfg, bool bypass_dlpf) {
  uint8_t reg_val[2];
  uint8_t current_config;
  uint8_t gyro_smplrt_div = 0; // Default divider

  // Validate DLPF config
  if (dlpf_cfg > 7) {
#ifndef NDEBUG
    printf("Invalid Gyro DLPF Config: %d\n", dlpf_cfg);
#endif
    return -1;
  }

  // --- Calculations (Only if DLPF is NOT bypassed) ---
  if (!bypass_dlpf) {
    // Calculate divider for desired ODR
    // ODR formula applies only when FCHOICE=1 (DLPF enabled) and DLPFCFG is 1-6
    // If DLPFCFG is 0 or 7, ODR is fixed according to Table 16 (9kHz or 361.4Hz)
    if (dlpf_cfg >= 1 && dlpf_cfg <= 6) {
      if (odr_hz <= 0.0f || odr_hz > 1125.0f) {
#ifndef NDEBUG
        printf("Invalid Gyro ODR (%.2f Hz). Must be > 0 and <= 1125 Hz when "
               "DLPF enabled.\n",
               odr_hz);
#endif
        return -1;
      }
      // Calculate divider: SMPLRT_DIV = (1125 / ODR_Hz) - 1
      double div_f = (1125.0 / odr_hz) - 1.0;
      gyro_smplrt_div = static_cast<uint8_t>(round(div_f));
      // Clamp (although check above should prevent overflow)
      if (div_f > 255.0)
        gyro_smplrt_div = 255;
      if (div_f < 0.0)
        gyro_smplrt_div = 0; // Should not happen with ODR check

#ifndef NDEBUG
      float actual_odr = 1125.0f / (1.0f + gyro_smplrt_div);
      printf("Gyro Config: Target ODR: %.2f Hz, Divider: %d, Actual ODR: %.2f "
             "Hz\n",
             odr_hz, gyro_smplrt_div, actual_odr);
#endif
    } else {
#ifndef NDEBUG
      printf("Gyro Config: DLPF_CFG %d selected. ODR fixed by table, target "
             "ODR ignored. Divider register ignored.\n",
             dlpf_cfg);
#endif
      // Divider register is ignored for DLPFCFG 0 or 7
    }
  } else {
#ifndef NDEBUG
    printf("Gyro Config: DLPF Bypassed. ODR fixed by internal rate (datasheet "
           "unclear, ~1.1kHz or 9kHz?). Divider register ignored.\n");
#endif
    // Divider register is ignored when FCHOICE=0
  }

  // --- Register Writes ---
  set_bank(config, 2);
  sleep_us(30); // Small delay after bank switch

  // Read current GYRO_CONFIG_1 to preserve FSR bits (although FSR is in this reg too!)
  spi_read(config, GYRO_CONFIG_1, &current_config, 1);
  sleep_us(30);

  // Modify GYRO_CONFIG_1
  // Clear FCHOICE (bit 0) and DLPFCFG (bits 5:3)
  current_config &=
      ~((0b111 << GYRO_DLPFCFG_LSB_BIT) | (1 << GYRO_FCHOICE_BIT));

  if (bypass_dlpf) {
    // Set FCHOICE = 0 (bypass)
    // DLPFCFG bits remain 0 (doesn't matter when bypassed)
  } else {
    // Set FCHOICE = 1 (enable)
    current_config |= (1 << GYRO_FCHOICE_BIT);
    // Set DLPFCFG bits
    current_config |= (dlpf_cfg << GYRO_DLPFCFG_LSB_BIT);
  }

  // Write modified GYRO_CONFIG_1
  reg_val[0] = GYRO_CONFIG_1;
  reg_val[1] = current_config;
  spi_write(config, reg_val, 2);
  sleep_us(30);

  // Write GYRO_SMPLRT_DIV (only if DLPF enabled and relevant cfg)
  if (!bypass_dlpf && dlpf_cfg >= 1 && dlpf_cfg <= 6) {
    reg_val[0] = GYRO_SMPLRT_DIV;
    reg_val[1] = gyro_smplrt_div;
    spi_write(config, reg_val, 2);
    sleep_us(30);
  }

  set_bank(config, 0); // Switch back to default bank
  sleep_us(30);

  return 0;
}

int8_t icm20948::set_accel_config(config_t *config, float odr_hz,
                                  uint8_t dlpf_cfg, uint8_t fsr_g,
                                  bool bypass_dlpf) {
  uint8_t reg_val[2];
  uint8_t current_config;
  uint16_t accel_smplrt_div = 0; // 12-bit divider
  uint8_t fsr_bits;

  // Validate DLPF config
  if (dlpf_cfg > 7) {
#ifndef NDEBUG
    printf("Invalid Accel DLPF Config: %d\n", dlpf_cfg);
#endif
    return -1;
  }

  // Validate and determine FSR bits
  switch (fsr_g) {
  case 2:
    fsr_bits = 0b00;
    break;
  case 4:
    fsr_bits = 0b01;
    break;
  case 8:
    fsr_bits = 0b10;
    break;
  case 16:
    fsr_bits = 0b11;
    break;
  default:
#ifndef NDEBUG
    printf("Invalid Accel FSR: %d G. Use 2, 4, 8, or 16.\n", fsr_g);
#endif
    return -1;
  }

  // --- Calculations (Only if DLPF is NOT bypassed) ---
  if (!bypass_dlpf) {
    // Calculate divider for desired ODR
    // ODR formula applies only when FCHOICE=1 (DLPF enabled) and DLPFCFG is 1-6
    // If DLPFCFG is 0 or 7, ODR is fixed according to Table 18 (4.5kHz or 473Hz)
    if (dlpf_cfg >= 1 && dlpf_cfg <= 6) {
      if (odr_hz <= 0.0f || odr_hz > 1125.0f) {
#ifndef NDEBUG
        printf("Invalid Accel ODR (%.2f Hz). Must be > 0 and <= 1125 Hz when "
               "DLPF enabled.\n",
               odr_hz);
#endif
        return -1;
      }
      // Calculate divider: SMPLRT_DIV = (1125 / ODR_Hz) - 1
      double div_f = (1125.0 / odr_hz) - 1.0;
      accel_smplrt_div = static_cast<uint16_t>(round(div_f));
      // Clamp to 12 bits
      if (div_f > 4095.0)
        accel_smplrt_div = 4095;
      if (div_f < 0.0)
        accel_smplrt_div = 0; // Should not happen

#ifndef NDEBUG
      float actual_odr = 1125.0f / (1.0f + accel_smplrt_div);
      printf("Accel Config: Target ODR: %.2f Hz, Divider: %d, Actual ODR: %.2f "
             "Hz\n",
             odr_hz, accel_smplrt_div, actual_odr);
#endif
    } else {
#ifndef NDEBUG
      printf("Accel Config: DLPF_CFG %d selected. ODR fixed by table, target "
             "ODR ignored. Divider registers ignored.\n",
             dlpf_cfg);
#endif
      // Divider registers ignored for DLPFCFG 0 or 7
    }
  } else {
#ifndef NDEBUG
    printf("Accel Config: DLPF Bypassed. ODR fixed by internal rate (datasheet "
           "unclear, ~1.1kHz or 4.5kHz?). Divider registers ignored.\n");
#endif
    // Divider registers ignored when FCHOICE=0
  }

  // --- Register Writes ---
  set_bank(config, 2);
  sleep_us(30);

  // Read current ACCEL_CONFIG to preserve reserved bits etc.
  spi_read(config, ACCEL_CONFIG, &current_config, 1);
  sleep_us(30);

  // Modify ACCEL_CONFIG
  // Clear FCHOICE (bit 0), FS_SEL (bits 2:1), and DLPFCFG (bits 5:3)
  current_config &=
      ~((1 << ACCEL_FCHOICE_BIT) | (0b11 << ACCEL_FS_SEL_LSB_BIT) |
        (0b111 << ACCEL_DLPFCFG_LSB_BIT));

  // Set FSR bits
  current_config |= (fsr_bits << ACCEL_FS_SEL_LSB_BIT);

  if (bypass_dlpf) {
    // Set FCHOICE = 0 (bypass)
    // DLPFCFG bits remain 0 (doesn't matter when bypassed)
  } else {
    // Set FCHOICE = 1 (enable)
    current_config |= (1 << ACCEL_FCHOICE_BIT);
    // Set DLPFCFG bits
    current_config |= (dlpf_cfg << ACCEL_DLPFCFG_LSB_BIT);
  }

  // Write modified ACCEL_CONFIG
  reg_val[0] = ACCEL_CONFIG;
  reg_val[1] = current_config;
  spi_write(config, reg_val, 2);
  sleep_us(30);

  // Write ACCEL_SMPLRT_DIV (only if DLPF enabled and relevant cfg)
  if (!bypass_dlpf && dlpf_cfg >= 1 && dlpf_cfg <= 6) {
    // Write upper bits to ACCEL_SMPLRT_DIV_1
    reg_val[0] = ACCEL_SMPLRT_DIV_1;
    reg_val[1] = (accel_smplrt_div >> 8) & 0x0F; // Only 4 bits [11:8]
    spi_write(config, reg_val, 2);
    sleep_us(30);

    // Write lower bits to ACCEL_SMPLRT_DIV_2
    reg_val[0] = ACCEL_SMPLRT_DIV_2;
    reg_val[1] = accel_smplrt_div & 0xFF; // Lower 8 bits [7:0]
    spi_write(config, reg_val, 2);
    sleep_us(30);
  }

  set_bank(config, 0); // Switch back to default bank
  sleep_us(30);

  return 0;
}

// --- Initialization (Modify to use new functions if desired, or keep explicit writes) ---
int8_t icm20948::init(config_t *config) {
  uint8_t reg[2], buf[1];

  // init gpio pins (Keep existing)
  if (config->id == 1) {
    gpio_init((uint)pinmap::Pico::IMU1_NCS);
    gpio_set_dir((uint)pinmap::Pico::IMU1_NCS, GPIO_OUT);
    gpio_put((uint)pinmap::Pico::IMU1_NCS, 1);
  } else {
    gpio_init((uint)pinmap::Pico::IMU2_NCS);
    gpio_set_dir((uint)pinmap::Pico::IMU2_NCS, GPIO_OUT);
    gpio_put((uint)pinmap::Pico::IMU2_NCS, 1);
  }
  spi_configure(config);

  // --- Device Reset and Clock Setup ---
  set_bank(config, 0); // Start in Bank 0
  sleep_us(30);

  // Optional: Reset device (might require re-init after)
  // reg[0] = PWR_MGMT_1;
  // reg[1] = 0x80; // Set H_RESET bit
  // spi_write(config, reg, 2);
  // sleep_ms(100); // Wait for reset

  // Wake up device (clear sleep bit)
  reg[0] = PWR_MGMT_1;
  reg[1] = 0x00; // Datasheet reset value is 0x41, but 0x00 wakes it
  spi_write(config, reg, 2);
  sleep_ms(10); // Allow time to wake

  // Auto select clock source (recommended)
  reg[0] = PWR_MGMT_1;
  reg[1] = 0x01; // CLKSEL = 1
  spi_write(config, reg, 2);
  sleep_ms(10); // Allow clock to stabilize

  // Enable Accel & Gyro (ensure they are not disabled)
  reg[0] = PWR_MGMT_2;
  reg[1] = 0x00; // Enable all axes
  spi_write(config, reg, 2);
  sleep_ms(10);

  // --- Check Accel/Gyro ID ---
  spi_read(config, WHO_AM_I_ICM20948, buf, 1);
#ifndef NDEBUG
  printf("AG. WHO_AM_I: 0x%X\n", buf[0]);
#endif
  if (buf[0] != 0xEA) {
#ifndef NDEBUG
    printf("Accel/Gyro WHO_AM_I check FAILED!\n");
#endif
    return -1;
  }

  // --- Configure Accel & Gyro using new functions ---
  // Example: ~100Hz ODR, ~50Hz BW DLPF enabled, Accel +/-4G
  int8_t gyro_ok =
      set_gyro_config(config, 100.0f, 3, false); // DLPFCFG=3 -> ~51Hz BW
  int8_t accel_ok = set_accel_config(config, 100.0f, 3, 4,
                                     false); // DLPFCFG=3 -> ~50Hz BW, FSR=4G

  if (gyro_ok != 0 || accel_ok != 0) {
#ifndef NDEBUG
    printf("Failed to configure Accel/Gyro ODR/DLPF\n");
#endif
    // return -1; // Optionally return error
  }

  // --- Magnetometer Setup (Assuming Bypass Mode) ---
  set_bank(config, 0); // Ensure Bank 0
  sleep_us(30);

  // Enable I2C Master Bypass Mode
  reg[0] = INT_PIN_CFG;
  reg[1] = 0x02; // Set BYPASS_EN
  spi_write(config, reg, 2);
  sleep_ms(10); // Allow bypass to enable

  // Now you would typically use a separate I2C library targeting the magnetometer's
  // address (0x0C usually) via the main MCU's I2C peripheral connected to SCL/SDA (pins 23/24).
  // The spi_read/write for mag registers in your original code won't work in bypass mode.
  // We'll keep the placeholder mag setup logic for now, assuming it might be adapted
  // for I2C Master mode later, or removed if only bypass is used.

  // Placeholder: Check Mag ID via Bypass (Requires external I2C read)
  // uint8_t mag_id;
  // your_i2c_read(MAG_ADDRESS, AK09916_WIA2_REG, &mag_id, 1);
  // if (mag_id != 0x09) return -1;
#ifndef NDEBUG
  printf("MAG Setup: Assuming bypass mode. Use external I2C for direct mag "
         "access.\n");
#endif

  // Placeholder: Configure Mag via Bypass (Requires external I2C write)
  // your_i2c_write(MAG_ADDRESS, AK09916_CNTL2, 0x08); // e.g., Continuous 100Hz

  // Disable Bypass if you intend to use ICM's I2C Master later
  // reg[0] = INT_PIN_CFG;
  // reg[1] = 0x00; // Clear BYPASS_EN
  // spi_write(config, reg, 2);
  // sleep_ms(10);

  return 0; // Or return status codes from config functions
}

// --- Keep existing read/cal functions ---
// ... (spi_configure, spi_write, spi_read) ...
// ... (set_mag_rate - needs update if using bypass or I2C master) ...
// ... (read_raw_*, cal_*, read_cal_*, read_temp_c) ...

// --- Update set_mag_rate (Placeholder for Bypass/I2C Master) ---
void icm20948::set_mag_rate(config_t *config, uint8_t mode) {
  // This function needs to be re-implemented based on how you access the magnetometer:
  // 1. Bypass Mode: Use the main MCU's I2C peripheral to write to AK09916_CNTL2
  // 2. ICM I2C Master Mode: Configure I2C_SLVx registers in Bank 3 to write to AK09916_CNTL2

#ifndef NDEBUG
  printf("WARNING: set_mag_rate needs implementation for Bypass or I2C Master "
         "mode.\n");
#endif
  // Placeholder logic from original code (won't work in bypass)
  /*
    uint8_t reg[2];
    uint8_t reg_val_byte;
    switch (mode) {
        case 0: reg_val_byte = 0x01; break; // single shot
        case 10: reg_val_byte = 0x02; break; // 10Hz continuous
        case 20: reg_val_byte = 0x04; break; // 20Hz continuous
        case 50: reg_val_byte = 0x06; break; // 50Hz continuous
        case 100: reg_val_byte = 0x08; break; // 100Hz continuous
        default: printf("error at set_mag_mode: wrong mode %d\n", mode); return;
    }
    reg[0] = AK09916_CNTL2;
    reg[1] = reg_val_byte;
    // This write assumes direct access, which isn't correct for mag via SPI
    // spi_write(config, reg, 2);
    */
  return;
}

// --- Keep existing read/cal functions ---
// ... (read_raw_accel, read_raw_gyro, read_raw_temp) ...
void icm20948::read_raw_mag(config_t *config, int16_t mag[3]) {
  // Needs re-implementation for Bypass or I2C Master mode
#ifndef NDEBUG
  printf("WARNING: read_raw_mag needs implementation for Bypass or I2C Master "
         "mode.\n");
#endif
  // Placeholder logic (won't work in bypass)
  /*
    uint8_t buf[8];
    // This read assumes direct access
    // spi_read(config, AK09916_XOUT_L, buf, 8);
    // ... rest of parsing ...
    */
  mag[0] = mag[1] = mag[2] = 0; // Return zeros for now
  return;
}

// ... (cal_gyro, read_cal_gyro, cal_accel, read_cal_accel) ...
void icm20948::cal_mag_simple(config_t *config, int16_t mag_bias[3]) {
  // Needs re-implementation for Bypass or I2C Master mode
#ifndef NDEBUG
  printf("WARNING: cal_mag_simple needs implementation for Bypass or I2C "
         "Master mode.\n");
#endif
  mag_bias[0] = mag_bias[1] = mag_bias[2] = 0; // Set bias to zero for now
  return;
}
void icm20948::read_cal_mag(config_t *config, int16_t mag[3], int16_t bias[3]) {
  // Needs re-implementation for Bypass or I2C Master mode
#ifndef NDEBUG
  printf("WARNING: read_cal_mag needs implementation for Bypass or I2C Master "
         "mode.\n");
#endif
  mag[0] = mag[1] = mag[2] = 0; // Return zeros for now
  return;
}
// ... (read_temp_c) ...
void icm20948::read_temp_c(config_t *config, float *temp) {
  int16_t tmp;
  read_raw_temp(config, &tmp);
  // temp  = ((raw_value - ambient_temp) / speed_of_sound) + 21
  *temp = (((float)tmp - 21.0f) / 333.87) + 21.0f;
  return;
}

void icm20948::read_raw_accel(config_t *config, int16_t accel[3]) {
  uint8_t buf[6];

  // accel: 2 bytes each axis
  spi_read(config, ACCEL_XOUT_H, buf, 6);

  for (uint8_t i = 0; i < 3; i++)
    accel[i] = (buf[i * 2] << 8 | buf[(i * 2) + 1]);

  return;
}

void icm20948::read_raw_gyro(config_t *config, int16_t gyro[3]) {
  uint8_t buf[6];

  // gyro: 2byte each axis
  spi_read(config, GYRO_XOUT_H, buf, 6);

  for (uint8_t i = 0; i < 3; i++)
    gyro[i] = (buf[i * 2] << 8 | buf[(i * 2) + 1]);

  return;
}
