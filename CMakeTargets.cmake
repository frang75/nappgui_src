# NAppGUI CMake targets

set(ALL_TARGETS "")
set(ALL_TARGETS ${ALL_TARGETS};src/sewer;src/osbs;src/core;src/geom2d;src/draw2d;src/osgui;src/gui;src/osapp;src/encode;src/inet;src/ogl3d;tools/nrc)

if (NAPPGUI_DEMO)
    set(ALL_TARGETS ${ALL_TARGETS};demo/bode;demo/bricks;demo/casino;demo/col2dhello;demo/colorview;demo/dice;demo/die;demo/drawbig;demo/drawhello;demo/drawimg;demo/fractals;demo/guihello;demo/hello;demo/hellocpp;demo/htjson;demo/products;demo/stlcmp;demo/urlimg;demo/webhello;demo/glhello)
endif()