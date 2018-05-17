# R-Engine : Raw-Engine - 3D Framework.
a never ending 3d engine

Rui Varela - rui.filipe.varela@gmail.com

I may have used code that was not mine... sorry


#Engine Progress
~~~
[xx.xx.xx] -- Image Processing System
[xx.xx.xx] -- Post Processing Units
[xx.xx.xx] -- Spline Lib and NURBS
[xx.xx.xx] -- Font image filtering.
[xx.xx.xx] -- DrawStates Comparison
[xx.xx.xx] -- RenderEngine Per Window
[xx.xx.xx] -- replace stb_imabe by [SOIL] Simple OpenGL Image Library
[xx.xx.xx] -- Refactor File
[xx.xx.xx] -- VAO support for performance (maybe mantain a map of association with program)
[xx.xx.xx] -- "centroid in", "noperspective in", "flat in", "noperspective centroid in", "smooth in", "smooth centroid in" varying support
[xx.xx.xx] -- Block support for Uniforms (GLSL)
[xx.xx.xx] -- Block support for Inputs (GLSL)
[xx.xx.xx] -- rengineBump
[xx.xx.xx] -- log (compute log current size at runtime, for long runs a new file will not be opened just at startup)
[xx.xx.xx] -- Add kerning support to true type fonts
[xx.xx.xx] -- Port VR Pinscreen
[xx.xx.xx] -- Finish Atlas Generator

[16.05.16] -- VAO integration. support for Opengl 3.2 Core
[06.10.11] -- Integrated Smokin::Performance analyzer
[06.10.11] -- Added several rectangle bin packing algorithms
[06.10.11] -- Inital (non working) implementation for Altlas Generator
[23.09.11] -- Backward compatability with OpenGL 2.1
           -- GLSL is rewritten according to this gl version
	   -- Window creation fallback for non supported extensions
[22.09.11] -- Logs now use the same file until a file size threshold is reached
[20.07.11] -- Added Support for True Type Fonts using stb_truetype
[22.02.11] -- Probably fixed AL compiler error on mac
[22.02.11] -- megadrive emulator integration. Sonic is correctly rendered!
[02.02.11] -- Added C memory functions and leak detection support (rg_malloc, rg_free, rg_realloc)
[27.12.10] -- Added Timestamp string formatting method
[26.12.10] -- Added stringPrinter (type safe sprintf, supports %1%, %2% generic argument usage)
[15.11.10] -- Implemented Timestamp representation
[22.08.10] -- Added Atomic synchronization method on linux and windows
           -- Added ReadWriteMutexs synchronization method on linux and windows
[20.08.10] -- Shared pointers are now thread safe. Mutex based thread safe implementation.
[08.08.10] -- Added stack trace symbol resolver on linux. No need for external script. Needed binutils-dev
[02.08.10] -- DynamicCheckUp now works with Visual Studio 2010, no need for external script to translate address to name
           -- Added support for backtrace on VS2010
[22.06.10] -- Updated SourceCodeLocation to include function location and pretty function
[08.04.10] -- Integrated dynamic checkup (linux), the system now detects memory leaks and other sort of c++ dynamic memory problems
[03.04.10] -- CMake lists update to support install option
           -- Thread, Mutex, Condition, Block and Barrier support on windows.
[01.04.10] -- Thread, Mutex, Condition, Block and Barrier development. Threading support on linux using Pthreads.
[23.03.10] -- Rubikal - Developed a method to compute correct order and orientation for a set of faces, cube mirroring finished :) !
[15.03.10] -- Rubikal - Developed a method to compute correct orientation for faces of an ordered set of faces.
[11.03.10] -- Developed rubikal face loading from xml
[10.03.10] -- Developed rubikal face saving to xml
[05.03.10] -- Integrated CubeDetector module. This module is only active if OpenCV is found by Cmake. This allows to detect a real cube faces using a webcam. 
           -- I need to develop an algorithm to find cube topology using these faces
