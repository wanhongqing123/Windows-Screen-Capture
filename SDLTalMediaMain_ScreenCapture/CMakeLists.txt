project(SDLTalMediaMain_ScreenCapture)

find_package(SDL2)


if(WIN32)
	include_directories("${SDL2_INCLUDE_DIRS}")
	include_directories(.)

	set(SDLTalMedia_Sources
		main.cpp
		../RenderDevice.cpp
		../SDLRenderWindow.cpp
		../DuplicatorCapture.cpp
		SDLScreenRender.cpp
	    )
	set(SDLTalMedia_Headers
		../SDLRenderWindow.h
		../RenderDevice.h
		../stdafx.h
		../linmath.h
		../DuplicatorCapture.h
		SDLScreenRender.h
		)
	
	
	add_executable(SDLTalMediaMain_ScreenCapture  WIN32 
	${SDLTalMedia_Sources}
	${SDLTalMedia_Headers}
	)
	
   set_target_properties(SDLTalMediaMain_ScreenCapture PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(SDLTalMediaMain_ScreenCapture PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
   set_target_properties(SDLTalMediaMain_ScreenCapture PROPERTIES LINK_FLAGS "/LARGEADDRESSAWARE")
	
	target_link_libraries(SDLTalMediaMain_ScreenCapture
	PUBLIC
		${SDL2_LIBRARIES}
		${SDL2main_LIBRARIES}
		opengl32
		d3d11
		dxgi
		)
	
endif()


install(TARGETS SDLTalMediaMain_ScreenCapture  RUNTIME DESTINATION ${SDLTalMedia_BINARY_DIR}/debug CONFIGURATIONS Debug)
install(TARGETS SDLTalMediaMain_ScreenCapture  RUNTIME DESTINATION ${SDLTalMedia_BINARY_DIR}/release CONFIGURATIONS Release)
install(TARGETS SDLTalMediaMain_ScreenCapture  RUNTIME DESTINATION ${SDLTalMedia_BINARY_DIR}/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)
