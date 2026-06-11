#pragma once
#include "camera.hpp"
#include "../img_loading/image_loader.hpp"

namespace gore {

    enum class billboard_type { FIXED_SIZE, DISTANCE_ADJUST_SIZE };

    // 2D quad in 3D space which always turns towards camera
    class billboard {
        public:
            gore::vec2 dimensions;
            // middle position
            gore::vec3 pos;
            gore::IMG img;
            billboard_type type;
            billboard (gore::vec3 pos, gore::vec2 dimensions, IMG img, billboard_type type) {
                this->pos = pos;
                this->dimensions = dimensions;
                this->img = std::move(img);
                this->type = type;
            }
            // copy
            billboard (const billboard& c) {
                this->pos = c.pos;
                this->img = imageloader::copyIMG(c.img);
                this->type = c.type;
            }
            //move
            billboard (billboard&& m) noexcept {
                this->pos = std::move(m.pos);
                this->img = std::move(m.img);
                this->type = std::move(m.type);
            }
            //assignment copy
            billboard& operator=(const billboard& c) {
                if (this != &c) {
                    this->pos = c.pos;
                    this->img = imageloader::copyIMG(c.img);
                    this->type = c.type;
                }
                return *this;
            }
            //asignment move
            billboard& operator=(billboard&& m) noexcept {
                if (this != &m) {
                    this->pos = std::move(m.pos);
                    this->img = std::move(m.img);
                    this->type = std::move(m.type);
                }
                return *this;
            }

            billboard_type getType () {
                return type;
            }
            // generate vertexs on fly since it's like two triangles
            std::vector<vec3> getVertexs (camera cam) {
                std::vector<vec3> vertexs;

                // camera's right/up axes define the billboard plane
                vec3 right = cam.getFront().crossProduct(cam.getUp()).normalize();
                vec3 up    = cam.getUp();

                float half_w = dimensions.x * 0.5f;
                float half_h = dimensions.y * 0.5f;

                if (type == billboard_type::FIXED_SIZE) {
                    // scale by distance so apparent screen size stays constant
                    float dist = (cam.getPos() - pos).length();
                    half_w *= dist;
                    half_h *= dist;
                }
                // else: DISTANCE_ADJUST_SIZE uses world-space dimensions directly

                vec3 r = right * half_w;
                vec3 u = up    * half_h;

                // quad corners
                vec3 tl = pos - r + u;
                vec3 tr = pos + r + u;
                vec3 bl = pos - r - u;
                vec3 br = pos + r - u;

                // triangle 1: tl, bl, br
                vertexs.push_back(tl);
                vertexs.push_back(bl);
                vertexs.push_back(br);
                // triangle 2: tl, br, tr
                vertexs.push_back(tl);
                vertexs.push_back(br);
                vertexs.push_back(tr);

                return vertexs;
            }
    };
}