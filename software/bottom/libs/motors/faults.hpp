#pragma once
#include "libs/utils/types.hpp"
#include <string>

namespace Fault {
enum class Fault : types::u8 {
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

inline std::string get_fault_description(types::u8 fault) {
  std::string description = "Faults: ";
  if (fault & (types::u8)Fault::SPI_ERR) {
    description += "SPI_ERR ";
  }
  if (fault & (types::u8)Fault::POR) {
    description += "POR ";
  }
  if (fault & (types::u8)Fault::FAULT) {
    description += "FAULT ";
  }
  if (fault & (types::u8)Fault::VMOV) {
    description += "VMOV ";
  }
  if (fault & (types::u8)Fault::VMUV) {
    description += "VMUV ";
  }
  if (fault & (types::u8)Fault::OCP) {
    description += "OCP ";
  }
  if (fault & (types::u8)Fault::TSD) {
    description += "TSD ";
  }
  if (fault & (types::u8)Fault::OLA) {
    description += "OLA ";
  }
  return description;
}
}