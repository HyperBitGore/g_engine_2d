#pragma once
#include "../util/matrix.hpp"

namespace gore {
    class camera {
        private:
            matrix view = matrix(4, 4);
            matrix projection = matrix(4, 4);
            float yaw;
            float pitch;
            float roll;
            vec3 pos;
            vec3 front;
            vec3 up;
            vec3 world_up;
        public:
            camera () {
                pos = {0.0, 0.0, 0.0};
                front = {0.0, 0.0, -1.0};
                up = {0.0, 1.0, 0.0};
                world_up = up;
                yaw = -(float)(M_PI / 2.0);
                pitch = 0.0f;
                roll = 0.0f;
            }
            camera (vec3 pos, vec3 front, vec3 up, vec3 world_up) {
                this->pos = pos;
                this->front = front;
                this->up = up;
                this->world_up = world_up;
                yaw = -(float)(M_PI / 2.0);
                pitch = 0.0f;
                roll = 0.0f;
            }

            vec3 getPos()      const { return pos; }
            vec3 getFront()    const { return front; }
            vec3 getUp()       const { return up; }
            vec3 getWorldUp()  const { return world_up; }
            float getYaw()     const { return yaw; }
            float getPitch()   const { return pitch; }
            float getRoll()    const { return roll; }

            void setPos      (vec3 v)  { pos = v; }
            void setFront    (vec3 v)  { front = v; }
            void setUp       (vec3 v)  { up = v; }
            void setWorldUp  (vec3 v)  { world_up = v; update(); }
            void setYaw      (float v) { yaw   = v; update(); }
            void setPitch    (float v) { pitch = v; update(); }
            void setRoll     (float v) { roll  = v; update(); }

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
            void tiltLeft (float speed) {
                roll -= speed;
                update();
            }
            void tiltRight (float speed) {
                roll += speed;
                update();
            }
            void update () {
                // clamp in radians — 89° expressed as radians
                constexpr float limit = 89.0f * (float)(M_PI / 180.0);
                if (pitch >  limit) pitch =  limit;
                if (pitch < -limit) pitch = -limit;
                front.x = std::cosf(yaw) * std::cosf(pitch);
                front.y = std::sinf(pitch);
                front.z = std::sinf(yaw) * std::cosf(pitch);
                front = front.normalize();
                vec3 right = front.crossProduct(world_up).normalize();
                vec3 up_base  = right.crossProduct(front).normalize();
                up = up_base * std::cosf(roll) + right * std::sinf(roll);
            }
    };
}