# AppIso

## Requirements

* Windows or Linux, x64.
* A C++ compiler:
  * Windows: MSVC 2022. Get it from [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (any edition, including the free Community one) or the standalone [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) — either way, install the **Desktop development with C++** workload. The build script only uses the compiler from the command line; you never need to open the IDE.
  * Linux: a C++ toolchain, e.g. `sudo apt install build-essential`.
* GLEW:
  * Windows: already included in the repository, nothing to install.
  * Linux: `sudo apt install libglew-dev` (Debian/Ubuntu), `sudo dnf install glew-devel` (Fedora), `sudo pacman -S glew` (Arch), or the equivalent for your distribution.
* **Qt 6**, with these modules: `Core`, `Gui`, `Widgets`, `OpenGL`, `OpenGLWidgets`, `Svg`.
  * Windows: install it with the [Qt Online Installer](https://www.qt.io/download-qt-installer) (requires a free Qt account), choosing the **MSVC 2022 64-bit** kit. By default the build script looks for it under `C:\Qt\`.
  * Linux: install it via your package manager (package names vary by distribution, e.g. on Debian/Ubuntu: `sudo apt install qt6-base-dev qt6-base-dev-tools qt6-svg-dev`), or with the Qt Online Installer under `~/Qt/`.
  * If Qt is installed somewhere else, set the `QT_DIR` environment variable to your Qt kit folder before running the build script (e.g. `C:\Qt\6.8.3\msvc2022_64` or `~/Qt/6.8.3/gcc_64`).

## Build

* **Windows:** double-click [`AppIso/AppIso/build.bat`](AppIso/AppIso/build.bat), or run it from a terminal.
* **Linux:** run `bash AppIso/AppIso/build.sh`.

Once it finishes, the executable is at:

* Windows: `AppIso/AppIso/build/AppIso.exe`
* Linux: `AppIso/AppIso/build/AppIso`

## Running the application

Run the executable with `AppIso/AppIso` as the working directory (some shaders and resources are loaded relative to it). If you need to launch it from elsewhere, set the `ARCHESLIBDIR` environment variable to the absolute path of `AppIso/AppIso` first.

## Alternative: build with CMake directly

Use this if you already have Qt 6 installed and want to manage it yourself, instead of using `build.bat`/`build.sh`.

Required Qt modules: `Core`, `Gui`, `Widgets`, `OpenGL`, `OpenGLWidgets`, `Svg`.

### Linux

```bash
sudo apt install libglew-dev   # or the equivalent for your distribution
cd AppIso/AppIso
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

`CMAKE_PREFIX_PATH` must point at the Qt "kit" directory that contains `bin/`, `lib/`, `include/` (e.g. `~/Qt/6.8.3/gcc_64`).

### Windows

Open the **"x64 Native Tools Command Prompt for VS 2022"** (search for it in the Start menu — a regular terminal will not have the MSVC compiler set up), then run:

```bat
cd AppIso\AppIso
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2022_64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

`-G Ninja` requires [Ninja](https://ninja-build.org/); drop it to use `NMake Makefiles` instead, or use `-G "Visual Studio 17 2022"` to generate a `.sln` you can open and build in the IDE.

## Alternative: build with the Visual Studio 2022 solution

1. Install Visual Studio 2022 with the **Desktop development with C++** workload.
2. Install Qt 6 for MSVC 2022 64-bit, with these modules: `Core`, `Gui`, `Widgets`, `OpenGL`, `OpenGLWidgets`, `Svg`.
3. Create a file named `Qt.props` next to `AppIso/AppIso/AppIso.vcxproj`:

   ```xml
   <Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
     <PropertyGroup>
       <QtDir>C:\Qt\6.x.x\msvc2022_64</QtDir>
     </PropertyGroup>
   </Project>
   ```

   Replace the path with your own Qt installation (this file is specific to your machine — do not commit it).

4. Open `AppIso.sln`, select `Debug|x64` or `Release|x64`, and build (`Ctrl+Shift+B`).

The executable is written to `AppIso/AppIso/Out/x64/Debug/AppIso.exe` or `AppIso/AppIso/Out/x64/Release/AppIso.exe`.

## Supported configuration

```text
Operating systems : Windows, Linux
Architecture      : x64
Compilers         : MSVC (Visual Studio 2022) / GCC / Clang
Qt                : Qt 6, 64-bit
```
