#include "SDLRenderWindow.h"
#include <string>
#include <SDL_opengl.h>


SDLRenderWindow::SDLRenderWindow(const char* name): delay(10)
    , window(NULL)
    , renderer(NULL)
	, fontTexture(NULL)
	, backgroundTexture(NULL)
	, context(NULL){
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1) {
		SDL_Log("SDL Init Success \n");
		return;
	}

	context = SDL_CreateMutex();
	void* userdata = NULL;
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);//SDL_WINDOW_FULLSCREEN
	SDL_WindowFlags window_flags = (SDL_WindowFlags) (/*SDL_WINDOW_OPENGL*/  SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_ALLOW_HIGHDPI*/);
	window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, current.w/3*2, current.h/3*2, window_flags);
	if (window == NULL) {
		throw "Create Window Failed";
	}

	//SDL_GetWindowData(window,);
	int n = SDL_GetNumRenderDrivers();
	
	SDL_RendererInfo info;
	int i = 0;
	printf("***********************************************\n");
	printf("Render Engine : \n");
	for (; i < n; ++i) {//"direct3d"
		SDL_GetRenderDriverInfo(i,&info);
		printf("%s \n",info.name);
	}
	//SDL_RendererFlags SDL_RENDERER_PRESENTVSYNC SDL_RENDERER_ACCELERATED  SDL_RENDERER_SOFTWARE direct3d11
	for (i = 0; i < n; ++i) {
		SDL_GetRenderDriverInfo(i,&info);
		if (strcmp(info.name, "direct3d11") == 0)
			break;
	}
	
	if (i < n) 
		renderer = SDL_CreateRenderer(window, i, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		printf("********************************************************************************");
		printf("Failed d3d11\n");
		renderer = SDL_CreateRenderer(window,-1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	}
	if (!renderer) {
		printf( "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
		renderer = SDL_CreateRenderer(window, -1, 0);
	}
	if (renderer) {
		SDL_RendererInfo renderer_info;
		if (!SDL_GetRendererInfo(renderer, &renderer_info))
			printf("*************************************\n");
			printf("Initialized %s renderer.\n", renderer_info.name);
	}

	if (renderer == NULL){
		throw "Can not Create OpenGL render";
	}
	
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	// Audio
	int nAudioDriver = SDL_GetNumAudioDrivers();
	printf("***********************************************\n");
	printf("Audio Driver :======\n");
	for (int i = 0; i < nAudioDriver; ++i) {
		printf("%s\n", SDL_GetAudioDriver(i));
	}

	int nAudioCapture = SDL_GetNumAudioDevices(true);
	printf("***********************************************\n");
	printf("Audio Capture Names: ==== \n");
	for (int i = 0; i < nAudioCapture; ++i) {
		printf("%s\n",SDL_GetAudioDeviceName(i, true));
	}

	int nAudioRender = SDL_GetNumAudioDevices(false);
	printf("Audio Render Names: ==== \n");
	for (int i = 0; i < nAudioRender; ++i) {
		printf("%s\n", SDL_GetAudioDeviceName(i, false));
	}
	printf("***********************************************\n");

}


void  SDLRenderWindow::Run()
{
	bool done = false;
	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)){
			if (event.type == SDL_QUIT)
				done = true;
		}
		
		SDL_EnterContext();

		SDL_SetRenderDrawColor(renderer, 88, 67, 100, 255);
		SDL_RenderClear(renderer);
		if (backgroundTexture)
			SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
		if (fontTexture) {
			int w;
			int h;
			int ww;
			int hh;
			SDL_Rect rect;
			SDL_QueryTexture(fontTexture, NULL, NULL, &w, &h);
			SDL_GetWindowSize(window, &ww, &hh);
			rect.x = (ww - w) / 2;
			rect.y = (hh - h) / 2;
			rect.w = w;
			rect.h = h;
			SDL_RenderCopy(renderer, fontTexture, NULL, &rect);

		}
		Draw();
		SDL_RenderPresent(renderer);
		SDL_LeaveContext();

		SDL_Delay(30);

	}

	return;
}

void SDLRenderWindow::DrawUIWindow()
{
	DrawCustom();
}



SDLRenderWindow::~SDLRenderWindow()
{
	if (renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
	if (window) {
		SDL_DestroyWindow(window);
		window = NULL;
	}

	if (context) {
		SDL_DestroyMutex(context);
		context = NULL;
	}

	if (backgroundTexture){
		SDL_DestroyTexture(backgroundTexture);
		backgroundTexture = NULL;
	}
	if (fontTexture) {
		SDL_DestroyTexture(fontTexture);
		fontTexture = NULL;
	}
	SDL_Quit();
	int num_allocs = SDL_GetNumAllocations();
	printf("Mem Leaks is %d\n", num_allocs);
}

