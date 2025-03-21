#include <string>
#include "types.hpp"
#include "status.hpp"

namespace driver {
namespace Status {

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
        description += "Overcurrent detected on high-side FET (short to GND) on OUT1. ";
    if (status & static_cast<types::u8>(STATUS1::OCP_L1))
        description += "Overcurrent detected on low-side FET (short to VM) on OUT1. ";
    if (status & static_cast<types::u8>(STATUS1::OCP_H2))
        description += "Overcurrent detected on high-side FET (short to GND) on OUT2. ";
    if (status & static_cast<types::u8>(STATUS1::OCP_L2))
        description += "Overcurrent detected on low-side FET (short to VM) on OUT2. ";

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

}
}