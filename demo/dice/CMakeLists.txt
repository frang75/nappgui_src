nap_desktop_app(Dice "osapp;casino" NRC_NONE)
target_include_directories(Dice PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
set_target_properties(Dice PROPERTIES FOLDER "demo")
