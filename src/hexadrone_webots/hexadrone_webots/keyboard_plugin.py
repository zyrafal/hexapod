import rclpy
from sensor_msgs.msg import Joy

# Key codes as returned by the Webots Keyboard device (uppercase ASCII)
KEY_W     = ord('W')
KEY_S     = ord('S')
KEY_A     = ord('A')
KEY_D     = ord('D')
KEY_SPACE = ord(' ')
KEY_Z     = ord('Z')
KEY_X     = ord('X')
KEY_C     = ord('C')
KEY_K     = ord('K')
KEY_1     = ord('1')
KEY_2     = ord('2')
KEY_3     = ord('3')
KEY_4     = ord('4')
KEY_5     = ord('5')
KEY_6     = ord('6')

LEG_KEYS  = [KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6]

# Fraction of max throttle sent when W or S is held (0.0 – 1.0)
KEYBOARD_THROTTLE = 0.05

# Joint offsets (degrees) applied when holding a leg key (1–6)
KEYBOARD_LEG_COXA  = 15.0  # Forward swing (sign map ensures consistent direction for all legs)
KEYBOARD_LEG_FEMUR = -30.0  # Lift up
KEYBOARD_LEG_TIBIA = -60.0  # Tuck tibia to keep foot raised


class KeyboardPlugin:
    """
    Webots robot plugin — reads the Webots window keyboard and publishes
    sensor_msgs/Joy on /joy so the teleop_node Brain loop can consume it.

    Controls:
      W / S        Forward / Reverse (Drive / Reverse gear)
      A / D        Turn left / right (yaw, works in any gear)
      Space        Arm / Disarm (toggle)
      Z / X / C    Posture: Crouch / Standard / High
      1 – 6        Hold to lift leg 1–6 (LF/LM/LB/RF/RM/RB). Only in Neutral gear.
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
        if KEY_Z in new_keys:
            self.__posture = -1.0  # Crouch
        if KEY_X in new_keys:
            self.__posture =  0.0  # Standard
        if KEY_C in new_keys:
            self.__posture =  1.0  # High

        # Continuous movement axes
        # W/S select gear (Drive/Reverse) and push throttle to max
        # A/D control yaw regardless of gear
        if KEY_W in keys:
            gear     = -1.0              # DRIVE
            velocity =  KEYBOARD_THROTTLE
        elif KEY_S in keys:
            gear     =  1.0              # REVERSE
            velocity =  KEYBOARD_THROTTLE
        else:
            gear     =  0.0  # NEUTRAL
            velocity =  0.0

        yaw  = 1.0 if KEY_A in keys else (-1.0 if KEY_D in keys else 0.0)
        kill = 1.0 if KEY_K in keys else 0.0

        # Leg lift: hold 1-6 to raise that leg (works in Neutral gear only)
        leg_selector = 0.0
        trim_coxa    = 0.0
        trim_femur   = 0.0
        trim_tibia   = 0.0
        for i, key in enumerate(LEG_KEYS):
            if key in keys:
                leg_selector = float(i + 1)
                trim_coxa    = KEYBOARD_LEG_COXA
                trim_femur   = KEYBOARD_LEG_FEMUR
                trim_tibia   = KEYBOARD_LEG_TIBIA
                break

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
            gear,             # CH8  gear        (-1: Drive, 0: Neutral, 1: Reverse)
            kill,             # CH9  kill button
            0.0,              # CH10 aux
            trim_coxa,        # CH11 trim coxa
            trim_femur,       # CH12 trim femur
            trim_tibia,       # CH13 trim tibia
            leg_selector,     # CH14 leg selector (1–6)
        ]
        msg.buttons = []
        self.__pub.publish(msg)
