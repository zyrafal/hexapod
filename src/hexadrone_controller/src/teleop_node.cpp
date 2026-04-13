#include "hexadrone_core/brain.hpp"
#include "hexadrone_controller/bridge_ops.hpp"
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

class TeleopNode : public rclcpp::Node
{
public:
    TeleopNode() : Node("teleop_node"), last_input_{}
    {
        // 1. Publisher for Webots/Servo commands
        pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>(
            "/forward_position_controller/commands", 10);

        // 2. Subscriber for RadioMaster (Joy)
        sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "/joy", 10, [this](const sensor_msgs::msg::Joy::SharedPtr msg) {
                // Use BridgeOps to translate raw axes into our ControllerInput struct
                last_input_ = Hexadrone::BridgeOps::translate(msg->axes);
            });

        // 3. Control Loop: 60Hz (approx 16.6ms)
        timer_ = this->create_wall_timer(16ms, std::bind(&TeleopNode::update, this));
        
        last_time_ = this->now();
        
        RCLCPP_INFO(this->get_logger(), "Hexadrone Teleop Node Started at 60Hz");
    }

private:
    void update()
    {
        // A. Timing: Calculate dt for smoothing and gait logic
        auto current_time = this->now();
        float dt = (current_time - last_time_).seconds();
        last_time_ = current_time;

        // B. Process: Send input through the Brain's layered logic
        // returns std::vector<float> of joint angles in degrees
        std::vector<float> angles_deg = brain_.update(dt, last_input_);

        // C. Convert degrees → radians for forward_position_controller
        std_msgs::msg::Float64MultiArray out;
        out.data.resize(angles_deg.size());
        constexpr double DEG_TO_RAD = M_PI / 180.0;
        for (size_t i = 0; i < angles_deg.size(); ++i)
            out.data[i] = angles_deg[i] * DEG_TO_RAD;
        
        pub_->publish(out);
    }

    // Members
    Hexadrone::Brain brain_;
    Hexadrone::ControllerInput last_input_;
    
    rclcpp::Time last_time_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr pub_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr sub_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TeleopNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
