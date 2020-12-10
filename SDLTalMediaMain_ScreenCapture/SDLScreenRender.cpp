#include <SDLScreenRender.h>
#include <DuplicatorCapture.h>

SDLScreenRender::SDLScreenRender(DuplicatorCapture* sc) : SDLRenderWindow("ScreenCapture"),
	m_screenCapture(sc)
{
    
}

SDLScreenRender::~SDLScreenRender()
{
    if (m_currentTexture)
    {
        SDL_DestroyTexture(m_currentTexture);
        m_currentTexture = NULL;
    }

    if (m_textTexture) 
    {
        SDL_DestroyTexture(m_textTexture);
        m_textTexture = NULL;
    }

    if (m_currentTextureI420)
    {
        SDL_DestroyTexture(m_currentTextureI420);
        m_currentTextureI420 = NULL;
    }

    if (m_dataY) {
        SDL_free(m_dataY);
        m_dataY = NULL;
    }

    if (m_dataU) {
        SDL_free(m_dataU);
        m_dataU = NULL;
    }
    if (m_dataV) {
        SDL_free(m_dataV);
        m_dataV = NULL;
    }
}

void SDLScreenRender::SetRenderParam(int width, int height)
{
    m_width = width;
    m_height = height;

    if (m_currentTexture) {
        SDL_DestroyTexture(m_currentTexture);
        m_currentTexture = NULL;
    }
    if (m_currentTextureI420) {
        SDL_DestroyTexture(m_currentTextureI420);
        m_currentTextureI420 = NULL;
    }

    m_currentTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING,
        m_width, m_height);

    m_currentTextureI420 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING,
        m_width, m_height);

    return;
}


void SDLScreenRender::DrawRGBA()
{
    if (m_screenCapture) {
        int width = 0;
        int height = 0;
        int stride = 0;
        uint8_t* ptr = NULL;

        if (m_screenCapture->CaptureOneFrame()) {

            m_screenCapture->LoadCaptureImageRGBA((void**)&ptr, &stride, &width, &height);
            if (m_currentTexture && ptr) {
                SDL_UpdateTexture(m_currentTexture, NULL, ptr, stride);

            }
        }
        SDL_RenderCopy(renderer, m_currentTexture, NULL, NULL);

    }
    return;
}

void SDLScreenRender::DrawI420()
{
    if (m_screenCapture) {
        int width = 0;
        int height = 0;
        int strideY = 0;
        int strideU = 0;
        int strideV = 0;
        uint8_t* ptrY = NULL;
        uint8_t* ptrU = NULL;
        uint8_t* ptrV = NULL;

        if (m_screenCapture->CaptureOneFrame()){

            m_screenCapture->LoadCaptureImageYUV((void**)&ptrY, &strideY, (void**)&ptrU, &strideU, (void**)&ptrV, &strideV, &width, &height);

            if (m_currentTexture && ptrY && ptrU && ptrV) {
                SDL_UpdateYUVTexture(m_currentTextureI420, NULL, ptrY, strideY, ptrU, strideU, ptrV, strideV);

            }
        }

        SDL_RenderCopy(renderer, m_currentTextureI420, NULL, NULL);
    }
    return;
}

void SDLScreenRender::Draw()
{
    DrawRGBA();
    return;
}
