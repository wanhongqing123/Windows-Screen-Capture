cmake_minimum_required(VERSION 3.0)


project(SDLTalMedia)

set(CMAKE_MODULE_PATH
	"${SDLTalMedia_SOURCE_DIR}/CMakeModule"
	"${SDLTalMedia_SOURCE_DIR}/CMakeModule/Packages"
)


if(WIN32)
    if (DepsPath OR DEFINED ENV{DepsPath} OR DEFINED ENV{DepsPath32} OR DEFINED ENV{DepsPath64})
		# Dependencies path set by user or env var
	else()
		set(DepsPath "${SDLTalMedia_SOURCE_DIR}/Dependencies/x86" CACHE PATH "Path to compiled dependencies (e.g. D:/dependencies/win64)")
		message(WARNING "DepsPath variable is missing.  Please set this variable to specify path to compiled dependencies (e.g. D:/dependencies/win64)")
	endif()
	add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
	

endif()

IF(WIN32)
    # hide insecure CRT warnings, common practice
    IF(NOT(MSVC_VERSION LESS "1600"))
    # minimum requirement for WM_TOUCH device
        ADD_DEFINITIONS(-D_WIN32_WINNT=0x0601)
        ADD_DEFINITIONS(-DWINVER=0x0601)
    ELSEIF(NOT(MSVC_VERSION LESS "1300"))
    # minimum requirement for spaceball device
        ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)
        ADD_DEFINITIONS(-DWINVER=0x0501)
    ELSE()
    # enable the use of Win2000 APIs (needed for really old compilers like MSVC6)
        ADD_DEFINITIONS(-D_WIN32_WINNT=0x0500)
        ADD_DEFINITIONS(-DWINVER=0x0500)
    ENDIF()

ENDIF()


if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wvla -Wno-unused-function -Wno-missing-field-initializers ${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wvla -Wno-unused-function -Werror-implicit-function-declaration -Wno-missing-braces -Wno-missing-field-initializers ${CMAKE_C_FLAGS} -std=gnu99 -fno-strict-aliasing")

	option(USE_LIBC++ "Use libc++ instead of libstdc++" ${APPLE})
	if(USE_LIBC++)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	endif()
elseif(MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		#string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()

	# Disable pointless constant condition warnings
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127 /wd4201 /wd4456 /wd4457 /wd4458 /wd4459 /wd4595")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4100  /wd4101 /wd4018 /wd4245 /wd4189 ")
endif()





if(WIN32)

	include_directories(.)
	add_subdirectory(SDLTalMediaMain_ScreenCapture)
	add_subdirectory(Win32TalMediaMain_ScreenCapture)
   # add_subdirectory(SDLTalMediaMain_BoostHttpServer)
	
endif()

	  file(GLOB INSTALLBINS ${DepsPath}/*.dll )
	  
	  install(FILES ${INSTALLBINS} DESTINATION ${SDLTalMedia_BINARY_DIR}/debug CONFIGURATIONS Debug)
	  install(FILES ${INSTALLBINS} DESTINATION ${SDLTalMedia_BINARY_DIR}/release CONFIGURATIONS Release)
	  install(FILES ${INSTALLBINS} DESTINATION ${SDLTalMedia_BINARY_DIR}/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)
