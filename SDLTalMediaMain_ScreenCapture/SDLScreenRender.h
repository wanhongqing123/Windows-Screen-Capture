#pragma once

#include <SDLRenderWindow.h>


class DuplicatorCapture;


class SDLScreenRender : public SDLRenderWindow
{
public:
	SDLScreenRender(DuplicatorCapture* sc);
	~SDLScreenRender();
	void SetRenderParam(int width, int height);
	void Draw() override;
	void DrawRGBA();
	void DrawI420();
private:
	SDL_Texture* m_currentTexture = NULL;
	SDL_Texture* m_currentTextureI420 = NULL;
	SDL_Texture* m_textTexture = NULL;
	DuplicatorCapture* m_screenCapture = NULL;

	int m_width;
	int m_height;

	Uint8* m_dataY = NULL;
	Uint8* m_dataU = NULL;
	Uint8* m_dataV = NULL;

};
