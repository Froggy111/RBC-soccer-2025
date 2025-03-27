#pragma once
#include <string>
#include "types.hpp"

namespace driver {
namespace FAULT {
enum class FAULT : types::u8 {
  SPI_ERR =
      (1 << 7),   // SPI communication fault occurred in the previous SPI frame
  POR = (1 << 6), // Power-on-reset detected
  FAULT = (1 << 5), // Logic OR of SPI_ERR, POR, VMOV, VMUV, OCP, TSD & OLA
  VMOV = (1 << 4),  // VM overvoltage detected
  VMUV = (1 << 3),  // VM undervoltage detected
  OCP = (1 << 2),   // Overcurrent detected in one or more power FETs
  TSD = (1 << 1),   // Over temperature detected
  OLA = (1 << 0)    // Open load condition detected in ACTIVE state
};

/**
 * @brief Returns a string with a description of the fault, based on the register values provided in the fault param.
 * 
 * @param fault 
 * @return std::string 
 */
std::string get_fault_description(types::u8 fault);
} // namespace Fault

namespace STATUS {
enum class STATUS1 : types::u8 {
  OLA1 = (1 << 7),      // Open load condition detected in ACTIVE state on OUT1
  OLA2 = (1 << 6),      // Open load condition detected in ACTIVE state on OUT2
  ITRIP_CMP = (1 << 5), // Load current has reached the ITRIP regulation level
  ACTIVE = (1 << 4),    // Device is in the ACTIVE state
  OCP_H1 =
      (1 << 3), // Overcurrent detected on high-side FET (short to GND) on OUT1
  OCP_L1 =
      (1 << 2), // Overcurrent detected on low-side FET (short to VM) on OUT1
  OCP_H2 =
      (1 << 1), // Overcurrent detected on high-side FET (short to GND) on OUT2
  OCP_L2 =
      (1 << 0) // Overcurrent detected on low-side FET (short to VM) on OUT2
};

enum class STATUS2 : types::u8 {
  DRVOFF_STAT = (1 << 7), // Status of the DRVOFF pin (1b = high)
  ACTIVE = (1 << 4), // Device is in ACTIVE state (copy of bit 4 in STATUS1)
  OLP_CMP = (1 << 0) // Output of the off-state diagnostics (OLP) comparator
};

/**
   * @brief Get a string describing the status1 register as given in the param
   * 
   * @param status 
   * @return std::string 
   */
std::string get_status1_description(types::u8 status);

/**
   * @brief Get a string describing the status2 register as given in the param
   * 
   * @param status 
   * @return std::string 
   */
std::string get_status2_description(types::u8 status);
} // namespace Status

namespace ITRIP {
enum class ITRIP : types::u8 {
  DISABLED = 0b000,
  TRIP_1_18V = 0b001,
  TRIP_1_41V = 0b010,
  TRIP_1_65V = 0b011,
  TRIP_1_98V = 0b100,
  TRIP_2_31V = 0b101,
  TRIP_2_64V = 0b110,
  TRIP_2_97V = 0b111
};
} // namespace ITRIP

namespace OCP {
enum class OCP : types::u8 {
  SETTING_100 = 0b00,
  SETTING_50 = 0b01,
  SETTING_75 = 0b10,
};
} // namespace OCP

} // namespace driver