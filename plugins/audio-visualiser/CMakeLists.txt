project(audio-visualiser)

if(MSVC)
	set(audio-visualiser_PLATFORM_DEPS
		w32-pthreads)
endif()

find_package(FFMpeg REQUIRED
	COMPONENTS avcodec avfilter avdevice avutil swscale avformat swresample)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(audio-visualiser_SOURCES
	plugin-main.cpp
	audiodata.cpp
	audiosource.cpp
	visualiser.cpp
	)
set(audio-visualiser_HEADERS
	visualiser.hpp
	audiodata.hpp
	audiosource.hpp)

add_library(audio-visualiser MODULE
	${audio-visualiser_HEADERS}
	${audio-visualiser_SOURCES})
target_link_libraries(audio-visualiser
	libobs
	${audio-visualiser_PLATFORM_DEPS}
	${FFMPEG_LIBRARIES})

install_obs_plugin_with_data(audio-visualiser data)
