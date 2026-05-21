#pragma once

#include "dm_openarm/types.hpp"

#include <vector>

namespace dm_openarm::backend {

class MotorBackend {
public:
  virtual ~MotorBackend() = default;

  virtual void connect() = 0;
  virtual void disconnect() = 0;
  virtual bool connected() const noexcept = 0;
  virtual void send_mit_all(const std::vector<MitCommand>& commands) = 0;
  virtual void send_zero_mit_all() = 0;
  virtual std::vector<MotorState> states() const = 0;
};

}  // namespace dm_openarm::backend
