#include <string>
#include "types.hpp"

namespace Status {
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
