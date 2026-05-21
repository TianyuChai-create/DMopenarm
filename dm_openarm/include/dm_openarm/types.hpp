#pragma once

#include <cstdint>

namespace dm_openarm {

enum class MotorModel {
  DM4310,
  DM8009,
};

enum class ControlMode {
  MIT,
};

struct MitCommand {
  double kp{0.0};
  double kd{0.0};
  double q{0.0};
  double dq{0.0};
  double tau{0.0};
};

struct MotorState {
  std::uint16_t can_id{0};
  std::uint16_t mst_id{0};
  double position{0.0};
  double velocity{0.0};
  double torque{0.0};
  double feedback_interval_s{0.0};
};

}  // namespace dm_openarm
