#include "faults.hpp"

namespace driver {
namespace Fault {
std::string get_fault_description(types::u8 fault) {
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
} // namespace Fault
}