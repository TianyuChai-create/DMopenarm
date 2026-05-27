# DM OpenArm

`DM OpenArm` 是一个面向达妙电机 USB-CANFD 控制器的机械臂控制库，按
OpenArm 风格重新封装了 5 自由度达妙机械臂的控制接口。项目核心为 C++17，
并通过 `nanobind` 提供 Python API。

当前版本主要支持 MIT 控制模式：

- C++ 负责连接 USB-CANFD、发送电机控制帧、读取状态和运行后台控制循环。
- Python 负责加载 YAML 配置、使能/失能机械臂、更新 MIT 目标命令和读取状态。
- 电机配置默认从 `dm_openarm/config/arm_5dof.yaml` 读取。

## 目录结构

| 路径 | 说明 |
| --- | --- |
| `dm_openarm/` | C++ 库、Python 绑定、配置、测试和示例程序 |
| `dm_openarm/config/arm_5dof.yaml` | 默认 5 自由度机械臂配置 |
| `dm_openarm/examples/` | C++ 示例 |
| `dm_openarm/python/dm_openarm/` | Python 封装 |
| `python_script/` | 仓库根目录下的 Python 示例脚本 |
| `resources/` | 上游或参考资源 |

## 硬件配置

默认配置适配当前 5 自由度达妙机械臂：

| 项目 | 默认值 |
| --- | --- |
| USB-CANFD SN | `14AA044B241402B10DDBDAFE448040BB` |
| nominal baud | `1000000` |
| data baud | `1000000` |
| 控制模式 | `MIT` |
| 控制频率 | `1000 Hz` |

默认电机列表：

| CAN ID | MST ID | 型号 | YAML 名称 | 位置 |
| --- | --- | --- | --- | --- |
| `0x01` | `0x11` | `DM4310` | `end_effector` | 末端 |
| `0x02` | `0x12` | `DM4310` | `wrist_2` | 末端上数第二 |
| `0x03` | `0x13` | `DM4310` | `wrist_3` | 末端上数第三 |
| `0x04` | `0x14` | `DM8009` | `elbow` | 肘部 |
| `0x05` | `0x15` | `DM8009` | `shoulder` | 肩部 |

如果 USB-CANFD 序列号、波特率或电机 ID 不同，先修改
`dm_openarm/config/arm_5dof.yaml`。只修改 YAML 不需要重新编译。

## 单位约定

| 字段 | 含义 | 单位 |
| --- | --- | --- |
| `q` | 目标绝对位置 | rad |
| `dq` | 目标速度 | rad/s |
| `tau` | 前馈力矩 | N·m |
| `kp` | 位置增益 | N·m/rad |
| `kd` | 速度阻尼增益 | N·m/(rad/s) |
| `position` | 反馈位置 | rad |
| `velocity` | 反馈速度 | rad/s |
| `torque` | 反馈力矩 | N·m |
| `feedback_interval_s` | 反馈间隔 | s |
| `hz` | 控制频率 | Hz |

`q` 是绝对位置，不是相对位移。例如 `q=0.8` 表示运动到绝对位置
`0.8 rad`，不是在当前位置上增加 `0.8 rad`。

## Python 安装

先安装系统依赖，例如 Ubuntu/Debian：

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libusb-1.0-0-dev libudev-dev libyaml-cpp-dev
```

从仓库根目录安装 Python 包：

```bash
python -m pip install ./dm_openarm
```

开发调试时可以使用 editable install：

```bash
python -m pip install -e ./dm_openarm
```

也可以进入包目录后安装：

```bash
cd dm_openarm
python -m pip install .
```

注意：仓库根目录没有 `pyproject.toml`，不要在根目录直接执行
`python -m pip install .`。

## Python 使用方法

最小示例：

```python
from dm_openarm import Arm

arm = Arm.from_yaml("dm_openarm/config/arm_5dof.yaml")

try:
    arm.enable()
    arm.start_mit_loop(hz=1000.0)

    arm.mit(0x01, kp=12.0, kd=0.6, q=0.4, dq=0.0, tau=0.0)

