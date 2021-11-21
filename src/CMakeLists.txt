#------------------------------------------
# NAppGUI CMake build script
#------------------------------------------
cmake_minimum_required(VERSION 2.8.12)
project(NAppGUI)

# NAppGUI Build Scripts
get_filename_component(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR} PATH)
include(${ROOT_PATH}/prj/CMakeNAppGUI.txt)

# Libraries
staticLib("sewer" "" NRC_NONE)
staticLib("osbs" "sewer" NRC_NONE)
staticLib("core" "osbs" NRC_NONE)
staticLib("geom2d" "core" NRC_NONE)
staticLib("draw2d" "geom2d" NRC_NONE)
staticLib("osgui" "draw2d" NRC_NONE)
staticLib("gui" "draw2d" NRC_EMBEDDED)
staticLib("osapp" "osgui;gui" NRC_NONE)
staticLib("inet" "core" NRC_NONE)
staticLib("utils/nlib" "core" NRC_NONE)
staticLib("demo/casino" "draw2d" NRC_EMBEDDED)

# Exes
commandApp("utils/nrc" "utils/nlib" NRC_NONE)
desktopApp("HelloWorld" "demo/hello" "" NRC_NONE)
desktopApp("HelloCpp" "demo/hellocpp" "" NRC_NONE)
desktopApp("Die" "demo/die" "demo/casino" NRC_EMBEDDED)
desktopApp("Dice" "demo/dice" "demo/casino" NRC_EMBEDDED)
desktopApp("Bricks" "demo/bricks" "" NRC_EMBEDDED)
desktopApp("Fractals" "demo/fractals" "" NRC_EMBEDDED)
desktopApp("Bode" "demo/bode" "" NRC_PACKED)
desktopApp("Products" "demo/products" "inet" NRC_PACKED)
desktopApp("GuiHello" "howto/guihello" "" NRC_EMBEDDED)
desktopApp("DrawHello" "howto/drawhello" "" NRC_EMBEDDED)
desktopApp("Col2dHello" "howto/col2dhello" "" NRC_EMBEDDED)
desktopApp("DrawImg" "howto/drawimg" "" NRC_EMBEDDED)
desktopApp("UrlImg" "howto/urlimg" "inet" NRC_EMBEDDED)


# Your projects here!

