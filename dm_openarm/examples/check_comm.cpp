#include "dm_openarm/dm_arm.hpp"
#include "dm_openarm/yaml_loader.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

namespace {

std::filesystem::path config_path(int argc, char** argv)
{
  if(argc > 1)
  {
    return argv[1];
  }
  return "../config/arm_5dof.yaml";
}

void print_states(const std::vector<dm_openarm::MotorState>& states)
{
  for(const auto& state : states)
  {
    std::cout << "canid is: " << state.can_id
              << " pos: " << state.position
              << " vel: " << state.velocity
              << " effort: " << state.torque
              << " time(s): " << state.feedback_interval_s << '\n';
  }
}

}  // namespace

int main(int argc, char** argv)
{
  try
  {
    const auto config = dm_openarm::load_arm_config(config_path(argc, argv));
    dm_openarm::DmArm arm(config);
    arm.connect();

    for(int i = 0; i < 100; ++i)
    {
      arm.send_zero_mit_all();
      print_states(arm.states());
      std::this_thread::sleep_for(config.loop_period);
    }

    arm.disable();
  }
  catch(const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }

  return 0;
}
