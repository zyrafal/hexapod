import rclpy
from sensor_msgs.msg import Joy

# Key codes as returned by the Webots Keyboard device (uppercase ASCII)
KEY_W     = ord('W')
KEY_S     = ord('S')
KEY_A     = ord('A')
KEY_D     = ord('D')
KEY_SPACE = ord(' ')
KEY_1     = ord('1')
KEY_2     = ord('2')
KEY_3     = ord('3')
KEY_K     = ord('K')


class KeyboardPlugin:
    """
    Webots robot plugin — reads the Webots window keyboard and publishes
    sensor_msgs/Joy on /joy so the teleop_node Brain loop can consume it.

    Controls:
      W / S        Forward / Back
      A / D        Turn left / right
      Space        Arm / Disarm (toggle)
      1 / 2 / 3   Posture: Crouch / Standard / High
      K            Kill switch
    """

    def init(self, webots_node, properties):
        self.__robot = webots_node.robot

        if not rclpy.ok():
            rclpy.init(args=None)
        self.__node = rclpy.create_node('keyboard_plugin')

        self.__keyboard = self.__robot.getKeyboard()
        self.__keyboard.enable(int(self.__robot.getBasicTimeStep()))

        self.__pub = self.__node.create_publisher(Joy, '/joy', 10)

        self.__armed   = -1.0   # -1 = disarmed, 1 = armed
        self.__posture =  0.0   # -1 = crouch, 0 = standard, 1 = high
        self.__prev_keys = set()

        self.__node.get_logger().info('KeyboardPlugin ready — WASD to move, Space to arm.')

    def step(self):
        rclpy.spin_once(self.__node, timeout_sec=0)

        # Collect all currently pressed keys
        keys = set()
        key = self.__keyboard.getKey()
        while key != -1:
            keys.add(key)
            key = self.__keyboard.getKey()

        # Rising-edge detection for toggle/momentary keys
        new_keys = keys - self.__prev_keys
        self.__prev_keys = keys

        if KEY_SPACE in new_keys:
            self.__armed = 1.0 if self.__armed < 0 else -1.0
        if KEY_1 in new_keys:
            self.__posture = -1.0
        if KEY_2 in new_keys:
            self.__posture =  0.0
        if KEY_3 in new_keys:
            self.__posture =  1.0

        # Continuous movement axes
        velocity = 1.0 if KEY_W in keys else (-1.0 if KEY_S in keys else 0.0)
        yaw      = 1.0 if KEY_A in keys else (-1.0 if KEY_D in keys else 0.0)
        kill     = 1.0 if KEY_K in keys else 0.0

        msg = Joy()
        msg.header.stamp = self.__node.get_clock().now().to_msg()
        msg.axes = [
            0.0,              # CH1  roll        (unused)
            0.0,              # CH2  pitch       (unused)
            velocity,         # CH3  velocity
            yaw,              # CH4  yaw
            self.__armed,     # CH5  arm switch
            0.0,              # CH6  aux
            self.__posture,   # CH7  posture
            0.0,              # CH8  gear        (neutral; velocity sign sets direction)
            kill,             # CH9  kill button
            0.0,              # CH10 aux
            0.0,              # CH11 trim coxa
            0.0,              # CH12 trim femur
            0.0,              # CH13 trim tibia
            0.0,              # CH14 leg selector
        ]
        msg.buttons = []
        self.__pub.publish(msg)
