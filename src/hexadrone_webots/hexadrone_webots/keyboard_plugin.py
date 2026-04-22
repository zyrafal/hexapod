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
KEY_1     = ord('1')
KEY_2     = ord('2')
KEY_3     = ord('3')
KEY_4     = ord('4')
KEY_5     = ord('5')
KEY_6     = ord('6')

LEG_KEYS  = [KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6]

# Fraction of max throttle sent when W or S is held
KEYBOARD_THROTTLE = 0.5

# Joint offsets (degrees) applied when holding a leg key (1–6)
KEYBOARD_LEG_COXA  = -10.0  
KEYBOARD_LEG_FEMUR = -10.0 
KEYBOARD_LEG_TIBIA = -10.0


class KeyboardPlugin:
    def init(self, webots_node, properties):
        self.__robot = webots_node.robot

        if not rclpy.ok():
            rclpy.init(args=None)
        self.__node = rclpy.create_node('keyboard_plugin')

        self.__keyboard = self.__robot.getKeyboard()
        self.__keyboard.enable(int(self.__robot.getBasicTimeStep()))

        self.__pub = self.__node.create_publisher(Joy, '/joy', 10)

        self.__armed   = -1.0   
        self.__posture =  0.0   
        self.__prev_keys = set()

        self.__node.get_logger().info('KeyboardPlugin ready — Space to arm/disarm.')

    def step(self):
        rclpy.spin_once(self.__node, timeout_sec=0)

        keys = set()
        key = self.__keyboard.getKey()
        while key != -1:
            keys.add(key)
            key = self.__keyboard.getKey()
        if key == -1 and len(self.__prev_keys) > 0:
            pass

        new_keys = keys - self.__prev_keys
        self.__prev_keys = keys

        if KEY_SPACE in new_keys:
            self.__armed = 1.0 if self.__armed < 0 else -1.0
        if KEY_Z in new_keys:
            self.__posture = -1.0  
        if KEY_X in new_keys:
            self.__posture =  0.0  
        if KEY_C in new_keys:
            self.__posture =  1.0  

        if KEY_W in keys:
            gear     = -1.0              
            velocity =  KEYBOARD_THROTTLE
        elif KEY_S in keys:
            gear     =  1.0              
            velocity =  KEYBOARD_THROTTLE
        else:
            gear     =  0.0  
            velocity =  0.0

        yaw  = 1.0 if KEY_A in keys else (-1.0 if KEY_D in keys else 0.0)

        leg_selector = 0.0
        trim_coxa    = 0.0
        trim_femur   = 0.0
        trim_tibia   = 0.0
        
        if self.__armed > 0:
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
            0.0,              # CH1  roll
            0.0,              # CH2  pitch
            velocity,         # CH3  throttle
            yaw,              # CH4  yaw
            self.__armed,     # CH5  arm switch
            0.0,              # CH6  (aux)
            self.__posture,   # CH7  posture
            gear,             # CH8  gear
            0.0,              # CH9  kill
            0.0,              # CH10 (aux)
            trim_coxa,        # CH11 trim coxa
            trim_femur,       # CH12 trim femur
            trim_tibia,       # CH13 trim tibia
            leg_selector,     # CH14 leg selector
        ]
        # msg.buttons = []- COMMENT OUT FOR JOYSTICK USAGE
        # self.__pub.publish(msg) - COMMENT OUT FOR JOYSTICK USAGE
        # AXES ABOVE ARE OUTDATED, ORDER CHANGED.
        # TODO: Rewrite if you need to, but make it that way that it doesn't mess up with the controller
