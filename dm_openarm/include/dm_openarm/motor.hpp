#pragma once

#include "dm_openarm/config.hpp"
#include "dm_openarm/types.hpp"

namespace dm_openarm {

class Motor {
public:
  explicit Motor(MotorConfig config);

  const MotorConfig& config() const noexcept;
  const MotorState& state() const noexcept;
  void set_state(MotorState state) noexcept;

private:
  MotorConfig config_;
  MotorState state_;
};

}  // namespace dm_openarm
