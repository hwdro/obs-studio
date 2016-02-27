project(line-aa)

if(MSVC)
	set(line-aa_PLATFORM_DEPS
		w32-pthreads)
endif()

find_package(FFmpeg REQUIRED
	COMPONENTS avcodec)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(line-aa_SOURCES
	line-aa.c)
set(line-aa_HEADERS
	line-aa.h
	slidingbuf.h
	audio-window.h)

add_library(line-aa MODULE
	${line-aa_HEADERS}
	${line-aa_SOURCES})
target_link_libraries(line-aa
	libobs
	${line-aa_PLATFORM_DEPS}
	${FFMPEG_LIBRARIES})

install_obs_plugin_with_data(line-aa data)