[21.02.10] -- Finished VideoCaptureDShow
[16.02.10] -- Initial Development of windows grabber VideoCaptureDShow
[15.02.10] -- Finished VideoCaptureV4L
[06.02.10] -- Upgraded stb_image to stbi-1.18
           -- Synchronization point with Visual C++ Compiler 2008. Updated CMake with Visual Studio Target.
           -- Ported to OpenGL 3.2 Core Profile on Win32
[05.01.10] -- Update CMake with known good targets (Win32 MinGW Make, Eclipse CDT Win32)
[01.01.10] -- Initial development of VideoCapture, VideoCaptureV4L (Video4Linux)
[29.01.10] -- Change Build System to CMake with known good targets (Linux Make, Eclipse CDT)
[24.01.10] -- Ported to OpenGL 3.2 Core Profile on X11
[13.12.09] -- Added MultiRenderTarget Support, FrameBuffer uses this new feature.
           -- Finished in/out shader parameter binding
           -- Finished program input and outputs auto discovery
[10.12.09] -- Finished glsl semantics support
           -- Shaders upgrate to support the new engines features.
           -- Out-of-order DataChannel binding
[09.12.09] -- Engine improvements towards OpengGL 3.0
           -- GLSL 1.3 Support
           -- //!pragma semantic development
[08.12.09] -- Multisample FrameBuffer support
           -- OperationBuffer State development (glDrawBuffer and glReadBuffer)
           -- Upgraded Shader system to support GLSL 1.3
[07.12.09] -- FrameBuffer example development
           -- FrameBufferObject Support
           -- RenderBuffer Support
           -- The engine now supports render to texture
           -- Stencil DrawState implementation
[02.12.09] -- Orbit Camera implementation.
[01.12.09] -- Rubikal solving algorithm finished
[15.11.09] -- Smokin::Exception was included in the project. The engine now supports traceble/stackable exceptions.
[01.11.09] -- Windows vsync Support
           -- Rubikal further development
[07.10.09] -- Added uniforms auto-discover for effects
           -- Added //!pragma default, for uniforms default values
           -- Text effect. Text is now rendered with a effect.
           -- Console effect. Console is now rendered with a effect.
[02.10.09] -- Added lang\Types as the base engine types float = Real
[29.09.09] -- DrawResource Base class creation, Texture, VertexBuffer and Programs are now DrawResources.
           -- Engine Shutdown reorder, some gl resourcer were not being released.
           -- Shader compilation, Program linking and resource release
           -- Uniform development.
           -- //!pragma include allow C/C++ like includes
[24.09.09] -- Effect Support, FragmentShader and VertexShader on the same file
[23.09.09] -- GLSL shader support development, ShaderUnit Draw State, ShaderResourceLoader...
[22.09.09] -- Started to port the engine to OpenGl 3.[0,3]
           -- VertexBufferObject implementation, this will be the main drawing object (No more immediate mode, display list, draw array)
           -- VertexBuffer class implementation to store vertex data
           -- DisplayListDraw was replaced by StaticDraw
           -- ArraysDraw was replace by DynamicDraw
[05.08.09] -- Several improvements have been done for the past months:
           -- DrawStates improvements, TextureUnit DrawState, Inital shader support, Rubikal application, engine port to win32
[20.04.09] -- System Variable expansion implementation: "/echo $copyright" will expand copyright to variable value.
           -- to escape a variable one should repeate the expansion prefix "/echo $$copyright" will echo $copyright
[20.04.09] -- InputBinder bugfix, now supports integer keys [1, 2 ,3 ..] correctly.
[11.04.09] -- SystemScript development for basic automation. SystemScriptResourceLoader and System integration. Scripts are cached for performance.
[10.04.09] -- Finished EventBinder, exposes bindingAdd, bindingList and bindingClear system commands. 
[08.04.09] -- EventBinder class development. This feature will allow to dynamically bind inputs to system commands.
[06.04.09] -- I have been working fulltime for Logica group for 2 months, so I don't get to spend much time on my own projects.
           -- I been coding the rubikal application very slowly for the past month.
[14.03.09] -- Development of a rubik cube solving system.
[13.03.09] -- Minor corrections to quadrilateral. It still only support vertical quads correctly.
[09.03.09] -- New example application to demonstrate Shapes: rengineShapes.
		   -- Capsule shape development.
