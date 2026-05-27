#include "../util/matrix.hpp"

namespace gore {
    class camera {
        private:
            matrix view = matrix(4, 4);
            matrix projectoin = matrix(4, 4);
            float yaw;
            float pitch;
            float roll;
        public:
            vec3 pos;
            vec3 front;
            vec3 up;
            camera () {
                pos = {0.0, 0.0, 0.0};
                front = {0.0, 0.0, -1.0};
                up = {0.0, 1.0, 0.0};
                yaw = -(float)(M_PI / 2.0);
                pitch = 0.0f;
                roll = 0.0f;
            }
            camera (vec3 pos, vec3 front, vec3 up) {
                this->pos = pos;
                this->front = front;
                this->up = up;
                yaw = -(float)(M_PI / 2.0);
                pitch = 0.0f;
                roll = 0.0f;
            }

            float getYaw()   const { return yaw; }
            float getPitch() const { return pitch; }
            float getRoll()  const { return roll; }

            void setYaw   (float v) { yaw   = v; }
            void setPitch (float v) { pitch = v; }
            void setRoll  (float v) { roll  = v; }

            void moveForward (float speed) {
                pos += front * speed;
            }
            void moveBackward (float speed) {
                pos -= front * speed;
            }
            void moveLeft (float speed) {
                pos -= front.crossProduct(up).normalize() * speed;
            }
            void moveRight (float speed) {
                pos += front.crossProduct(up).normalize() * speed;
            }
            void updateFront () {
                //std::cout << "yaw: " << yaw << " pitch: " << pitch << "\n";
                // clamp in radians — 89° expressed as radians
                const float limit = 89.0f * (float)(M_PI / 180.0);
                if (pitch >  limit) pitch =  limit;
                if (pitch < -limit) pitch = -limit;
                //std::cout << front.x << ", " << front.y << ", " << front.z << "\n";
                front.x = std::cosf(yaw) * std::cosf(pitch);
                front.y = std::sinf(pitch);
                front.z = std::sinf(yaw) * std::cosf(pitch);
                front = front.normalize();
                //std::cout << "after\n";
                //std::cout << front.x << ", " << front.y << ", " << front.z << "\n";
            }
    };
}