# NAppGUI
Cross-Platform C SDK. Build portable desktop applications for Windows, macOS and Linux, using just C.

![NAppGUI Stack](https://nappgui.com/img/start/nappgui_stack.png)

## Quick start in Windows

### Prerequisites
- [Visual Studio](https://visualstudio.microsoft.com/vs/)
- [CMake](https://cmake.org/download/)

### Open the Developer Command Prompt
```
git clone --depth 1 https://github.com/frang75/nappgui_src.git
cd nappgui_src
cmake -S . -B build
cmake --build build --config Debug

// Run examples in 'demo' and 'howto' folders
.\build\Debug\bin\Die.exe
.\build\Debug\bin\Bricks.exe
.\build\Debug\bin\Products.exe
.\build\Debug\bin\Col2dHello.exe
.\build\Debug\bin\GuiHello.exe
...
```
![Running demo project in Windows](https://nappgui.com/img/start/run_demo_windows.png)

## Quick start in macOS

### Prerequisites
- [Xcode](https://developer.apple.com/xcode/)
- [CMake](https://cmake.org/download/)

### Open the Terminal
```
git clone --depth 1 https://github.com/frang75/nappgui_src.git
cd nappgui_src
cmake -G Xcode -S . -B build
cmake --build build --config Debug

// Run examples in 'demo' and 'howto' folders
./build/Debug/bin/Die.app/Contents/MacOS/Die
./build/Debug/bin/Bricks.app/Contents/MacOS/Bricks
./build/Debug/bin/Products.app/Contents/MacOS/Products
./build/Debug/bin/Col2dHello.app/Contents/MacOS/Col2dHello
./build/Debug/bin/GuiHello.app/Contents/MacOS/GuiHello
...
```
![Running demo project in macOS](https://nappgui.com/img/start/run_demo_macos.png)

## Quick start in Linux

### Prerequisites
```
// Development tools
sudo apt-get install build-essential
sudo apt-get install git
sudo apt-get install cmake

// Development libraries
sudo apt-get install libgtk-3-dev
sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev
sudo apt-get install libcurl4-openssl-dev
```
### Open the Terminal
```
git clone --depth 1 https://github.com/frang75/nappgui_src.git
cd nappgui_src
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 4

// Run examples in 'demo' and 'howto' folders
./build/Debug/bin/Die
./build/Debug/bin/Bricks
./build/Debug/bin/Products
./build/Debug/bin/Col2dHello
./build/Debug/bin/GuiHello
...
```
![Running demo project in Linux](https://nappgui.com/img/start/run_demo_linux.png)

## Language Bindings
- [RingNAppGUI](https://github.com/ring-lang/ring/tree/master/extensions/ringnappgui)

## More info
- [NAppGUI Quick Start](https://nappgui.com/en/start/quick.html)
- [NAppGUI Hello World](https://nappgui.com/en/start/hello.html)
- [Create a new project](https://nappgui.com/en/guide/newprj.html)
- [About Portability](https://nappgui.com/en/guide/win_mac_linux.html)
