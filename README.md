<h1>Graphics engine</h1>

<p>The program is an OpenGL engine based on the articles by <a href="https://twitter.com/JoeyDeVriez">Joey de Vries</a> in <a href="https://learnopengl.com/">https://learnopengl.com/</a> under the terms of the <a href="https://creativecommons.org/licenses/by/4.0/">CC BY-NC 4.0</a>.</p>
<p>It is used for my personal experimentations on computer graphics, shaders and rendering.</p>

<img width="1973" height="1387" alt="image" src="https://github.com/user-attachments/assets/b95bdfb5-4795-4956-8aff-a6208cdd4803" />

## Features

- Data-driven scenes from JSON files
- Scene cycling via `assets/scenes/scene_config.json`
- Object behaviors (None, Oscillate, Spin, Fly)
- Scripted camera routes with keyframes (position + lookAt)
- Material/shader selection per object
- Runtime asset sync to `build/assets`

## Dependencies

- CMake (3.10+)
- C++17 compiler
- OpenGL
- GLFW3
- GLM
- FreeType
- nlohmann/json

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Run

```bash
./build/OpenGLEngine
```

## Controls

- `W/A/S/D`: move camera on horizontal plane
- `Q/E`: move camera down/up
- Mouse: look around
- Mouse wheel: zoom
- `C`: toggle scripted/manual camera mode (when keyframes exist)
- `ESC`: quit

## Scene Configuration

- Scene registry and cycle: `assets/scenes/scene_config.json`
- Scene files: `assets/scenes/basic.scene.json`, `assets/scenes/alternate.scene.json`
- Shaders: `assets/shaders/`
- Meshes: `assets/meshes/`

## Asset Loading Notes

- The executable loads scene JSON, shader files, and mesh files at runtime.
- Build copies assets into `build/assets` automatically (`copy_assets` target).
- Because runtime file IO is used, the executable currently expects assets to be present next to the build output.

## Notes

- Ground and other object roles are semantic scene tags.
- Render behavior is controlled by material `renderMode`.
