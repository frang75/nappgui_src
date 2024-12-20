nap_desktop_app(Die "osapp;casino" NRC_EMBEDDED)
target_include_directories(Die PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
set_target_properties(Die PROPERTIES FOLDER "demo")