# imgui_impl_rgfw.h 
RGFW platform backend for Dear ImGUI

# Example
![alt text](image.png)

The example uses RGFW with ImGui and OpenGL3.
To compile the example, `cd` into the example directory:
```
cd example
```

### Build on Linux, macOS, and Windows (MinGW/MSYS2)
Build with `make` .
```sh
make
```
To build with docking
```sh
make docking=1
```

### Build on Windows (MSVC/Clang)
Ensure you have [Visual Studio](https://visualstudio.microsoft.com/vs/) installed, then run `build.bat`.
```cmd
build
```
To build with docking
```cmd
build docking
```

This compiles RGFW, ImGUI and then main.cpp