finally:
    arm.stop_mit_loop()
    arm.disable()
```

如果脚本从 `dm_openarm/` 目录内运行，配置路径使用：

```python
Arm.from_yaml("config/arm_5dof.yaml")
```

如果脚本从仓库根目录运行，配置路径使用：

```python
Arm.from_yaml("dm_openarm/config/arm_5dof.yaml")
```

### 读取状态

```python
from dm_openarm import Arm

arm = Arm.from_yaml("dm_openarm/config/arm_5dof.yaml")

try:
    arm.enable()
    for state in arm.states():
        print(
            state.can_id,
            state.mst_id,
            state.position,
            state.velocity,
            state.torque,
            state.feedback_interval_s,
        )
finally:
    arm.disable()
```

### 多电机 MIT 控制

```python
from dm_openarm import Arm, MitCommand

arm = Arm.from_yaml("dm_openarm/config/arm_5dof.yaml")

try:
    arm.enable()
    arm.start_mit_loop(hz=1000.0)

    arm.mit({
        0x01: MitCommand(kp=12.0, kd=0.6, q=0.0, dq=0.0, tau=0.0),
        0x02: MitCommand(kp=12.0, kd=0.6, q=0.5, dq=0.0, tau=0.0),
        0x04: MitCommand(kp=10.0, kd=0.8, q=-0.3, dq=0.0, tau=0.0),
    })

finally:
    arm.stop_mit_loop()
    arm.disable()
```

多电机字典中没有出现的电机会保持之前设置的命令。

### 设置零位

把单个电机当前位置设置为零位：

```python
arm.enable()
arm.set_zero(0x01, persist=True)
arm.disable()
```

把全部电机当前位置设置为零位：

```python
arm.enable()
arm.set_zero_all(persist=True)
arm.disable()
```

`persist=True` 会写入电机 flash，断电后仍然保留；不要频繁反复写入。
`persist=False` 只做临时设零。

### 运行 Python 示例

先安装包：

```bash
python -m pip install ./dm_openarm
```

从仓库根目录运行：

```bash
python python_script/enable_disable.py
python python_script/read_states.py
python python_script/mit_control_one_motor.py
```

| 脚本 | 作用 |
| --- | --- |
| `enable_disable.py` | 使能所有电机，等待回车后失能 |
| `read_states.py` | 使能后读取并打印电机状态 |
| `mit_control_one_motor.py` | 启动 MIT loop 并控制单个电机 |

## Python API 使用列表

### 高层 API：`dm_openarm.Arm`

| API | 参数 | 说明 |
| --- | --- | --- |
| `Arm.from_yaml(path)` | `path: str | Path` | 从 YAML 创建机械臂对象；不会自动连接硬件 |
| `arm.enable()` | 无 | 连接 USB-CANFD 并使能配置中的全部电机 |
| `arm.disable()` | 无 | 停止 MIT loop 并失能电机 |
| `arm.states()` | 无 | 返回全部电机状态列表 |
| `arm.set_zero(can_id, persist=True)` | `can_id: int`, `persist: bool` | 将指定电机当前位置设置为零位 |
| `arm.set_zero_all(persist=True)` | `persist: bool` | 将全部电机当前位置设置为零位 |
| `arm.start_mit_loop(hz=1000.0, zero_timeout=5.0)` | `hz: float`, `zero_timeout: float` | 启动后台 MIT 控制循环，并先让电机回到保存零位 |
| `arm.stop_mit_loop()` | 无 | 停止后台 MIT 控制循环 |
| `arm.mit(can_id, kp=0, kd=0, q=0, dq=0, tau=0)` | `can_id: int` 和 MIT 参数 | 更新单个电机 MIT 命令 |
| `arm.mit(commands)` | `dict[int, MitCommand]` | 批量更新多个电机 MIT 命令 |
| `arm.commands()` | 无 | 返回当前 MIT 命令列表 |
| `arm.mit_loop_running` | 属性 | 返回后台 MIT loop 是否正在运行 |

`Arm` 也支持上下文管理器：

```python
from dm_openarm import Arm

