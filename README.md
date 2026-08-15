# g_engine_2d

A lightweight C++17 game-engine foundation built directly on modern OpenGL and
native platform APIs. `g_engine_2d` provides windowing, input, rendering, asset
loading, audio, and math utilities without requiring a larger engine runtime or
editor.

The project is designed as a low-level abstraction layer for games, tools, and
higher-level engine systems. Its source-oriented layout makes it possible to
embed only the systems your application needs.

> [!NOTE]
> `g_engine_2d` is under active development. APIs, renderer behavior, and build
> integration may change as the project matures.

## Features

- Native Win32 and X11 window creation
- OpenGL 4.6 core rendering context
- Cross-platform keyboard and mouse input
- Immediate and batched rendering for:
  - Triangles, quads, lines, points, circles, and Bezier curves
  - Textured and grayscale images
  - Outline and rasterized text
  - Wireframes and 3D models
  - Persistent model instances
- Render-to-texture draw passes
- Camera, view, orthographic, and transform matrices
- PNG and BMP image loading
- Image atlas creation and texture updates
- TrueType font loading
- OBJ and glTF model loading
- WAV playback and file streaming on a dedicated audio thread
- Frame limiting, delta timing, fullscreen, cursor capture, and DPI queries
- Extensible CRTP renderer API for custom shaders and vertex formats

## Platform support

| Platform | Window and input | Graphics | Audio |
| --- | --- | --- | --- |
| Windows | Win32 | OpenGL | Windows multimedia APIs |
| Linux | X11 and XKB | OpenGL/GLX | ALSA |

## Requirements

- A C++17-compatible compiler
- CMake 4.0 or newer
- An OpenGL 4.6-capable driver

### Linux dependencies

Install the X11, XKB, OpenGL, and ALSA development packages. On Debian or
Ubuntu-based distributions:

```bash
sudo apt install build-essential cmake libx11-dev libgl1-mesa-dev libasound2-dev
```

### Windows dependencies

The engine links against libraries supplied by the Windows SDK:

```text
gdi32 user32 opengl32 ole32
```

## Building

Clone the repository and generate an out-of-source build:

```bash
git clone https://github.com/HyperBitGore/g_engine_2d.git
cd g_engine_2d
cmake -B build
cmake --build build
```

The default configuration is `Debug`. To create a release build:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The current CMake project builds the main example and the test executables.
When embedding the engine in another project, add the files listed in
`ENGINE_SOURCES` to your target and expose the repository's `include`
directory. A dedicated installable library target is not yet provided.

## Minimal example

```cpp
#include <g_engine/g_engine_2d.hpp>
#include <memory>

int main() {
    constexpr uint32_t width = 800;
    constexpr uint32_t height = 600;

    gore::g_engine_2d engine(
        "g_engine_2d",
        width,
        height,
        0,
        gore::LogType::CONSOLE
    );

    auto triangles = gore::trianglerenderer::create(width, height);
    engine.addRenderer(triangles.get(), false, false, false);

    engine.setRenderFunction([&]() {
        triangles->setColor({0.15f, 0.65f, 1.0f, 1.0f});
        triangles->drawTriangle(
            {400.0f, 100.0f},
            {250.0f, 400.0f},
            {550.0f, 400.0f}
        );
    });

    while (engine.updateWindow()) {
        engine.updateInputState();

        if (engine.getKeyReleased(g_Escape)) {
            break;
        }
    }
}
```

Renderers are created through their static factories and then registered with
the engine so viewport and view-matrix updates can be propagated when needed.
For larger batches, use each renderer's `add*` methods followed by
`drawBuffer()`.

## Render to a texture

`gore::drawpass` wraps an OpenGL framebuffer and can be used for compositing,
post-processing, editor viewports, or off-screen rendering:

```cpp
gore::drawpass scene(width, height, GL_COLOR_ATTACHMENT0);

scene.bind();
// Draw the scene.
scene.unbind();

imageRenderer->drawTexture(
    scene.getTexture(),
    {0.0f, 0.0f},
    {static_cast<float>(width), static_cast<float>(height)},
    {0.0f, 1.0f, 1.0f, -1.0f}
);
```

Resize the draw pass from the window resize callback when its dimensions should
track the window.

## Tests and examples

Each test is an independent CMake target:

```bash
cmake --build build --target test_primitives
./build/test_primitives
```

Notable targets include:

| Target | Coverage |
| --- | --- |
| `test_audio` | WAV loading, playback, and streaming |
| `test_images` | PNG/BMP loading and image rendering |
| `test_primitives` | 2D primitive renderers |
| `test_text` | Font loading and text rendering |
| `test_matrix_vector` | Matrix and vector operations |
| `test_frame_limit` | Timing and frame limiting |
| `test_image_stress` | High-volume image rendering |
| `test_rotate_cube_soft` | Software-transformed 3D cube |
| `test_instance_render` | Persistent instanced model rendering |
| `test_three_dee_texture_split` | Textured 3D batch behavior |
| `test_dual_engine` | Multiple engine instances |

Most rendering tests are interactive visual tests and should be run from the
repository root so their resource paths resolve correctly.

## Project structure

```text
include/g_engine/
├── audio/           Audio loading, playback, and streaming
├── file_loading/    Fonts, JSON, OBJ, and glTF assets
├── img_loading/     PNG, BMP, textures, and image atlases
├── rendering/       2D, text, wireframe, and 3D renderers
├── util/            Logging, shaders, vectors, matrices, and containers
├── g_engine_2d.hpp  Main engine interface
├── backend.hpp      Platform abstractions and input constants
└── gl_defines.hpp   OpenGL declarations and loaded entry points

tests/               Examples, visual tests, and math tests
```

Implementation files intentionally live beside the public headers under
`include/g_engine`. The CMake build lists them explicitly in `ENGINE_SOURCES`.

## Design overview

All public APIs use the `gore` namespace. `gore::g_engine_2d` owns the native
window, OpenGL context, input state, timing, and renderer coordination.
Rendering systems are separate objects, allowing applications to create only
the renderers they need.

Concrete renderers use a CRTP base:

```cpp
gore::renderer<DerivedRenderer, VertexType>
```

Custom renderers provide their shader setup and vertex layout while reusing the
engine's buffer, shader, and batching infrastructure.

## Contributing

Bug reports and focused pull requests are welcome. Before submitting a change:

1. Build the affected target.
2. Run the smallest relevant test executable.
3. Keep platform-specific code behind `_WIN32` or `__unix__` guards.
4. Preserve C++17 compatibility.

## License

This project is available under the [MIT License](LICENSE).
