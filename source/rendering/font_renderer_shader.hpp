
const char* vertexShaderSourceFont = "#version 450 core\n"
        "\n"
        "layout(location = 0) in vec2 pos;\n"
        "uniform mat4 projection;\n"
        "void main(){\n"
        "    gl_Position = projection * vec4(pos.x, pos.y, 0.0, 1.0);\n"
        "}\n"
        "";
const char* fragmentShaderSourceFont = "#version 450 core\n"
        "out vec4 color;\n"
        "uniform vec4 set_color;\n"
        "void main(){\n"
        "    color = set_color;\n"
        "}";