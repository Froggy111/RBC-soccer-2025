#include <string>
#include "types.hpp"
#include "registers.hpp"

namespace driver {

namespace STATUS {
std::string get_status1_description(types::u8 status) {
  std::string description;

  if (status & static_cast<types::u8>(STATUS1::OLA1))
    description += "Open load condition detected on OUT1. ";
  if (status & static_cast<types::u8>(STATUS1::OLA2))
    description += "Open load condition detected on OUT2. ";
  if (status & static_cast<types::u8>(STATUS1::ITRIP_CMP))
    description += "Load current reached ITRIP regulation level. ";
  if (status & static_cast<types::u8>(STATUS1::ACTIVE))
    description += "Device is in ACTIVE state. ";
  if (status & static_cast<types::u8>(STATUS1::OCP_H1))
    description +=
        "Overcurrent detected on high-side FET (short to GND) on OUT1. ";
  if (status & static_cast<types::u8>(STATUS1::OCP_L1))
    description +=
        "Overcurrent detected on low-side FET (short to VM) on OUT1. ";
  if (status & static_cast<types::u8>(STATUS1::OCP_H2))
    description +=
        "Overcurrent detected on high-side FET (short to GND) on OUT2. ";
  if (status & static_cast<types::u8>(STATUS1::OCP_L2))
    description +=
        "Overcurrent detected on low-side FET (short to VM) on OUT2. ";

  return description.empty() ? "No faults detected." : description;
}

std::string get_status2_description(types::u8 status) {
  std::string description;

  if (status & static_cast<types::u8>(STATUS2::DRVOFF_STAT))
    description += "DRVOFF pin is high. ";
  if (status & static_cast<types::u8>(STATUS2::ACTIVE))
    description += "Device is in ACTIVE state. ";
  if (status & static_cast<types::u8>(STATUS2::OLP_CMP))
    description += "Off-state diagnostics (OLP) comparator output is set. ";

  return description.empty() ? "No faults detected." : description;
}
} // namespace STATUS

namespace FAULT {
std::string get_fault_description(types::u8 fault) {
  std::string description = "Faults: ";
  if (fault & (types::u8)FAULT::SPI_ERR) {
    description += "SPI_ERR ";
  }
  if (fault & (types::u8)FAULT::POR) {
    description += "POR ";
  }
  if (fault & (types::u8)FAULT::FAULT) {
    description += "FAULT ";
  }
  if (fault & (types::u8)FAULT::VMOV) {
    description += "VMOV ";
  }
  if (fault & (types::u8)FAULT::VMUV) {
    description += "VMUV ";
  }
  if (fault & (types::u8)FAULT::OCP) {
    description += "OCP ";
  }
  if (fault & (types::u8)FAULT::TSD) {
    description += "TSD ";
  }
  if (fault & (types::u8)FAULT::OLA) {
    description += "OLA ";
  }
  return description;
}
} // namespace FAULT
} // namespace driver