#include <SDL.h>
#include <SDL_image.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <DuplicatorCapture.h>
#include <SDLScreenRender.h>


int main(int argc, char** argv){
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1) {
		SDL_Log("SDL Init Success \n");
		return 0;
	}

	
	SDL_Rect rect;
	int num = SDL_GetNumVideoDisplays();
	for (int i = 0; i < 1; ++i) {
		SDL_GetDisplayBounds(0,&rect);
	}
	DuplicatorCapture* pCapture = new DuplicatorCapture(NULL,false);
	bool bInit = pCapture->Initialize();
	if (!bInit) {
		printf("DuplicatorCapture Init Failed\n ");
		SDL_Delay(3000);
		return -1;
	}

	pCapture->SetCaptureOutParam(rect.w,rect.h);
	SDLScreenRender render(pCapture);
	render.SetRenderParam(rect.w, rect.h);
	rect.x = 0;
	rect.y = 0;
	rect.w = rect.w + 500;
	rect.h = rect.h;

	pCapture->SetCaptureRect(rect);


	render.Run();
	
    return(0);
}




