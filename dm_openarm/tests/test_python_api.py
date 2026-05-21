from pathlib import Path

from dm_openarm import Arm


class FakeState:
    def __init__(self, can_id: int, position: float):
        self.can_id = can_id
        self.position = position


def assert_close(actual: float, expected: float) -> None:
    assert abs(actual - expected) < 1e-9, f"{actual} != {expected}"


def test_move_to_and_hold_at_update_commands() -> None:
    config_path = Path(__file__).resolve().parents[1] / "config" / "arm_5dof.yaml"
    arm = Arm.from_yaml(config_path)

    arm.move_to(0x01, q=0.25, kp=12.0, kd=0.6)
    arm.hold_at(0x02, q=-0.10, kp=10.0, kd=0.8)

    commands = arm.commands()

    assert_close(commands[0].kp, 12.0)
    assert_close(commands[0].kd, 0.6)
    assert_close(commands[0].q, 0.25)
    assert_close(commands[0].dq, 0.0)
    assert_close(commands[0].tau, 0.0)

    assert_close(commands[1].kp, 10.0)
    assert_close(commands[1].kd, 0.8)
    assert_close(commands[1].q, -0.10)
    assert_close(commands[1].dq, 0.0)
    assert_close(commands[1].tau, 0.0)


def test_wait_until_reached_returns_true_when_position_enters_tolerance() -> None:
    arm = object.__new__(Arm)
    samples = [
        [FakeState(0x01, 0.00)],
        [FakeState(0x01, 0.20)],
        [FakeState(0x01, 0.249)],
    ]
    arm.states = lambda: samples.pop(0)

    reached = arm.wait_until_reached(0x01, q=0.25, pos_tol=0.01, timeout=0.1, poll_period=0.0)

    assert reached


def test_wait_until_reached_returns_false_on_timeout() -> None:
    arm = object.__new__(Arm)
    arm.states = lambda: [FakeState(0x01, 0.00)]

    reached = arm.wait_until_reached(0x01, q=0.25, pos_tol=0.01, timeout=0.0, poll_period=0.0)

    assert not reached


def main() -> None:
    test_move_to_and_hold_at_update_commands()
    test_wait_until_reached_returns_true_when_position_enters_tolerance()
    test_wait_until_reached_returns_false_on_timeout()


if __name__ == "__main__":
    main()