[08.03.09] -- Torus shape development.
[07.03.09] -- Cone shape development.
[06.03.09] -- Cylinder shape development.
[03.03.09] -- Sphere shape development.
[01.03.09] -- Support for basic shapes. Box shape development.
[26.02.09] -- Console now uses drawables for rendering.
           -- Added support for textured Quadrilateral.
           -- Texture Support on meshes.
[25.02.09] -- Quadrilateral drawable added.
           -- Flat Color Added, Material Added. Text refactoring.
           -- Drawables delay texture loading and resource other aquisition to the first draw call.
[24.02.09] -- Added TextureUnitState, still needs some work, but it is a start.
           -- DrawStates now support aggregated states. Example(multiple textures).
[16.02.09] -- Drawables now have a DrawState.
           -- Texture2D rewritten, loading and bind is done automatically by the render engine apply method.
[12.02.09] -- Texture Source and Headers moved to rengine/state. Texture must be rewritten to be used as a state.
[06.02.09] -- clearColor, clearDepth and clearBuffers RenderEngine methods.
           -- DrawStates Stack development.
           -- PolygonMode States.
[05.02.09] -- ColorChannelMask State Implementation.
           -- BlendFunction, BlendEquation, BlendColor, CullFace, Depth and AlphaFunc States Implementation.
           -- State Management System Improvement.
[04.02.09] -- State Management system development : DrawStates, BaseStates implementation start.
           -- RenderEngine Matrix Stack development;
           -- C/C++ Demangler for linux (gcc) using ABI.
[03.02.09] -- resourceClearCache system command added.
           -- resourceReportInfo system command added.
           -- Resource manager reporting implemented.
           -- DefaultFontsResourceLoader and WinfontResourceLoader implementation.
[02.02.09] -- ImageResourceLoader and TextureResourceLoader development. rengine now supports image an texture caching.
           -- ResourceManager now allows multiple ResourceLoaders usage.
[01.02.09] -- BaseResourceLoader and ResourceLoader finished and tested with the IntResourceLoader unit test.
[31.01.09] -- ResourceLoader development.
           -- Implementation start for the Generic ResourceManager development.
[30.01.09] -- OpaqueProperty development with UnitTests.
[29.01.09] -- TypeSafe Generic type Any implementation. Any UnitTests validation.
[24.11.08] -- math/Streams.h Development.
           -- Console Render rewrite finished.
           -- Line Renderer.
           -- Win32 Compatibility fixes (Win32 Windowing, Font, Text, File).
[20.11.08] -- Version System Variable is automatically updated on each build.
           -- HudWriter was developed for simple text rendering.
           -- CharMap was replaced by HudWriter.
           -- Removed GLU dependency.
[18.11.08] -- Development of ConsoleFont. ConsoleFont is the Default font. Can be loaded calling loadFont("Default").
           -- Font base metrics representation changed to pixel.
[17.11.08] -- Winfont dump to .C font.
           -- Development pixel metric font text rendering.
[11.11.08] -- Finished centimeter metric font text rendering.
[20.10.08] -- Matrix44 Support for Fustum, Projection, Ortho and Ortho2D
[**.10.08] -- Windows .fon font loader development.
[**.09.08] -- Started font rendering support.
[31.08.08] -- Added rengineCreateWindows to demonstrate WindowingSystem basic usage.
[30.08.08] -- Correction of minor UnitTest bugs.
           -- LexicalCast.h was removed. The new String.h has a new lexical_cast with support for default values.
           -- StringToken was changed to CStringTokenizer, in the near future this will be deprecaded.
           -- Included SharedPointer and String UnitTests.
           -- Smokin::String was included in the project.
           -- File Handling UnitTests Development.
[28.08.08] -- File handling from Smokin::libs was included in the project.
           -- Included XmlSerialization and Math UnitTests.
[27.08.08] -- UnitTest framework Development.
[26.08.08] -- XmlSerialization from Smokin::libs was included in rengine.
           -- Port to linux started (again).
