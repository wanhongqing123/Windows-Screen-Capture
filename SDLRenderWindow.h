#pragma once

#include <SDL.h>

enum {
	USER_UPDATE_TEXTURE = 0x8001,   //update  code 1:rtc 2 live 3 player 4 rtcPlayer
	USER_PULL
};



class SDLRenderWindow 
{
public:
	SDLRenderWindow(const char* name);
	virtual ~SDLRenderWindow();

	void DrawUIWindow();

	virtual void Draw( ) {};
	virtual void DrawCustom() {};

	void Run();

	void SDL_EnterContext()
	{
		SDL_LockMutex(context);
	}

	void SDL_LeaveContext() {
		SDL_UnlockMutex(context);
	}
protected:
	
	SDL_Window*   window;
	SDL_Renderer* renderer;
	SDL_mutex*    context;
	SDL_Texture*  fontTexture;
	SDL_Texture*  backgroundTexture;
	bool          flag;
	int delay;
};