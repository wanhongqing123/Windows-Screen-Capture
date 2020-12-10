project(Win32TalMediaMain_ScreenCapture)

find_package(SDL2)

if(WIN32)
	include_directories(.)
    include_directories("${SDL2_INCLUDE_DIRS}")
	set(SDLTalMedia_Sources
		main.cpp
		../RenderDevice.cpp
		../DuplicatorCapture.cpp
	    )
	set(SDLTalMedia_Headers
		../RenderDevice.h
		../stdafx.h
		../linmath.h
		../DuplicatorCapture.h
		)
	
	
	add_executable(Win32TalMediaMain_ScreenCapture  WIN32 
	${SDLTalMedia_Sources}
	${SDLTalMedia_Headers}
	)
	
   set_target_properties(Win32TalMediaMain_ScreenCapture PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(Win32TalMediaMain_ScreenCapture PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
   set_target_properties(Win32TalMediaMain_ScreenCapture PROPERTIES LINK_FLAGS "/LARGEADDRESSAWARE")
	
	target_link_libraries(Win32TalMediaMain_ScreenCapture
	PUBLIC
	    ${SDL2_LIBRARIES}
		${SDL2main_LIBRARIES}
		d3d11
		dxgi
		Dwmapi
		)
endif()


install(TARGETS Win32TalMediaMain_ScreenCapture  RUNTIME DESTINATION ${SDLTalMedia_BINARY_DIR}/debug CONFIGURATIONS Debug)
install(TARGETS Win32TalMediaMain_ScreenCapture  RUNTIME DESTINATION ${SDLTalMedia_BINARY_DIR}/release CONFIGURATIONS Release)
install(TARGETS Win32TalMediaMain_ScreenCapture  RUNTIME DESTINATION ${SDLTalMedia_BINARY_DIR}/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)
