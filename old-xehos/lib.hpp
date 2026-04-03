#include <cmath>
#include <iostream>

const float rad2deg = 180 / M_PI;
const float deg2rad = 1 / rad2deg;


struct Vec3 {
    float x, y, z;

    // --- Constructors ---
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // --- Vector Arithmetic (Operator Overloading) ---
    
    // Addition
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    // Subtraction
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    // Scalar Multiplication
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    // Scalar Division
    Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    // --- Compound Operators (+=, -=, etc.) ---
    Vec3& operator+=(const Vec3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    // --- Vector Operations ---

    // Dot Product: Calculates the cosine of the angle between two vectors
    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Cross Product: Returns a vector perpendicular to both input vectors
    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // Magnitude (Length) squared: Faster (avoids sqrt), good for comparisons
    float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    // Magnitude (Length)
    float length() const {
        return std::sqrt(lengthSquared());
    }

    // Normalize: Returns a unit vector (length of 1) in the same direction
    Vec3 normalize() const {
        float len = length();
        if (len > 0) return *this / len;
        return Vec3(0,0,0);
    }

    Vec3 rotate(const Vec3& axis, float angleRad) const {
        Vec3 pivot = Vec3();
       
        // 1. Shift the point so the pivot acts as the origin
        Vec3 v = *this - pivot;

        // 2. Ensure the axis is normalized (unit vector)
        Vec3 k = axis.normalize();

        // 3. Calculate Trigonometry
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);

        // 4. Apply Rodrigues' Rotation Formula
        // v_rot = v * cos(theta) + (k x v) * sin(theta) + k * (k . v) * (1 - cos(theta))
  
        Vec3 crossProd = k.cross(v);
        float dotProd = k.dot(v);

        Vec3 v_rot = v * c + crossProd * s + k * dotProd * (1.0f - c);

        // 5. Shift back to the original pivot position
        return v_rot + pivot;
    }

    // --- Utility ---
    
    // Print helper
    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};
