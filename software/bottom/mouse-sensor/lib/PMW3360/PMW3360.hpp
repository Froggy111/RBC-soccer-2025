#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "types.hpp"

namespace PMW3360 {

// Registers
const types::byte REG_Product_ID = 0x00;
const types::byte REG_Revision_ID = 0x01;
const types::byte REG_Motion = 0x02;
const types::byte REG_Delta_X_L = 0x03;
const types::byte REG_Delta_X_H = 0x04;
const types::byte REG_Delta_Y_L = 0x05;
const types::byte REG_Delta_Y_H = 0x06;
const types::byte REG_SQUAL = 0x07;
const types::byte REG_Raw_Data_Sum = 0x08;
const types::byte REG_Maximum_Raw_data = 0x09;
const types::byte REG_Minimum_Raw_data = 0x0A;
const types::byte REG_Shutter_Lower = 0x0B;
const types::byte REG_Shutter_Upper = 0x0C;
const types::byte REG_Control = 0x0D;
const types::byte REG_Config1 = 0x0F;
const types::byte REG_Config2 = 0x10;
const types::byte REG_Angle_Tune = 0x11;
const types::byte REG_Frame_Capture = 0x12;
const types::byte REG_SROM_Enable = 0x13;
const types::byte REG_Run_Downshift = 0x14;
const types::byte REG_Rest1_Rate_Lower = 0x15;
const types::byte REG_Rest1_Rate_Upper = 0x16;
const types::byte REG_Rest1_Downshift = 0x17;
const types::byte REG_Rest2_Rate_Lower = 0x18;
const types::byte REG_Rest2_Rate_Upper = 0x19;
const types::byte REG_Rest2_Downshift = 0x1A;
const types::byte REG_Rest3_Rate_Lower = 0x1B;
const types::byte REG_Rest3_Rate_Upper = 0x1C;
const types::byte REG_Observation = 0x24;
const types::byte REG_Data_Out_Lower = 0x25;
const types::byte REG_Data_Out_Upper = 0x26;
const types::byte REG_Raw_Data_Dump = 0x29;
const types::byte REG_SROM_ID = 0x2A;
const types::byte REG_Min_SQ_Run = 0x2B;
const types::byte REG_Raw_Data_Threshold = 0x2C;
const types::byte REG_Config5 = 0x2F;
const types::byte REG_Power_Up_Reset = 0x3A;
const types::byte REG_Shutdown = 0x3B;
const types::byte REG_Inverse_Product_ID = 0x3F;
const types::byte REG_LiftCutoff_Tune3 = 0x41;
const types::byte REG_Angle_Snap = 0x42;
const types::byte REG_LiftCutoff_Tune1 = 0x4A;
const types::byte REG_Motion_Burst = 0x50;
const types::byte REG_LiftCutoff_Tune_Timeout = 0x58;
const types::byte REG_LiftCutoff_Tune_Min_Length = 0x5A;
const types::byte REG_SROM_Load_Burst = 0x62;
const types::byte REG_Lift_Config = 0x63;
const types::byte REG_Raw_Data_Burst = 0x64;
const types::byte REG_LiftCutoff_Tune2 = 0x65;

/**
* @brief Struct for data read from PMW3360::read_burst(void).
*/
struct Data {
  bool is_motion; /** True if a motion is detected. */
  bool is_on_surface; /** True when the chip is on the surface (below lift_cutoff). */
  types::i16 dx; /** Displacement in x direction. Unit: Count. */
  types::i16 dy; /** Displacement in y direction. Unit: Count. */
  types::u8 surface_quality; /** Number of features on surface = surface_quality * 8. range: 0-128 (inclusive). */
  types::u8 raw_data_sum; /** Sum of all 1296 raw pixel data in the current frame. */
  types::u8 max_raw_data; /** Max raw data value in current frame, range: 0-127 (inclusive). */
  types::u8 min_raw_data; /** Min raw data value in current frame, range: 0-127 (inclusive). */
  types::u32 shutter; /** Number of clock cycles of the internal oscillator. Shutter is adjusted automatically to keep the average raw data values within normal operating ranges. */
}; 

enum class OperationMode : types::u8 {
  tracking,
  liftoff_calibration,
  undefined
};

enum class LiftoffHeight : types::u8 {
  _2mm,
  _3mm
};

enum class LiftoffCalibrationStatus : types::u8 {
  in_progress,
  min_length_met,
  min_length_and_timeout_met,
  failed_timeout,
  undefined    
};

struct LiftoffCalibrationResult {
  types::u8 raw_data_threshold;
  types::u8 minimum_squal;
};

class PMW3360 {
public:
  PMW3360(void);
  /**
    * @brief Resets and initilalises the PMW3360 sensor.
    * @param SPI: SPI interface to use. The SPI interface should be initialised before initialising this.
    * @param SCS: SPI select pin number.
    * @param CPI: Counts per inch. Default: 5000. Maximum: 12000. Multiples of 100 are allowed.
    * @param operation_mode: Mode of operation of the PMW3360 sensor. Defaults to tracking mode, can be set to liftoff_calibration mode.
    * @returns 1 if initialisation was successful, 0 if unsuccessful.
    */
  bool begin(types::GenericSPI* SPI, const types::u8 SCS, const types::u16 CPI = 5000, const OperationMode operation_mode = OperationMode::tracking);
  /**
    * @brief Sets the CPI of the PMW3360 sensor.
    * @param CPI: Counts per inch. Maximum: 12000. Multiples of 100 are allowed.
    * @returns 1 if CPI is a valid value, 0 if CPI is invalid.
    */
  bool set_CPI(const types::u16 CPI);
  /**
    * @brief Reads the CPI of the PMW3360 sensor from the sensor's register.
    * @returns CPI of the PMW3360 sensor.
    */
  types::u16 get_CPI(void);
  /**
    * @brief Performs a burst read operation on the PMW3360 sensor.
    * @returns Refer to the PMW3360::data struct.
    */
  Data read_burst(void);
  /**
    * @brief Enters frame capture mode.
    */
  void prepare_image(void);
  /**
    * @brief Reads the next pixel in the frame.
    * @returns Data of the pixel.
    */
  types::byte read_image_pixel(void);
  /**
    * @brief Exits frame capture mode. This must be called after capturing a frame! To resume normal operation, call begin again.
    */
  void end_image(void);
  /**
    * @brief Begins the liftoff calibration routine. The liftoff calibration SROM should be loaded first, by beginning with operation_mode set to liftoff_calibration.
    * Will hold the SPI interface until liftoff calibration is stopped.
    * @param min_length_val: Value of LiftCutoff_Tune_Min_Length register. Minimum length (in freedom units) = (min_length_val + 1) * 2. Allowed range: 0x00 to 0xF9 (inclusive).
    * Default: 0x09 (20 freedom units).
    * @param timeout_val: Value of LiftCutoff_Tune_Timeout register. Timeout (in seconds) = (timeout_val + 1) * 0.5. Allowed range: 0x00 to 0xF9 (inclusive). Default: 0x27 (20s)
    * @returns 0 if sensor is not in liftoff_calibration operation mode, 1 if successfully started liftoff calibration.
    */
  bool begin_liftoff_calibration(const types::byte min_length_val = 0x09, const types::byte timeout_val = 0x27);
  /**
    * @brief Reads the status of the ongoing liftoff calibration. This should only be called while a liftoff calibration is being ran.
    * @returns The status of the liftoff calibration. It can be in_progress, min_length_met (calibration can be stopped here),
    * min_length_and_timeout_met (calibration should be stopped here), failed_timeout (calibration should be restarted here), or invalid (something really went wrong here).
    */
  LiftoffCalibrationStatus liftoff_calibration_status(void);
  /**
    * @brief Cancels the ongoing liftoff calibration. Call this if liftoff calibration has failed with failed_timeout, then begin liftoff calibration again.
    */
  void cancel_liftoff_calibration(void);
  /**
    * @brief Ends the liftoff calibration.
    * @returns Recommended value for raw data threshold and minimum surface quality.
    */
  LiftoffCalibrationResult end_liftoff_calibration(void);
  /**
    * @brief Sets the minimum surface quality register. PMW3360 sensor must be in tracking mode.
    * @param minimum_squal: Minimum surface quality. Default: 0x10
    * @returns 0 if PMW3360 sensor is not in tracking mode, 1 if successful.
    */
  bool set_minimum_squal(const types::u8 minimum_squal = 0x10);
  /**
    * @brief Reads the minimum surface quality register. PMW3360 sensor must be in tracking mode.
    * @returns The current minimum surface quality value.
    */
  types::Result<types::u8, types::Status> get_minimum_squal(void);
  /**
    * @brief Sets the raw data threshold register. PMW3360 sensor must be in tracking mode.
    * @param raw_data_threshold: Raw data threshold. Default: 0x0A
    * @returns 0 if PMW3360 sensor is not in tracking mode, 1 if successful.
    */
  bool set_raw_data_threshold(const types::u8 raw_data_threshold = 0x0A);
  /**
    * @brief Sets the raw data threshold register. PMW3360 sensor must be in tracking mode.
    * @returns The current raw data threshold value.
    */
  types::Result<types::u8, types::Status> get_raw_data_threshold(void);
  /**
    * @brief Sets the lift detection height threshold.
    * @param liftoff_height: Height above which sensor becomes inactive. Default: 2mm.
    */
  void set_liftoff_height(const LiftoffHeight liftoff_height = LiftoffHeight::_2mm);
  /**
    * @brief Sets the lift detection height threshold.
    * @returns The current liftoff height value.
    */
  types::Result<LiftoffHeight, types::Status> get_liftoff_height(void);

private:
  types::GenericSPI* SPI;
  types::u8 SCS;
  OperationMode operation_mode = OperationMode::undefined;
  bool in_burst_mode = false;
  unsigned long last_burst = 0;
  void begin_spi_transaction(void);
  void end_spi_transaction(void);
  void begin_spi_communication(void);
  void end_spi_communication(void);
  types::byte read_register(const types::byte register_addr);
  void write_register(const types::byte register_addr, const types::byte data);
  void upload_firmware(const types::byte* firmware_blob, const types::u16 firmware_length);
  bool check_signature(const types::byte SROM_version);
};

}
