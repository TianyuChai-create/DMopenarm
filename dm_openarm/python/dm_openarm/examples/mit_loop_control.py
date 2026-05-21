from dm_openarm import Arm


def move_to_and_wait(arm: Arm, can_id: int, q: float) -> None:
    arm.move_to(can_id, q=q, kp=12.0, kd=0.6)
    if not arm.wait_until_reached(can_id, q=q, pos_tol=0.01, timeout=3.0):
        raise TimeoutError(f"motor 0x{can_id:02X} did not reach {q:.3f} rad")


def main() -> None:
    arm = Arm.from_yaml("config/arm_5dof.yaml")

    try:
        arm.enable()

        arm.start_mit_loop(hz=1000.0)
        move_to_and_wait(arm, 0x01, 0.10)
        move_to_and_wait(arm, 0x01, 0.0)

    finally:
        arm.stop_mit_loop()
        arm.disable()


if __name__ == "__main__":
    main()
