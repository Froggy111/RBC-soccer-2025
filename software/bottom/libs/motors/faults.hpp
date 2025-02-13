#pragma once
#include "libs/utils/types.hpp"
#include <string>

namespace Fault {
enum class Fault : types::u8 {
  SPI_ERR = (1 << 7),   // SPI communication fault occurred in the previous SPI frame
  POR = (1 << 6),       // Power-on-reset detected
  FAULT = (1 << 5),     // Logic OR of SPI_ERR, POR, VMOV, VMUV, OCP, TSD & OLA
  VMOV = (1 << 4),      // VM overvoltage detected
  VMUV = (1 << 3),      // VM undervoltage detected
  OCP = (1 << 2),       // Overcurrent detected in one or more power FETs
  TSD = (1 << 1),       // Over temperature detected
  OLA = (1 << 0)        // Open load condition detected in ACTIVE state
};

std::string get_fault_description(types::u8 fault);
}