#ifndef HEXADRONE_LIB_HPP
#define HEXADRONE_LIB_HPP

#include <cmath>
#include <algorithm>

namespace hexadrone
{
    namespace core
    {

        // --- Constants ---
        constexpr float PI_F = 3.14159265358979323846f;
        constexpr float RAD_TO_DEG_F = 180.0f / PI_F;
        constexpr float DEG_TO_RAD_F = PI_F / 180.0f;

        struct Vec3
        {
            float x, y, z;

            // --- Constructors ---
            Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
            Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

            // --- Vector Arithmetic ---
            Vec3 operator+(const Vec3 &other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
            Vec3 operator-(const Vec3 &other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
            Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
            Vec3 operator/(float scalar) const { return (scalar != 0.0f) ? Vec3(x / scalar, y / scalar, z / scalar) : Vec3(); }

            Vec3 &operator+=(const Vec3 &other)
            {
                x += other.x;
                y += other.y;
                z += other.z;
                return *this;
            }
            Vec3 &operator-=(const Vec3 &other)
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                return *this;
            }

            // --- Vector Operations ---
            float dot(const Vec3 &other) const { return x * other.x + y * other.y + z * other.z; }

            Vec3 cross(const Vec3 &other) const
            {
                return Vec3(
                    y * other.z - z * other.y,
                    z * other.x - x * other.z,
                    x * other.y - y * other.x);
            }

            float lengthSquared() const { return x * x + y * y + z * z; }
            float length() const { return std::sqrt(lengthSquared()); }

            Vec3 normalize() const
            {
                float len = length();
                return (len > 0.00001f) ? *this / len : Vec3(0, 0, 0);
            }

            Vec3 rotateX(float angleRad) const
            {
                float c = std::cos(angleRad);
                float s = std::sin(angleRad);
                return Vec3(x, y * c - z * s, y * s + z * c);
            }

            Vec3 rotateY(float angleRad) const
            {
                float c = std::cos(angleRad);
                float s = std::sin(angleRad);
                return Vec3(x * c + z * s, y, -x * s + z * c);
            }

            Vec3 rotateZ(float angleRad) const
            {
                float c = std::cos(angleRad);
                float s = std::sin(angleRad);
                return Vec3(x * c - y * s, x * s + y * c, z);
            }

            // --- General Rotation (Rodrigues' Formula) ---
            Vec3 rotate(const Vec3 &axis, float angleRad, const Vec3 &pivot = Vec3(0, 0, 0)) const
            {
                Vec3 v = *this - pivot;
                Vec3 k = axis.normalize();
                float c = std::cos(angleRad);
                float s = std::sin(angleRad);

                // v_rot = v*cos + (k x v)*sin + k*(k.v)*(1-cos)
                Vec3 v_rot = v * c + k.cross(v) * s + k * k.dot(v) * (1.0f - c);
                return v_rot + pivot;
            }

            // --- Utility Functions ---
            static Vec3 lerp(const Vec3 &start, const Vec3 &end, float alpha)
            {
                alpha = std::max(0.0f, std::min(1.0f, alpha));
                return start + (end - start) * alpha;
            }

            void clamp(const Vec3 &min, const Vec3 &max)
            {
                x = std::max(min.x, std::min(max.x, x));
                y = std::max(min.y, std::min(max.y, y));
                z = std::max(min.z, std::min(max.z, z));
            }
        };

    } // namespace core
} // namespace hexadrone

#endif // HEXADRONE_LIB_HPP
