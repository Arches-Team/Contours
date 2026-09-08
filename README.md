# AppIso

## Requirements

The application is currently supported on **Windows x64**.

You need:

* **Visual Studio 2022** with the **Desktop development with C++** workload.
* **Qt 6 for MSVC 2022 64-bit**.
* The following Qt modules:

### Qt libraries

Debug:

```text
Qt6Cored
Qt6Guid
Qt6Widgetsd
Qt6OpenGLd
Qt6OpenGLWidgetsd
Qt6Svgd
```

Release:

```text
Qt6Core
Qt6Gui
Qt6Widgets
Qt6OpenGL
Qt6OpenGLWidgets
Qt6Svg
```

The corresponding Qt development files (`include`, `lib`, and `bin`) must be available in the Qt installation.

## Qt configuration

Create a file named:

```text
Qt.props
```

at the root of the project, next to the `.vcxproj` file.

Its contents should be:

```xml
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <QtDir>C:\Qt\6.x.x\msvc2022_64</QtDir>
  </PropertyGroup>
</Project>
```

Replace the path with the location of your Qt installation.

For example:

```xml
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <QtDir>C:\Qt\6.8.3\msvc2022_64</QtDir>
  </PropertyGroup>
</Project>
```

The `Qt.props` file is specific to each development environment and should not be committed to the repository (the file name is already in a `.gitignore`).

## Build

1. Install Visual Studio 2022 with the C++ workload.
2. Install the required Qt 6 MSVC 64-bit package.
3. Create `Qt.props` with the path to your Qt installation.
4. Open the `.sln` file in Visual Studio.
5. Select:

```text
Debug | x64
```

or:

```text
Release | x64
```

6. Build the solution with **Build → Build Solution** or `Ctrl+Shift+B`.

The project automatically performs the required Qt code generation and deployment during the build.

## Other dependencies

No additional configuration is required from the user.

The project handles the required dependencies automatically during the build.

The following libraries are used by the application:

```text
glew32
glu32
opengl32
```

`glew32.dll` is included with the project and is automatically copied to the output directory.

`glu32` and `opengl32` are Windows system libraries and are provided by Windows.

The user does not need to install, configure, or manually copy any of these dependencies.

## Output directory

After building the project, the final executable can be found in:

```text
Out\x64\Debug\
```

for a **Debug** build, or:

```text
Out\x64\Release\
```

for a **Release** build.

The main application executable is:

```text
AppIso.exe
```

For example:

```text
Out\
└── x64\
    └── Release\
        └── AppIso.exe
```

The executable can be launched directly from this directory.

## Supported configuration

```text
Operating system : Windows
Architecture     : x64
Compiler         : MSVC / Visual Studio 2022
Qt               : Qt 6, MSVC 2022, 64-bit
```
