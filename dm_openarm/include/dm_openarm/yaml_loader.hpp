#pragma once

#include "dm_openarm/config.hpp"

#include <filesystem>

namespace dm_openarm {

ArmConfig load_arm_config(const std::filesystem::path& path);

}  // namespace dm_openarm
