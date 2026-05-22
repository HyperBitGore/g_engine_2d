#include "../util/shader.hpp"
namespace gore {
    template <class T>
    class renderer {
        protected:
            std::vector<T> vertexs;
            shader shader;
            GLuint vao;
            GLuint vertex_buffer;
            GLuint allocated;
            uint32_t width, height;
            int32_t texture_units;
            GLuint draw_arrays_mode = GL_TRIANGLES;
            // pure virtual function which children use to setup vertex attribs / other shader setup like uniforms
            virtual void shader_setup() = 0;
        public:
            renderer (std::string vertex_shader, std::string fragment_shader, uint32_t width, uint32_t height);
            virtual void drawBuffer ();
            // sets the ortho matrix in shader
            virtual void setDimensions (uint32_t width, int32_t height);
            // sets the view matrix in shader
	        virtual void updateView (float x, float y, float zoom);
    };
}