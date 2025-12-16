#!/usr/bin/env python3 (шебанг)


import json
import math
import time
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, Point
from nav_msgs.msg import Odometry
from visualization_msgs.msg import Marker

class CommandExecutor(Node):
    def __init__(self):
        super().__init__('command_executor')

        # Publisher скорости
        self.cmd_pub = self.create_publisher(Twist, '/cmd_vel', 10)

        # Publisher для визуализации контура
        self.marker_pub = self.create_publisher(Marker, '/visualization_marker', 10)

        # Подписка на одометрию
        self.odom_sub = self.create_subscription(
            Odometry,
            '/odom',
            self.odom_callback,
            10
        )

        self.current_yaw = None
        self.current_pos = None
        self.odom_ready = False

        # Загрузка JSON
        with open('/home/samira/ros2_ws/data/commands.json', 'r') as f:
            data = json.load(f)
            self.commands = data['commands']

            # Если есть контур — рисуем
            if 'contour' in data and 'points' in data['contour']:
                # Ждём одометрию, чтобы знать стартовую позицию
                while rclpy.ok() and not self.odom_ready:
                    rclpy.spin_once(self, timeout_sec=0.1)
                self.draw_contour(data['contour']['points'])

        self.get_logger().info(f'Loaded {len(self.commands)} commands')
        self.get_logger().info('Waiting for odometry...')

        # Ждём одометрию перед началом команд
        while rclpy.ok() and not self.odom_ready:
            rclpy.spin_once(self, timeout_sec=0.1)

        self.get_logger().info('Odometry received, starting execution')
        self.execute_commands()

    # ---------- ODOM CALLBACK ----------
    def odom_callback(self, msg):
        self.current_pos = msg.pose.pose.position
        self.current_yaw = self.quaternion_to_yaw(msg.pose.pose.orientation)
        self.odom_ready = True

    # ---------- EXECUTION ----------
    def execute_commands(self):
        for cmd in self.commands:
            if cmd['cmd'] == 'rotate':
                self.rotate(cmd['data']['delta_angle'])
            elif cmd['cmd'] == 'move':
                self.move(cmd['data']['distance_m'])
        self.get_logger().info('All commands executed')

    # ---------- MOVE ----------
    def move(self, distance):
        self.get_logger().info(f'Moving {distance} meters')

        start_x = self.current_pos.x
        start_y = self.current_pos.y

        msg = Twist()
        msg.linear.x = 2.9  # скорость движения вперед

        while rclpy.ok():
            dx = self.current_pos.x - start_x
            dy = self.current_pos.y - start_y
            traveled = math.sqrt(dx*dx + dy*dy)

            if traveled >= distance:
                break

            self.cmd_pub.publish(msg)
            rclpy.spin_once(self, timeout_sec=0.1)

        self.stop_robot()

    # ---------- ROTATE ----------
    def rotate(self, angle_deg):
        self.get_logger().info(f'Rotating {angle_deg} degrees')

        target = math.radians(angle_deg)
        start_yaw = self.current_yaw
        target_yaw = self.normalize(start_yaw + target)

        msg = Twist()
        msg.angular.z = 0.4 if target > 0 else -0.4

        while rclpy.ok():
            error = self.normalize(target_yaw - self.current_yaw)
            if abs(error) < 0.05:  # ~3 градуса
                break
            self.cmd_pub.publish(msg)
            rclpy.spin_once(self, timeout_sec=0.1)

        self.stop_robot()

    # ---------- DRAW CONTOUR ----------
    def draw_contour(self, points):
        marker = Marker()
        marker.header.frame_id = "odom"  
        marker.header.stamp = self.get_clock().now().to_msg()
        marker.type = Marker.LINE_STRIP
        marker.action = Marker.ADD
        marker.scale.x = 0.05  # толщина линии
        marker.color.r = 0.0
        marker.color.g = 1.0
        marker.color.b = 0.0
        marker.color.a = 1.0
        marker.lifetime.sec = 0  # навсегда

        #start_x = self.current_pos.x
        #start_y = self.current_pos.y

        for p in points:
            pt = Point()
            pt.x = float(p["x"]) #+ start_x
            pt.y = float(p["y"]) #+ start_y
            pt.z = float(p["z"])
            marker.points.append(pt)

        # соединяем последнюю точку с первой
        if len(points) > 1:
            first = Point()
            first.x = float(points[0]["x"]) #+ start_x
            first.y = float(points[0]["y"]) #+ start_y
            first.z = float(points[0]["z"])
            marker.points.append(first)

        # публикуем несколько раз, чтобы Gazebo/RViz успел отобразить
        for _ in range(20):
            self.marker_pub.publish(marker)
            rclpy.spin_once(self, timeout_sec=0.05)

    # ---------- HELPERS ----------
    def stop_robot(self):
        self.cmd_pub.publish(Twist())
        time.sleep(0.5)

    def quaternion_to_yaw(self, q):
        siny = 2.0 * (q.w * q.z + q.x * q.y)
        cosy = 1.0 - 2.0 * (q.y*q.y + q.z*q.z)
        return math.atan2(siny, cosy)

    def normalize(self, angle):
        while angle > math.pi:
            angle -= 2*math.pi
        while angle < -math.pi:
            angle += 2*math.pi
        return angle

def main():
    rclpy.init()
    node = CommandExecutor()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
