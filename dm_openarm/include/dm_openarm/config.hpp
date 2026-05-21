#pragma once

#include "dm_openarm/types.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace dm_openarm {

struct MotorConfig {
  std::string name;
  MotorModel model{MotorModel::DM4310};
  ControlMode mode{ControlMode::MIT};
  std::uint16_t can_id{0};
  std::uint16_t mst_id{0};
};

struct ArmConfig {
  std::string usb_serial;
  std::uint32_t nom_baud{1000000};
  std::uint32_t dat_baud{1000000};
  std::chrono::milliseconds loop_period{1};
  std::vector<MotorConfig> motors;
};

}  // namespace dm_openarm
