project(audio-vis)

if(MSVC)
	set(audio-vis_PLATFORM_DEPS
		w32-pthreads)
endif()

find_package(FFmpeg REQUIRED
	COMPONENTS avcodec)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(audio-vis_SOURCES
	audio-visualizer.c
	vis-spectrum.c
	audiocapture.c)
set(audio-vis_HEADERS
	slidingbuf.h
	audiobuf.h
	audiocapture.h)

add_library(audio-vis MODULE
	${audio-vis_HEADERS}
	${audio-vis_SOURCES})
target_link_libraries(audio-vis
	libobs
	${audio-vis_PLATFORM_DEPS}
	${FFMPEG_LIBRARIES})

install_obs_plugin_with_data(audio-vis data)
