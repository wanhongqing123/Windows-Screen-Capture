# Once done these will be defined:


find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(_SDL QUIET SDL2)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
else()
	set(_lib_suffix 32)
endif()

find_path(SDL2_INCLUDE_DIR
	NAMES SDL.h
	HINTS
		ENV SDL2Path${_lib_suffix}
		ENV SDL2Path
		ENV SDL2_DIR
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${SDL2Path${_lib_suffix}}
		${SDL2Path}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_SDL2_INCLUDE_DIRS}
	PATHS
		/usr/include /usr/local/include /opt/local/include /sw/include
	PATH_SUFFIXES
		SDL2 include/SDL2 include)

find_library(SDL2_LIB 
	NAMES ${_SDL2_LIBRARIES} SDL2 libSDL2
	HINTS
		ENV SDL2Path${_lib_suffix}
		ENV SDL2Path
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${SDL2Path${_lib_suffix}}
		${SDL2Path}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_SDL2_LIBRARY_DIRS}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		lib${_lib_suffix} lib
		libs${_lib_suffix} libs
		bin${_lib_suffix} bin
		../lib${_lib_suffix} ../lib
		../libs${_lib_suffix} ../libs
		../bin${_lib_suffix} ../bin)
		
find_library(SDL2main_LIB
    NAMES ${_SDL2main_LIBRARIES} SDL2main libSDL2main
	HINTS 
		ENV SDL2Path${_lib_suffix}
		ENV SDL2Path
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${SDL2Path${_lib_suffix}}
		${SDL2Path}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_SDL2_LIBRARY_DIRS}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		lib${_lib_suffix} lib
		libs${_lib_suffix} libs
		bin${_lib_suffix} bin
		../lib${_lib_suffix} ../lib
		../libs${_lib_suffix} ../libs
		../bin${_lib_suffix} ../bin
)

find_library(SDL2_image_LIB
    NAMES ${_SDL2_image_LIBRARIES} SDL2_image libSDL2_image
	HINTS 
		ENV SDL2Path${_lib_suffix}
		ENV SDL2Path
		ENV DepsPath${_lib_suffix}
		ENV DepsPath
		${SDL2Path${_lib_suffix}}
		${SDL2Path}
		${DepsPath${_lib_suffix}}
		${DepsPath}
		${_SDL2_LIBRARY_DIRS}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		lib${_lib_suffix} lib
		libs${_lib_suffix} libs
		bin${_lib_suffix} bin
		../lib${_lib_suffix} ../lib
		../libs${_lib_suffix} ../libs
		../bin${_lib_suffix} ../bin
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 DEFAULT_MSG SDL2_LIB SDL2_INCLUDE_DIR)
mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIB)

if(SDL2_FOUND)
	set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
	set(SDL2_LIBRARIES ${SDL2_LIB})
	set(SDL2main_LIBRARIES ${SDL2main_LIB})
	set(SDL2_image_LIBRARIES ${SDL2_image_LIB})
endif()
