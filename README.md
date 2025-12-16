Проект для управления роботом в симуляции ROS2 Humble + Gazebo с использованием JSON-команд.

---

## 🛠 Требования

* **Windows 10/11** с WSL2
* **Ubuntu 22.04 LTS** (через WSL2)
* **ROS2 Humble**
* **Gazebo** (ROS2-пакет `ros-humble-gazebo-ros-pkgs`)
* Python 3.10+ (для нод на Python)

---

## 1) Установка ROS2 Humble + Gazebo на WSL2

1. Обновляем систему и устанавливаем нужные пакеты:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y curl gnupg lsb-release build-essential
```

2. Добавляем репозиторий ROS2 Humble:

```bash
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    | sudo gpg --dearmor -o /usr/share/keyrings/ros-archive-keyring.gpg

echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
http://packages.ros.org/ros2/ubuntu $(lsb_release -cs) main" \
    | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

sudo apt update
```

3. Устанавливаем ROS2 Humble Desktop:

```bash
sudo apt install ros-humble-desktop -y
echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

4. Устанавливаем Gazebo для симуляции:

```bash
sudo apt install ros-humble-gazebo-ros-pkgs -y
gazebo
```

---

## 2)Создание ROS2 workspace

```bash
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws
colcon build
source install/setup.bash
```

---

## 3) Установка пакета agroscout_control

1. Создание пакета:

```bash
cd ~/ros2_ws/src
ros2 pkg create agroscout_control \
  --build-type ament_python \
  --dependencies rclpy geometry_msgs nav_msgs
```

2. Основные файлы пакета:

| Файл/папка            | Назначение                     |
| --------------------- | ------------------------------ |
| `agroscout_control/`  | Python-ноды                    |
| `command_executor.py` | Исполнение команд из JSON      |
| `package.xml`         | Описание пакета и зависимостей |
| `setup.py`            | Регистрация нод                |
| `resource/`           | Метаданные                     |
| `test/`               | Тесты                          |

3. JSON с командами:

```bash
cd ~/ros2_ws
mkdir data
nano data/commands.json
```

Пример структуры:

```json
{
  "commands": [
    { "cmd": "rotate", "data": { "delta_angle": 90 } },
    { "cmd": "move", "data": { "distance_m": 2 } }
  ]
}
```

---

## 4) Сборка workspace

```bash
cd ~/ros2_ws
colcon build --symlink-install
source install/setup.bash
```

> `--symlink-install` позволяет сразу применять изменения в Python-коде.

---

## 5) Запуск системы

### Терминал 1 - Gazebo

```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
export TURTLEBOT3_MODEL=burger
export DISPLAY=:0

ros2 launch turtlebot3_gazebo turtlebot3_world.launch.py
```

### Терминал 2 - RViz (визуализация)

```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
rviz2
```

* **Fixed Frame**: `world`
* Добавьте дисплеи:

  * **RobotModel** (`/robot_description`)
  * **TF**
  * **Odometry** (`/odom`)
  * **Marker** (`/visualization_marker`)

### Терминал 3 - Выполнение команд робота

```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
ros2 run agroscout_control command_executor
```

* Узел читает команды из JSON
* Публикует скорости в `/cmd_vel`
* Использует `/odom` для точного движения и поворота
* Контур движения отображается в RViz

---

## 5) Итог

* Робот движется по контуру, привязанному к стартовой позиции
* Контур отображается корректно в RViz
* Все команды выполняются последовательно
