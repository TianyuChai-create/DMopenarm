#include "dm_openarm/motor.hpp"

#include <utility>

namespace dm_openarm {

Motor::Motor(MotorConfig config)
  : config_(std::move(config))
{
  state_.can_id = config_.can_id;
  state_.mst_id = config_.mst_id;
}

const MotorConfig& Motor::config() const noexcept
{
  return config_;
}

const MotorState& Motor::state() const noexcept
{
  return state_;
}

void Motor::set_state(MotorState state) noexcept
{
  state_ = state;
}

}  // namespace dm_openarm
