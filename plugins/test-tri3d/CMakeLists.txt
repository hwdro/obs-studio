project(test-tri3d)

if(MSVC)
	set(audio-vis_PLATFORM_DEPS
		w32-pthreads)
endif()

find_package(FFmpeg REQUIRED
	COMPONENTS avcodec)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(test-tri3d_SOURCES
	plugin-main.cpp
	vertex-buffer.cpp
	index-buffer.cpp
	effect.cpp
	model.cpp
	object.cpp
	transform.cpp
	camera.cpp
	render-target.cpp
	texture-renderer.cpp
	source.cpp)
set(test-tri3d_HEADERS
	vertex-buffer.hpp
	index-buffer.hpp
	effect.hpp
	model.hpp
	transform.hpp
	camera.hpp
	render-target.hpp
	texture-renderer.hpp
	object.hpp)

add_library(test-tri3d MODULE
	${test-tri3d_HEADERS}
	${test-tri3d_SOURCES})
target_link_libraries(test-tri3d
	libobs
	${test-tri3d_PLATFORM_DEPS}
	${FFMPEG_LIBRARIES})

install_obs_plugin_with_data(test-tri3d data)
