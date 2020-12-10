#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>  
#include <string>
#include <vector>
#include <SDL.h>

#define SCREEEN_MAX_NUM 2
class RenderDevice;

//typedef 
typedef struct TalMonitorInfo
{
    RenderDevice* renderDevice;
    ID3D11Device* d3d11Device;
    ID3D11DeviceContext* d3d11Context;
    IDXGIOutputDuplication* duplication;
    uint32_t                nIndex;     // 屏幕的索引 0：主显示器 1： 辅助显示器
    SDL_Rect                screenRect; // 屏幕的大小
    SDL_Rect                captureRect;
    bool                    bCapture;
    bool                    bUpdateImageData;
    TalMonitorInfo() {
        memset(this, 0, sizeof(this));
    }
    ~TalMonitorInfo() {

    }
}MonitorInfo;



typedef bool (*ScreenCaptureCallback)(void* data, int width, int height, int format, void* textureId);

class DuplicatorCapture
{
public:
    DuplicatorCapture(void* hwnd,bool  renderToWindow = false);
    ~DuplicatorCapture();

    bool Initialize();
    void SetCaptureRect(SDL_Rect rect);
    void SetCaptureOutParam(int width,int height);
    bool CaptureOneFrame();
    void UnInitialize();

    void LoadCaptureImageRGBA(void** ptr,int *stride,int*width,int *height);
    void LoadCaptureImageRGBA(void* ptr, int linesize);
    void UnloadCaptureImageRGBA();
    void LoadCaptureImageYUV(void** ptrY, int* strideY, void** ptrU,int* strideU,void** ptrV,int *strideV,int* width, int* height);

    void RenderToWindow();
    
private:
    bool CaptureOneFrame( MonitorInfo& monitor, int index);

private:
    SDL_Rect m_captureRect;
    std::vector<TalMonitorInfo> monitors_;
    bool m_bSameDevice = true;
    int m_combineDir = 0;
    int m_curMonitorIdx = 0;
    bool m_bCombine = false;
    int m_renderWidth = -1;
    int m_renderHeight = -1;
    
    void* m_hwnd = NULL;
    bool m_renderToWindow = false;
};











