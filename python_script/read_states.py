from dm_openarm import Arm


def main() -> None:
    arm = Arm.from_yaml("dm_openarm/config/arm_5dof.yaml")
    arm.enable()

    for state in arm.states():
        print(state.can_id, state.position, state.velocity, state.torque)

    arm.disable()


if __name__ == "__main__":
    main()
