from __future__ import annotations

import time
from pathlib import Path
from typing import Iterable

from . import _core


class Arm:
    def __init__(self, config: _core.ArmConfig):
        self._arm = _core.DmArm(config)
        self._loop = _core.MitLoopController(self._arm)

    @classmethod
    def from_yaml(cls, path: str | Path) -> "Arm":
        return cls(_core.load_arm_config(str(path)))

    def enable(self) -> None:
        self._arm.connect()

    def disable(self) -> None:
        self.stop_mit_loop()
        self._arm.disable()

    def states(self):
        return self._arm.states()

    def set_zero(self, can_id: int, persist: bool = True) -> None:
        self._arm.set_zero(can_id, persist)

    def set_zero_all(self, persist: bool = True) -> None:
        self._arm.set_zero_all(persist)

    def start_mit_loop(self, hz: float = 1000.0, zero_timeout: float = 5.0) -> None:
        """Start the MIT control loop.

        All motors are first commanded to position 0.  The method
        blocks until every motor reports a position within
        tolerance of 0, or *zero_timeout* seconds elapse.
        """
        self._loop.start(hz)

        # Command all motors to zero position.
        states = self.states()
        self._loop.set_all_commands([
            _core.MitCommand(kp=12.0, kd=0.6, q=0.0, dq=0.0, tau=0.0)
            for _ in states
        ])

        # Wait until all motors are near zero or timeout.
        pos_tol = 0.05  # radians
        deadline = time.monotonic() + zero_timeout
        while True:
            states = self.states()
            if all(abs(s.position) < pos_tol for s in states):
                break
            if time.monotonic() >= deadline:
                break
            time.sleep(0.01)  # ~100 Hz

    def stop_mit_loop(self) -> None:
        if self._loop.running():
            self._loop.stop()

    @property
    def mit_loop_running(self) -> bool:
        return self._loop.running()

    def commands(self):
        return self._loop.commands()

    # ── The unified MIT API ───────────────────────────────────────────

    def mit(
        self,
        target: int | dict[int, _core.MitCommand],
        /,
        kp: float = 0.0,
        kd: float = 0.0,
        q: float = 0.0,
        dq: float = 0.0,
        tau: float = 0.0,
    ) -> None:
        """Send MIT command(s) with full parameter control.

        **Single motor** — pass CAN ID + keyword arguments::

            arm.mit(0x01, kp=12.0, kd=0.6, q=0.8, dq=0.0, tau=0.5)

        **Multiple motors** — pass a dict of CAN ID → MitCommand::

            from dm_openarm import MitCommand
            arm.mit({
                0x01: MitCommand(kp=12.0, kd=0.6, q=0.8),
                0x04: MitCommand(kp=10.0, kd=0.8, q=0.5, tau=1.0),
            })

        Non-targeted motors keep whatever command was previously set
        (typically hold-at-position from :meth:`start_mit_loop`).
        """
        if isinstance(target, dict):
            for can_id, cmd in target.items():
                self._loop.set_command(can_id, cmd)
        else:
            self._loop.set_command(
                target,
                _core.MitCommand(kp=kp, kd=kd, q=q, dq=dq, tau=tau),
            )

    # ── Context manager ───────────────────────────────────────────────

    def __enter__(self) -> "Arm":
        self.enable()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.disable()

    def __del__(self) -> None:
        try:
            self.disable()
        except Exception:
            pass
