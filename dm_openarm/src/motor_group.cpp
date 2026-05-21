#include "dm_openarm/motor_group.hpp"

#include <stdexcept>

namespace dm_openarm {

MotorGroup::MotorGroup(const std::vector<MotorConfig>& configs)
{
  motors_.reserve(configs.size());
  for(const auto& config : configs)
  {
    motors_.emplace_back(config);
  }
}

std::size_t MotorGroup::size() const noexcept
{
  return motors_.size();
}

const std::vector<Motor>& MotorGroup::motors() const noexcept
{
  return motors_;
}

std::vector<MotorState> MotorGroup::states() const
{
  std::vector<MotorState> result;
  result.reserve(motors_.size());
  for(const auto& motor : motors_)
  {
    result.push_back(motor.state());
  }
  return result;
}

void MotorGroup::update_states(const std::vector<MotorState>& states)
{
  if(states.size() != motors_.size())
  {
    throw std::invalid_argument("state count does not match motor count");
  }

  for(std::size_t i = 0; i < states.size(); ++i)
  {
    motors_[i].set_state(states[i]);
  }
}

}  // namespace dm_openarm
