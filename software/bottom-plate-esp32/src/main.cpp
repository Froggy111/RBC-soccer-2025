#include "PMW3360.hpp"
#include "debug.hpp"
#include "types.hpp"
#include <cstdio>

using namespace types;

const u8 pmw3360_scs = 5;
const f32 inch_to_meters = 0.0254;
const u16 CPI = 12000;

PMW3360::PMW3360 sensor;

f32 count_to_meters(i32 count, u16 CPI) {
  return ((float) count / (float) CPI) * inch_to_meters;
}

void setup() {
  Serial.begin(115200);  
  while(!Serial); // wait for serial to start

  SPI.begin();
  
  if (sensor.begin(&SPI, pmw3360_scs, CPI, PMW3360::OperationMode::tracking)) {
    Serial.println("Sensor initialization succeeded");
  }
  else {
    Serial.println("Sensor initialization failed");
  }
}

i32 count_sum_x = 0;
i32 count_sum_y = 0;
i32 raw_count_sum_x = 0;
i32 raw_count_sum_y = 0;
bool running = false;

void loop() {
  std::string input = debug::read_input();
  if (input == "reset") {
    debug::printf("RESETTING\n");
    count_sum_x = 0;
    count_sum_y = 0;
    raw_count_sum_x = 0;
    raw_count_sum_y = 0;
  }
  else if (input == "start") {
    debug::printf("STARTING\n");
    running = true;
  }
  else if (input == "stop") {
    debug::printf("STOPPING\n");
    running = false;
  }
  else if (input == "calibrate") {
    debug::printf("Entering liftoff calibration mode...\n");
    sensor.begin(&SPI, pmw3360_scs, CPI, PMW3360::OperationMode::liftoff_calibration);
    std::string start_input;
    do {
      start_input = debug::get_input("Enter \"start\" to begin calibration: ");
    } while (start_input != "start");
    debug::printf("Starting liftoff calibration....\n");
    sensor.begin_liftoff_calibration();
    debug::printf("Started liftoff calibration.\n");
    PMW3360::LiftoffCalibrationStatus liftoff_calibration_status;
    do {
      liftoff_calibration_status = sensor.liftoff_calibration_status();
      if (liftoff_calibration_status == PMW3360::LiftoffCalibrationStatus::min_length_met) {
        debug::printf(".");
        if (!debug::read_input().empty()) {
          debug::printf("\n");
          break;
        }
      }
      else if (debug::read_input() == "cancel") {
        debug::printf("Calibration failed. Reason: Cancelled by user.");
        break;
      }
    } while (liftoff_calibration_status == PMW3360::LiftoffCalibrationStatus::in_progress ||
          liftoff_calibration_status == PMW3360::LiftoffCalibrationStatus::min_length_met);
    bool calibration_success = false;
    if (liftoff_calibration_status == PMW3360::LiftoffCalibrationStatus::failed_timeout) {
      debug::printf("Calibration failed. Reason: did not reach min_length before timeout.");
      calibration_success = false;
    }
    else if (liftoff_calibration_status == PMW3360::LiftoffCalibrationStatus::min_length_and_timeout_met) {
      debug::printf("Calibration completed. Reason: both min_length and timeout were met.");
      calibration_success = true;
    }
    else if (liftoff_calibration_status == PMW3360::LiftoffCalibrationStatus::min_length_met) {
      debug::printf("Calibration completed. Reason: min_length reached and user stopped calibration.");
      calibration_success = true;
    }
    else if (liftoff_calibration_status == PMW3360::LiftoffCalibrationStatus::undefined) {
      debug::crash("Liftoff calibration status undefined. Crashing.");
      calibration_success = false;
    }

    if (calibration_success) {
      debug::printf("checkpoint 0");
      PMW3360::LiftoffCalibrationResult calibration_result = sensor.end_liftoff_calibration();
      debug::printf("checkpoint 1");
      sensor.begin(&SPI, pmw3360_scs, CPI, PMW3360::OperationMode::tracking);
      debug::printf("checkpoint 2");
      Result<u8, Status> original_min_squal = sensor.get_minimum_squal();
      if (original_min_squal.status != Status::ok) {
        debug::crash("Error reading minimum surface quality.");
      }
      Result<u8, Status> original_raw_data_threshold = sensor.get_raw_data_threshold();
      if (original_raw_data_threshold.status != Status::ok) {
        debug::crash("Error reading raw data threshold.");
      }
      debug::printf("\
  Current minimum surface quality: %u. Current raw data threshold: %u\n\
  Recommended minimum surface quality: %u. Recommended raw data threshold: %u\n",
        original_min_squal.value, original_raw_data_threshold.value,
        calibration_result.minimum_squal, calibration_result.raw_data_threshold);
      std::string choice = debug::get_input("Update values? (Y/n): ");
      if (choice == "Y" || choice == "y" || choice.empty()) {
        debug::printf("Overriding with recommended values.\n");
        sensor.set_minimum_squal(calibration_result.minimum_squal);
        sensor.set_raw_data_threshold(calibration_result.raw_data_threshold);
        debug::printf("Recommended values are now in use.\n");
      }
      else {
        debug::printf("Keeping original values.\n");
      }
    }
    else {
      sensor.cancel_liftoff_calibration();
      sensor.begin(&SPI, pmw3360_scs, CPI, PMW3360::OperationMode::tracking);
    }
  }
  else if (input == "reset_calibrate") {
    sensor.set_minimum_squal();
    sensor.set_raw_data_threshold();
  }
  else if (input == "set_liftoff_height") {
    Result<PMW3360::LiftoffHeight, Status> original_liftoff_height = sensor.get_liftoff_height();
    if (original_liftoff_height.status != Status::ok) {
      debug::crash("Error reading liftoff height.");
    }
    if (original_liftoff_height.value == PMW3360::LiftoffHeight::_2mm) {
      debug::printf("Current liftoff height is: 2mm\n");
    }
    else if (original_liftoff_height.value == PMW3360::LiftoffHeight::_3mm) {
      debug::printf("Current liftoff height is: 3mm\n");
    }
    std::string liftoff_height_choice;
    do {
      liftoff_height_choice = debug::get_input("Enter the desired liftoff height (2mm/3mm): ");
    } while (liftoff_height_choice != "2mm" && liftoff_height_choice != "3mm");
    if (liftoff_height_choice == "2mm") {
      sensor.set_liftoff_height(PMW3360::LiftoffHeight::_2mm);
    }
    else if (liftoff_height_choice == "3mm") {
      sensor.set_liftoff_height(PMW3360::LiftoffHeight::_3mm);
    }
    else {
      debug::crash("What in the bit flip\n");
      sensor.set_liftoff_height();
    }
  }
  else if (input == "reset_liftoff_height") {
    sensor.set_liftoff_height();
  }

  if (running) {
    PMW3360::Data data = sensor.read_burst();
    
    if(data.is_on_surface && data.is_motion)
    {
      count_sum_x += data.dx;
      count_sum_y += -data.dy;
      raw_count_sum_x += abs(data.dx);
      raw_count_sum_y += abs(data.dy);
      f32 change = ((f32) count_sum_y / (f32) raw_count_sum_y) * 100;
      Serial.printf("dx:%i dy:%i csx:%i csy:%i rcsx:%i rcsy:%i delta:%f csxm:%f csym:%f cpi:%i\n",
                    data.dx, -data.dy, count_sum_x, count_sum_y, raw_count_sum_x, raw_count_sum_y, change,
                    count_to_meters(count_sum_x, CPI), count_to_meters(count_sum_y, CPI), sensor.get_CPI());
    }
  }
  
  delay(1);
}