with Arm.from_yaml("dm_openarm/config/arm_5dof.yaml") as arm:
    print(arm.states())
```

### 数据类型

| 类型 | 字段/枚举值 | 说明 |
| --- | --- | --- |
| `MitCommand` | `kp`, `kd`, `q`, `dq`, `tau` | MIT 控制命令 |
| `MotorState` | `can_id`, `mst_id`, `position`, `velocity`, `torque`, `feedback_interval_s` | 电机反馈状态 |
| `MotorConfig` | `name`, `model`, `mode`, `can_id`, `mst_id` | 单个电机配置 |
| `ArmConfig` | `usb_serial`, `nom_baud`, `dat_baud`, `motors` | 机械臂配置 |
| `MotorModel` | `DM4310`, `DM8009` | 支持的电机型号 |
| `ControlMode` | `MIT` | 支持的控制模式 |

### 底层 Python 绑定：`dm_openarm._core`

通常优先使用高层 `Arm`。如果需要更直接的控制，可以使用 `_core` 暴露的底层
C++ 绑定：

| API | 说明 |
| --- | --- |
| `_core.load_arm_config(path)` | 加载 YAML 并返回 `ArmConfig` |
| `_core.DmArm(config)` | 创建底层机械臂对象 |
| `DmArm.connect()` | 连接并使能电机 |
| `DmArm.disable()` | 失能电机 |
| `DmArm.connected()` | 查询连接状态 |
| `DmArm.send_mit_all(commands)` | 直接发送全部电机 MIT 命令 |
| `DmArm.send_zero_mit_all()` | 发送零 MIT 命令 |
| `DmArm.set_zero(can_id, persist=True)` | 设置单个电机零位 |
| `DmArm.set_zero_all(persist=True)` | 设置全部电机零位 |
| `DmArm.states()` | 读取电机状态 |
| `_core.MitLoopController(arm)` | 创建后台 MIT 控制器 |
| `MitLoopController.start(hz=1000.0)` | 启动后台控制线程 |
| `MitLoopController.stop()` | 停止后台控制线程 |
| `MitLoopController.running()` | 查询控制线程状态 |
| `MitLoopController.set_command(can_id, command)` | 设置单个电机命令 |
| `MitLoopController.set_all_commands(commands)` | 设置全部电机命令 |
| `MitLoopController.commands()` | 返回当前命令列表 |

## C++ 构建和使用

构建 C++ 库、示例程序和测试：

```bash
cd dm_openarm
mkdir -p build
cd build
cmake ..
make
ctest --output-on-failure
```

构建完成后，示例程序位于 `dm_openarm/build/`：

```bash
cd dm_openarm/build

./check_comm ../config/arm_5dof.yaml
./enable_disable ../config/arm_5dof.yaml
./hold_position ../config/arm_5dof.yaml
./set_zero_position ../config/arm_5dof.yaml all
./arm_mit_control ../config/arm_5dof.yaml
```

### C++ 最小示例

```cpp
#include "dm_openarm/dm_arm.hpp"
#include "dm_openarm/mit_loop_controller.hpp"
#include "dm_openarm/types.hpp"
#include "dm_openarm/yaml_loader.hpp"

#include <chrono>
#include <thread>

