#pragma once

#include "dm_openarm/motor.hpp"
#include "dm_openarm/types.hpp"

#include <cstddef>
#include <vector>

namespace dm_openarm {

class MotorGroup {
public:
  explicit MotorGroup(const std::vector<MotorConfig>& configs);

  std::size_t size() const noexcept;
  const std::vector<Motor>& motors() const noexcept;
  std::vector<MotorState> states() const;
  void update_states(const std::vector<MotorState>& states);

private:
  std::vector<Motor> motors_;
};

}  // namespace dm_openarm
