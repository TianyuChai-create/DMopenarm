#pragma once

#include <cstdint>
#include <string>

namespace dm_openarm {

enum class MitControlCommandKind {
  Status,
  Move,
  HomeOne,
  HomeAll,
  Stop,
};

struct MitControlCommand {
  MitControlCommandKind kind{MitControlCommandKind::Status};
  std::uint16_t can_id{0};
  double delta_rad{0.0};
};

MitControlCommand parse_mit_control_command(
  const std::string& line,
  double max_delta_rad = 0.10);

}  // namespace dm_openarm
