#include "dm_openarm/mit_control_command.hpp"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace dm_openarm {
namespace {

std::vector<std::string> split_tokens(const std::string& line)
{
  std::istringstream input(line);
  std::vector<std::string> tokens;
  std::string token;
  while(input >> token)
  {
    tokens.push_back(token);
  }
  return tokens;
}

std::uint16_t parse_can_id(const std::string& text)
{
  std::size_t parsed = 0;
  unsigned long value = 0;
  try
  {
    value = std::stoul(text, &parsed, 0);
  }
  catch(const std::exception&)
  {
    throw std::invalid_argument("invalid CAN ID: " + text);
  }

  if(parsed != text.size() || value > std::numeric_limits<std::uint16_t>::max())
  {
    throw std::invalid_argument("invalid CAN ID: " + text);
  }

  return static_cast<std::uint16_t>(value);
}

double parse_delta(const std::string& text, double max_delta_rad)
{
  std::size_t parsed = 0;
  double value = 0.0;
  try
  {
    value = std::stod(text, &parsed);
  }
  catch(const std::exception&)
  {
    throw std::invalid_argument("invalid delta_rad: " + text);
  }

  if(parsed != text.size())
  {
    throw std::invalid_argument("invalid delta_rad: " + text);
  }

  if(std::abs(value) > max_delta_rad)
  {
    throw std::invalid_argument("move delta exceeds limit");
  }

  return value;
}

void require_arg_count(
  const std::vector<std::string>& tokens,
  std::size_t expected,
  const std::string& command)
{
  if(tokens.size() != expected)
  {
    throw std::invalid_argument(command + " command expects " + std::to_string(expected - 1) + " argument(s)");
  }
}

}  // namespace

MitControlCommand parse_mit_control_command(const std::string& line, double max_delta_rad)
{
  const auto tokens = split_tokens(line);
  if(tokens.empty())
  {
    throw std::invalid_argument("empty command");
  }

  const auto& command = tokens[0];
  if(command == "status")
  {
    require_arg_count(tokens, 1, "status");
    return MitControlCommand{MitControlCommandKind::Status, 0, 0.0};
  }

  if(command == "move")
  {
    require_arg_count(tokens, 3, "move");
    return MitControlCommand{
      MitControlCommandKind::Move,
      parse_can_id(tokens[1]),
      parse_delta(tokens[2], max_delta_rad)};
  }

  if(command == "home")
  {
    require_arg_count(tokens, 2, "home");
    if(tokens[1] == "all")
    {
      return MitControlCommand{MitControlCommandKind::HomeAll, 0, 0.0};
    }
    return MitControlCommand{MitControlCommandKind::HomeOne, parse_can_id(tokens[1]), 0.0};
  }

  if(command == "stop")
  {
    require_arg_count(tokens, 1, "stop");
    return MitControlCommand{MitControlCommandKind::Stop, 0, 0.0};
  }

  throw std::invalid_argument("unknown command: " + command);
}

}  // namespace dm_openarm