int main()
{
  auto config = dm_openarm::load_arm_config("../config/arm_5dof.yaml");
  dm_openarm::DmArm arm(config);

  arm.connect();

  dm_openarm::MitLoopController loop(arm);
  loop.start(1000.0);
  loop.set_command(
    0x01,
    dm_openarm::MitCommand{12.0, 0.6, 0.4, 0.0, 0.0});

  std::this_thread::sleep_for(std::chrono::seconds(2));

  loop.stop();
  arm.disable();
  return 0;
}
```

在其他 CMake 项目中作为源码依赖：

```cmake
add_subdirectory(path/to/dm_openarm)
target_link_libraries(my_app PRIVATE dm_openarm)
```

## C++ API 使用列表

主要头文件：

| 头文件 | 说明 |
| --- | --- |
| `dm_openarm/yaml_loader.hpp` | 加载 YAML 配置 |
| `dm_openarm/dm_arm.hpp` | 连接、失能、发送 MIT、读取状态、设零 |
| `dm_openarm/mit_loop_controller.hpp` | 后台 MIT 控制循环 |
| `dm_openarm/types.hpp` | MIT 命令和状态等基础类型 |
| `dm_openarm/config.hpp` | 机械臂和电机配置结构 |

### `dm_openarm::load_arm_config`

| API | 说明 |
| --- | --- |
| `load_arm_config(path)` | 从 YAML 文件读取并返回 `ArmConfig` |

### `dm_openarm::DmArm`

| API | 说明 |
| --- | --- |
| `DmArm(ArmConfig config)` | 创建机械臂对象 |
| `connect()` | 连接 USB-CANFD 并使能电机 |
| `disable()` | 失能电机 |
| `connected()` | 查询连接状态 |
| `send_mit_all(commands)` | 对全部电机发送 MIT 命令 |
| `send_zero_mit_all()` | 对全部电机发送零 MIT 命令 |
| `set_zero(can_id, persist=true)` | 设置指定电机零位 |
| `set_zero_all(persist=true)` | 设置全部电机零位 |
| `states()` | 返回全部电机状态 |
| `config()` | 返回当前配置 |

### `dm_openarm::MitLoopController`

| API | 说明 |
| --- | --- |
| `MitLoopController(DmArm& arm)` | 创建后台 MIT 控制器 |
| `start(hz=1000.0)` | 启动后台控制线程 |
| `stop()` | 停止后台控制线程 |
| `running()` | 查询线程是否运行 |
| `set_command(can_id, command)` | 设置单个电机命令 |
| `set_all_commands(commands)` | 设置全部电机命令 |
| `commands()` | 返回当前命令列表 |

### 基础类型

| 类型 | 字段/枚举值 | 说明 |
| --- | --- | --- |
| `MitCommand` | `kp`, `kd`, `q`, `dq`, `tau` | MIT 控制命令 |
| `MotorState` | `can_id`, `mst_id`, `position`, `velocity`, `torque`, `feedback_interval_s` | 电机状态 |
| `MotorModel` | `DM4310`, `DM8009` | 电机型号 |
| `ControlMode` | `MIT` | 控制模式 |

## C++ FIFO 示例

`arm_mit_control` 示例会创建 FIFO 控制入口：

```bash
cd dm_openarm/build
./arm_mit_control ../config/arm_5dof.yaml
```

另一个终端发送命令：

```bash
echo "status" > /tmp/dm_openarm_mit_control.cmd
echo "move 0x01 0.08" > /tmp/dm_openarm_mit_control.cmd
echo "home 0x01" > /tmp/dm_openarm_mit_control.cmd
echo "home all" > /tmp/dm_openarm_mit_control.cmd
echo "stop" > /tmp/dm_openarm_mit_control.cmd
```

该示例中的 `move` 是示例程序自己的命令格式，内部有单步限制和斜坡：

- 单次 `move` 限制：`[-0.10, 0.10] rad`
- 目标位置斜坡：约 `0.6 rad/s`
- 默认 MIT 参数：`kp=8.0`，`kd=0.4`，`dq=0.0 rad/s`，`tau=0.0 N·m`

Python 的 `arm.mit(...)` 不会自动应用 FIFO 示例里的单步限制和斜坡。

## 安全注意事项

- `start_mit_loop()` 会让所有电机尝试回到已保存零位 `q=0.0`，运行前确认机械臂没有干涉。
- `set_zero(..., persist=True)` 会写入电机 flash，只在机械零位确认正确后执行。
- `arm.mit(...)` 会立即更新目标命令；Python API 当前没有自动斜坡限制。
- 初次测试建议只控制单个电机，使用小角度、低增益和 `tau=0.0`。
- 所有会运动机械臂的脚本都应使用 `try/finally`，退出时执行 `stop_mit_loop()` 和 `disable()`。

## Related Projects

This library builds on [dm-motor-driver](https://github.com/TianyuChai-create/dm-motor-driver) for low-level motor control.