[**.07.08] -- Development of the X11 Windowing System.
[**.06.08] -- Development of the Win32 Windowing System.
[**.05.08] -- Development of BoundingVolume class.
[**.05.08] -- Development of Quaternion class.
[**.05.08] -- Development of Matrix class.
[**.05.08] -- Development of Vector classes.
[**.05.08] -- Development of Math Helpers.
[13.04.08] -- Camera System updated and generalized.
[13.04.08] -- Lib3ds was replaced by ATI sdk 3ds loader.
[12.04.08] -- FreeImage was replaced by Sean Barrett public domain image loaders http://nothings.org/.
[15.03.08] -- Boost Libs (SharedPointer, XML serialization) were replaced by smokin lightweight libs.
[xx.xx.xx] -- I should have begun reporting engine status a long long time ago...
~~~

# Notes on Effect System
An effect file contains the source needed for the corresponding shaders (VertexShader, FragmentShader). The source is organized in sections.

A section starts with //!pragma section [common, vertex, fragment, varying], only one whitespace is allowed between tokens.

The following are valid:
~~~
//!pragma section common
//!pragma section vertex
//!pragma section fragment
~~~
The following are invalid:
~~~
//!pragma section  common
//!pragma  section vertex
// !pragma section fragment
~~~

The "common" section will be included at the beginning all shaders.

The "varying" section contains the variables passed from the vertex shader to the fragment shader, they will be included on both shaders with the correct prefix (out: vertex, in: fragment) 
~~~
vec3 normal;
vec3 light_vector;
vec3 eye_vector;
~~~

One can include source code from another file with pragma include. This pragma recursively includes source from shader files.
~~~
//!pragma include includes.shd
//!pragma include test/math.shd
~~~

Lines starting with uniform, will allow uniform's name autodetection. They will be available on C++ code with no further effort.
~~~
uniform vec4 color;
~~~
Uniforms present in the effect file are auto-discovered and are added to the Program object. Auto-Discover currently supports:
~~~
float, vec2, vec3, vec4
int ivec2, ivec3, ivec4
mat4, mat4x4
arrays
multi-variable declaration on the same statement: int x, y, z;
~~~
Structs are not supported!

One can set the initial/default value for a uniform using //!pragma default [name] [values]
~~~
//!pragma default color 1.0, 1.0, 1.0, 1.0
~~~

VertexShader inputs are autodiscovered and registered on the program object. By default every input is binded to VertexBufferObject data channel using the order of declaration, The engine allows semantic binding, to allow out of order channel binding, the //!pragma semantic directive sets the input semantic.

~~~
//!pragma semantic input_color color
~~~
At runtime the engine will bind the vertex shader input "input_color" to a VertexBufferObject data channel with the same semantic (Color)

# Error Codes
- 001XX Image
- 002XX CharMap
- 003XX Console
- 004XX System
- 005XX Mesh
- 006XX Text
- 1XXXX Application Range

# Third Party Source Code
- OpenGL Extension Wrangler Library (GLEW) - http://glew.sourceforge.net/ - BSD License
- 3ds - ATI SDK - No License
- stb_image.c - http://nothings.org/ - Public Domain
- tinyxml - http://www.grinninglizard.com/tinyxml/ - Permissive License

# Preprocessor definitions
- #define _CRT_SECURE_NO_DEPRECATE //windows
- #define GLEW_STATIC
- #define XRANDR_EXTENSIONS // to enable X11 resolution change (if defined you must link with Xrandr)

# Notes
- 3ds is not thread safe!
- Copywrite symbol on each file :  // __!!rengine_copyright!!__ //

# Eclipse IDE
- Aditional Include Dirs	"${workspace_loc:/librengine/include}"
- Aditional Link Dirs		"${workspace_loc:/librengine/[Debug,Release].[Win32,Linux]}"
- Link Libs					rengine GL X11 Xrandr

# CoreDumps on linux
```shell
ulimit -c unlimited
```

# valgrind
Generate supression :
```shell
valgrind --gen-suppressions=all --log-file=dump.txt --leak-check=full --error-limit=no -v glxgears
cat dump.txt | valgrind_supressor > opengl.supp~
```

run with supressions
```shell
valgrind --gen-suppressions=all --log-file=dump.txt --leak-check=full --suppressions=opengl.supp --error-limit=no -v glxgears
```


