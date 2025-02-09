#include <Arduino.h>
#include <SPI.h>
#include "PMW3360.hpp"
#include "PMW3360_liftoff_srom.hpp"
#include "PMW3360_tracking_srom.hpp"
#include "types.hpp"

using namespace types;

namespace PMW3360 {

const SPISettings spi_settings(8000000, MSBFIRST, SPI_MODE3);

PMW3360::PMW3360(void) = default;

bool PMW3360::begin(GenericSPI* SPI, const types::u8 SCS, const types::u16 CPI, const OperationMode operation_mode) {
  this->SPI = SPI;
  this->SCS = SCS;
  pinMode(this->SCS, OUTPUT);
  digitalWrite(this->SCS, HIGH);

  // hard reset
  begin_spi_transaction();
  end_spi_communication(); begin_spi_communication(); end_spi_communication(); // ensure that serial port is reset
  
  write_register(REG_Shutdown, 0xb6); // Shutdown first
  end_spi_transaction();
  delay(300);
  
  begin_spi_transaction();
  begin_spi_communication();
  delayMicroseconds(40);
  end_spi_communication();
  delayMicroseconds(40);

  write_register(REG_Power_Up_Reset, 0x5a); // force reset
  end_spi_transaction();;
  delay(50); // wait for it to reboot
  
  begin_spi_transaction();
  // read registers 0x02 to 0x06 (and discard the data)
  read_register(REG_Motion);
  read_register(REG_Delta_X_L);
  read_register(REG_Delta_X_H);
  read_register(REG_Delta_Y_L);
  read_register(REG_Delta_Y_H);
  
  // upload the firmware
  if (operation_mode == OperationMode::tracking) {
    upload_firmware(tracking_srom_0x04, tracking_srom_0x04_len);
  }
  else if (operation_mode == OperationMode::liftoff_calibration) {
    upload_firmware(liftoff_srom_0x81, liftoff_srom_0x81_len);
  }
  else {
    this->operation_mode = OperationMode::undefined;
    return false;
  }

  end_spi_transaction();
  
  delay(10);
  set_CPI(CPI);

  if (operation_mode == OperationMode::tracking) {
    if (check_signature(0x04)) {
      this->operation_mode = OperationMode::tracking;
      return true;
    }
  }
  else if (operation_mode == OperationMode::liftoff_calibration) {
    if (check_signature(0x81)) {
      this->operation_mode = OperationMode::liftoff_calibration;
      return true;
    }
  }
  else {
    this->operation_mode = OperationMode::undefined;
    return false;
  }

  return false;
}

bool PMW3360::set_CPI(const u16 cpi) {
  if ((cpi % 100 != 0) || (cpi > 12000) || (cpi < 100)) { // CPI is an invalid value.
    return false;
  }
  u8 cpi_val = (cpi / 100) - 1;
  begin_spi_transaction();
  write_register(REG_Config1, cpi_val);
  end_spi_transaction();
  return true;
}

u16 PMW3360::get_CPI(void) {
  begin_spi_transaction();
  u16 cpi_val = read_register(REG_Config1);
  end_spi_transaction();
  u16 cpi = (cpi_val + 1) * 100;
  return cpi;
}

Data PMW3360::read_burst(void) {
  u64 time_from_last_burst = micros() - this->last_burst;
  byte burst_buffer[12];
  
  begin_spi_transaction();
  
  if(!this->in_burst_mode || time_from_last_burst > 500*1000)
  {
    write_register(REG_Motion_Burst, 0x00);
    this->in_burst_mode = true;    
  }

  begin_spi_communication();
  this->SPI->transfer(REG_Motion_Burst);    
  delayMicroseconds(35); // waits for tSRAD  

  this->SPI->transfer(burst_buffer, 12); // read burst buffer
  delayMicroseconds(1); // tSCLK-NCS for read operation is 120ns

  end_spi_communication();
  end_spi_transaction();

  if(burst_buffer[0] & 0b111) // panic recovery, sometimes burst mode works weird.
  {
    this->in_burst_mode = false;
  }

  this->last_burst = micros();

  Data data;

  bool motion = (burst_buffer[0] & 0x80) != 0;
  bool surface = (burst_buffer[0] & 0x08) == 0;   // 0 if on surface / 1 if off surface

  uint8_t xl = burst_buffer[2];    // dx LSB
  uint8_t xh = burst_buffer[3];    // dx MSB
  uint8_t yl = burst_buffer[4];    // dy LSB
  uint8_t yh = burst_buffer[5];    // dy MSB
  uint8_t sl = burst_buffer[10];   // shutter LSB
  uint8_t sh = burst_buffer[11];   // shutter MSB
  
  int x = (int16_t)((xh << 8) | xl);
  int y = (int16_t)((yh << 8) | yl);
  unsigned int shutter = sh<<8 | sl;

  data.is_motion = motion;
  data.is_on_surface = surface;
  data.dx = x;
  data.dy = y;
  data.surface_quality = burst_buffer[6];
  data.raw_data_sum = burst_buffer[7];
  data.max_raw_data = burst_buffer[8];
  data.min_raw_data = burst_buffer[9];
  data.shutter = shutter;

  return data;
}

void PMW3360::prepare_image()
{
  delay(250); // wait 250ms before frame capture
  begin_spi_transaction();

  write_register(REG_Config2, 0x00);

  write_register(REG_Frame_Capture, 0x83);
  write_register(REG_Frame_Capture, 0xc5);  

  delay(20);

  begin_spi_communication();
  this->SPI->transfer(REG_Raw_Data_Burst & 0x7f);
  delayMicroseconds(20);  
}

byte PMW3360::read_image_pixel()
{
  byte pixel = this->SPI->transfer(0);
  delayMicroseconds(20);
  return pixel;
}

void PMW3360::end_image()
{
  end_spi_communication();
  end_spi_transaction();
}

bool PMW3360::begin_liftoff_calibration(const byte min_length_val, const types::byte timeout_val) {
  if (min_length_val > 0xF9) { // range of min_length_val is 0x00 (2 freedom units) to 0xF9 (500 freedom units).
    return false;
  }
  if (timeout_val > 0xF9) { // range of timeout_val is 0x00 (0.5s) to 0xF9 (125s)
    return false;
  }
  if (this->operation_mode != OperationMode::liftoff_calibration) {
    return false;
  }
  delay(30); // according to datasheet
  begin_spi_transaction();
  write_register(REG_LiftCutoff_Tune_Min_Length, min_length_val);
  write_register(REG_LiftCutoff_Tune_Timeout, timeout_val);
  write_register(REG_LiftCutoff_Tune1, 0x80);
  return true;
}

LiftoffCalibrationStatus PMW3360::liftoff_calibration_status(void) {
  byte calib_status = read_register(REG_LiftCutoff_Tune1) & 0x07;
  if (calib_status == 0x01) {
    return LiftoffCalibrationStatus::in_progress;
  }
  else if (calib_status == 0x02) {
    return LiftoffCalibrationStatus::min_length_met;
  }
  else if (calib_status == 0x03) {
    return LiftoffCalibrationStatus::min_length_and_timeout_met;
  }
  else if (calib_status == 0x04) {
    return LiftoffCalibrationStatus::failed_timeout;
  }
  else {
    return LiftoffCalibrationStatus::undefined; // Something went wrong.
  }
}

void PMW3360::cancel_liftoff_calibration(void) {
  write_register(REG_LiftCutoff_Tune1, 0x00);
  end_spi_transaction();
}

LiftoffCalibrationResult PMW3360::end_liftoff_calibration(void) {
  write_register(REG_LiftCutoff_Tune1, 0x00);
  delay(0);
  u8 raw_data_threshold = read_register(REG_LiftCutoff_Tune2) & 0x7F;
  u8 minimum_squal = read_register(REG_LiftCutoff_Tune3) & 0xFF;
  end_spi_transaction();
  LiftoffCalibrationResult result;
  result.raw_data_threshold = raw_data_threshold;
  result.minimum_squal = minimum_squal;
  return result;
}

bool PMW3360::set_minimum_squal(const u8 minimum_squal) {
  if (this->operation_mode != OperationMode::tracking) {
    return false;
  }
  begin_spi_transaction();
  write_register(REG_Min_SQ_Run, minimum_squal);
  end_spi_transaction();
  return true;
}

Result<u8, Status> PMW3360::get_minimum_squal(void) {
  if (this->operation_mode != OperationMode::tracking) {
    return {0, Status::error};
  }
  begin_spi_transaction();
  u8 minimum_squal = read_register(REG_Min_SQ_Run);
  end_spi_transaction();
  return {minimum_squal, Status::ok};
}

bool PMW3360::set_raw_data_threshold(const u8 raw_data_threshold) {
  if (this->operation_mode != OperationMode::tracking) {
    return false;
  }
  begin_spi_transaction();
  write_register(REG_Raw_Data_Threshold, raw_data_threshold);
  end_spi_transaction();
  return true;
}

Result<u8, Status> PMW3360::get_raw_data_threshold(void) {
  if (this->operation_mode != OperationMode::tracking) {
    return {0, Status::error};
  }
  begin_spi_transaction();
  u8 raw_data_threshold = read_register(REG_Raw_Data_Threshold);
  end_spi_transaction();
  return {raw_data_threshold, Status::ok};
}

void PMW3360::set_liftoff_height(const LiftoffHeight liftoff_height) {
  begin_spi_transaction();
  if (liftoff_height == LiftoffHeight::_2mm) {
    write_register(REG_Lift_Config, 0x02);
  }
  else if (liftoff_height == LiftoffHeight::_3mm) {
    write_register(REG_Lift_Config, 0x03);
  }
  end_spi_transaction();
  return;
}

Result<LiftoffHeight, Status> PMW3360::get_liftoff_height(void) {
  begin_spi_transaction();
  byte liftoff_height_raw = read_register(REG_Lift_Config);
  end_spi_transaction();
  if (liftoff_height_raw == 0x02) {
    return {LiftoffHeight::_2mm, Status::ok};
  }
  else if (liftoff_height_raw == 0x03) {
    return {LiftoffHeight::_3mm, Status::ok};
  }
  return {LiftoffHeight::_2mm, Status::error};
}

byte PMW3360::read_register(const types::byte register_addr) {
  if(register_addr != REG_Motion_Burst)
  {
    this->in_burst_mode = false;
  }

  begin_spi_communication();
  // send adress of the register, with MSBit = 0 to indicate it's a read
  this->SPI->transfer(register_addr & 0x7f );
  delayMicroseconds(100); // tSRAD is 25, but 100us seems to be stable.
  // read data
  byte data = this->SPI->transfer(0);

  end_spi_communication();
  delayMicroseconds(19); //  tSRW/tSRR (=20us) minus tSCLK-NCS

  return data;
}

void PMW3360::write_register(const byte register_addr, const types::byte data) {
  if(register_addr != REG_Motion_Burst)
  {
    this->in_burst_mode = false;
  }

  begin_spi_communication();
  //send adress of the register, with MSBit = 1 to indicate it's a write
  this->SPI->transfer(register_addr | 0x80 );
  //sent data
  this->SPI->transfer(data);

  delayMicroseconds(20); // tSCLK-NCS for write operation
  end_spi_communication();
  delayMicroseconds(100); // tSWW/tSWR (=120us) minus tSCLK-NCS. Could be shortened, but is looks like a safe lower bound
}

void PMW3360::begin_spi_transaction(void) {
  this->SPI->beginTransaction(spi_settings);
}

void PMW3360::end_spi_transaction(void) {
  delayMicroseconds(1);
  digitalWrite(this->SCS, HIGH);
  this->SPI->endTransaction();
}

void PMW3360::begin_spi_communication(void) {
  digitalWrite(this->SCS, LOW);
  delayMicroseconds(1);
}

void PMW3360::end_spi_communication(void) {
  delayMicroseconds(1);
  digitalWrite(this->SCS, HIGH);
}

void PMW3360::upload_firmware(const byte* firmware_blob, const types::u16 firmware_length) {
  // write 0 to Rest_En bit of Config2 register to disable Rest mode.
  write_register(REG_Config2, 0x00);

  // write 0x1d in SROM_enable reg for initializing
  write_register(REG_SROM_Enable, 0x1d);

  // wait for more than one frame period
  delay(10); // assume that the frame rate is as low as 100fps... even if it should never be that low

  // write 0x18 to SROM_enable to start SROM download
  write_register(REG_SROM_Enable, 0x18);

  // write the SROM file (=firmware data)
  begin_spi_communication();
  this->SPI->transfer(REG_SROM_Load_Burst | 0x80); // write burst destination adress
  delayMicroseconds(15);

  // send all bytes of the firmware
  byte c;
  for (int i = 0; i < firmware_length; i++) {
    c = firmware_blob[i];
    this->SPI->transfer(c);
    delayMicroseconds(15);
  }

  //Read the SROM_ID register to verify the ID before any other register reads or writes.
  read_register(REG_SROM_ID);

  //Write 0x00 (rest disable) to Config2 register for wired mouse or 0x20 for wireless mouse design. 
  write_register(REG_Config2, 0x00);

  end_spi_communication();
}

bool PMW3360::check_signature(const byte SROM_version) {
  begin_spi_transaction();
  byte pid = read_register(REG_Product_ID);
  byte iv_pid = read_register(REG_Inverse_Product_ID);
  byte detected_SROM_ver = read_register(REG_SROM_ID);
  end_spi_transaction();
  return (pid==0x42 && iv_pid == 0xBD && detected_SROM_ver == SROM_version);
}

}
