#------------------------------------------
# NAppGUI CMake build script
#------------------------------------------
cmake_minimum_required(VERSION 2.8.12)
project(NAppGUI)

# NAppGUI Build Scripts
get_filename_component(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR} PATH)
include(${ROOT_PATH}/prj/CMakeNAppGUI.cmake)

# Static libraries
staticLib("sewer" "sewer" "" NRC_NONE)
staticLib("osbs" "osbs" "sewer" NRC_NONE)
staticLib("core" "core" "osbs" NRC_NONE)
staticLib("geom2d" "geom2d" "core" NRC_NONE)
staticLib("draw2d" "draw2d" "geom2d" NRC_NONE)
staticLib("osgui" "osgui" "draw2d" NRC_NONE)
staticLib("gui" "gui" "draw2d" NRC_EMBEDDED)
staticLib("inet" "inet" "core" NRC_NONE)
staticLib("osapp" "osapp" "osgui;gui" NRC_NONE)
staticLib("casino" "demo/casino" "draw2d" NRC_EMBEDDED)

# Executables
commandApp("nrc" "utils/nrc" "core" NRC_NONE)
desktopApp("Bode" "demo/bode" "osapp" NRC_EMBEDDED)
desktopApp("Bricks" "demo/bricks" "osapp" NRC_EMBEDDED)
desktopApp("Die" "demo/die" "osapp;casino" NRC_EMBEDDED)
desktopApp("Dice" "demo/dice" "osapp;casino" NRC_EMBEDDED)
desktopApp("Fractals" "demo/fractals" "osapp" NRC_EMBEDDED)
desktopApp("HelloWorld" "demo/hello" "osapp" NRC_EMBEDDED)
desktopApp("HelloCpp" "demo/hellocpp" "osapp" NRC_EMBEDDED)
desktopApp("Products" "demo/products" "osapp;inet" NRC_PACKED)
desktopApp("Col2dHello" "howto/col2dhello" "osapp" NRC_EMBEDDED)
desktopApp("DrawBig" "howto/drawbig" "osapp" NRC_EMBEDDED)
desktopApp("DrawHello" "howto/drawhello" "osapp" NRC_EMBEDDED)
desktopApp("DrawImg" "howto/drawimg" "osapp" NRC_EMBEDDED)
desktopApp("GuiHello" "howto/guihello" "osapp" NRC_EMBEDDED)
desktopApp("UrlImg" "howto/urlimg" "osapp;inet" NRC_EMBEDDED)

# Your projects here!

generateSolution()
