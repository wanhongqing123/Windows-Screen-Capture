#include <DuplicatorCapture.h>
#include <RenderDevice.h>

DuplicatorCapture::DuplicatorCapture(void* hwnd,bool bRenderToWindow):
    m_hwnd(hwnd),
    m_renderToWindow(bRenderToWindow)
{

}

DuplicatorCapture::~DuplicatorCapture()
{
    UnInitialize();
}

bool DuplicatorCapture::Initialize()
{
	HRESULT hr;
	IDXGIFactory1* factory = NULL;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1),(void**)&factory);
	if (!factory)
	{
		printf("CreateDXGIFactory Failed\n");
		return false;
	}

	std::vector<IDXGIAdapter1*> drivers;
	IDXGIAdapter1* adaptor;
	uint32_t index = 0;
	while (factory->EnumAdapters1(index++, &adaptor) == S_OK)
	{
		DXGI_ADAPTER_DESC desc;
		hr = adaptor->GetDesc(&desc);
		if (FAILED(hr))
			continue;

		if (desc.VendorId == 0x1414 && desc.DeviceId == 0x8c)
			continue;

		drivers.push_back(adaptor);
	}

	D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
	};
	UINT NumFeatureLevels = sizeof(FeatureLevels) / sizeof(FeatureLevels[0]);
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_10_0;

    for (auto dxgiAdapter : drivers) {
        UINT monitor = 0;
        IDXGIOutput* dxgiOutput;

        ID3D11Device* device = NULL;
        ID3D11DeviceContext* context = NULL;
        bool bRelease = true;
        if (FAILED(D3D11CreateDevice(dxgiAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, FeatureLevels,
            NumFeatureLevels, D3D11_SDK_VERSION, &device,
            &FeatureLevel, &context))) {
            continue;
        }

        while (dxgiAdapter->EnumOutputs(monitor++, &dxgiOutput) == S_OK) {
            DXGI_ADAPTER_DESC aDesc;
            dxgiAdapter->GetDesc(&aDesc);
            DXGI_OUTPUT_DESC desc;
            dxgiOutput->GetDesc(&desc);
            IDXGIOutput1* dxgiOutput1;
            hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1),(void**)&dxgiOutput1);

            if (FAILED(hr))
            {
                dxgiOutput->Release();
                continue;
            }
            IDXGIOutputDuplication* duplication = NULL;
            if (FAILED((hr = dxgiOutput1->DuplicateOutput(device, &duplication))))
            {
                dxgiOutput1->Release();
                dxgiOutput->Release();
                continue;
            }
         
            TalMonitorInfo info;
            info.d3d11Device = device;
            info.d3d11Context = context;
            info.screenRect.x = desc.DesktopCoordinates.left;
            info.screenRect.y = desc.DesktopCoordinates.top;
            info.screenRect.w = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
            info.screenRect.h = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
            DirectX11Rect *directx11Rect = (DirectX11Rect*)(&info.screenRect);

            info.renderDevice = new RenderDevice(directx11Rect);
            info.duplication = duplication;
            bRelease = false;
            if (SDL_IntersectRect(&m_captureRect, &info.screenRect,&info.captureRect))
            {
                info.bCapture = true;
            }
            if (monitors_.size() == 0)
                info.nIndex = 0;
            else
                info.nIndex = 1;
            if (monitors_.size() < SCREEEN_MAX_NUM) {
                monitors_.emplace_back(info);
            }

            dxgiOutput1->Release();
            dxgiOutput->Release();
        }

        if (bRelease) {
            device->Release();
            context->Release();
        }
    }

    for (auto dxgiAdapter : drivers) {
        dxgiAdapter->Release();
    }

    for (auto iter : monitors_)
    {
        printf("[DesktopDuplication]  captureScope_ x %d,y %d,w %d, h %d\n",
            m_captureRect.x, m_captureRect.y, m_captureRect.w, m_captureRect.h);
    }

    if (monitors_.size() > 1) {
        if (monitors_.at(0).d3d11Device == monitors_.at(1).d3d11Device)
            m_bSameDevice = true;
        else
            m_bSameDevice = false;

        TalMonitorInfo info = monitors_.at(1);
        if (info.screenRect.x > 0 || info.screenRect.y > 0)
        {
            if (info.screenRect.x == monitors_.at(0).screenRect.w)
                m_combineDir = CombDirection::Combine_LeftRight;

            else
                m_combineDir = CombDirection::Combine_UpDown;
        }

        if (info.screenRect.x < 0 || info.screenRect.y < 0) {
            if (info.screenRect.w  + info.screenRect.x== 0)
                m_combineDir = CombDirection::Combine_RightLeft;
            else
                m_combineDir = CombDirection::Combine_DownUp;
        }
    }

    if (monitors_.empty()) {
        printf("No Monitors \n");
        return false;
    }

    return true;
}

void DuplicatorCapture::UnInitialize()
{
    for (auto iter : monitors_) {
        delete iter.renderDevice;
        iter.d3d11Device->Release();
        iter.d3d11Context->Release();
        iter.duplication->Release();
    }

    monitors_.clear();
}

void DuplicatorCapture::SetCaptureOutParam(int width, int height)
{
    if (m_renderWidth != width || m_renderHeight != height) {
        m_renderWidth = width;
        m_renderHeight = height;
        for (auto iter : monitors_)
        {
            if (iter.renderDevice->isInit() == false)
            {
                iter.renderDevice->RenderDeviceInit(iter.d3d11Device, iter.d3d11Context);
                if (m_renderToWindow && iter.nIndex == 0) {
                    iter.renderDevice->CreateSwainChain((HWND)m_hwnd,m_renderWidth,m_renderHeight);
                }
            }
            iter.renderDevice->SetTargetParam(m_renderWidth, m_renderHeight);
        }
    }
}

void DuplicatorCapture::SetCaptureRect(SDL_Rect rect)
{
    m_captureRect = rect;

    m_curMonitorIdx = 0;
    for (unsigned int i = 0; i < monitors_.size(); ++i)
    {
        monitors_[i].bCapture = false;
        if (SDL_IntersectRect(&m_captureRect, &monitors_[i].screenRect, &monitors_[i].captureRect))
        {
            monitors_[i].bCapture = true;
            m_curMonitorIdx = i;
            monitors_[i].renderDevice->SetCaptureRect(monitors_[i].captureRect.x,
                monitors_[i].captureRect.y,
                monitors_[i].captureRect.w,
                monitors_[i].captureRect.h);
        }
    }

    m_bCombine = false;
    if (monitors_.size() > 1) {
        if (monitors_.at(0).bCapture && monitors_.at(1).bCapture)
            m_bCombine = true;
    }
}

bool DuplicatorCapture::CaptureOneFrame()
{
    bool bRet = false;
    for (uint32_t i = 0; i < monitors_.size();++i) {
        CaptureOneFrame(monitors_[i], i);
    }

    if (m_bCombine) {
        bRet = monitors_.at(0).bUpdateImageData || monitors_.at(1).bUpdateImageData;
        SDL_Rect r0 = monitors_.at(0).captureRect;
        SDL_Rect r1 = monitors_.at(1).captureRect;
        float proportion = 1;
        if (m_combineDir == Combine_LeftRight || m_combineDir == Combine_RightLeft)
        {
            proportion = (float)r0.w / (float)r1.w;
        }
        if (m_combineDir == Combine_UpDown || m_combineDir == Combine_DownUp)
        {
            proportion = (float)r0.h / (float)r1.h;
        }

        if (m_bSameDevice) {
            DirectX11Texture2D* tex1 = monitors_.at(0).renderDevice->GetCurrentTarget();//primary tex
            DirectX11Texture2D* tex2 = monitors_.at(1).renderDevice->GetCurrentTarget();//sub tex

            monitors_.at(0).renderDevice->RenderCombine(tex1, tex2, proportion, (CombDirection)m_combineDir);
        }
        else {
            DirectX11Texture2D* tex1 = monitors_.at(0).renderDevice->GetCurrentTarget();
            DirectX11Surface2D* surface = monitors_.at(1).renderDevice->GetCurrentSurface();
            monitors_.at(0).renderDevice->RenderCombine(tex1, surface, proportion, (CombDirection)m_combineDir);
        }
    }
    else{
        bRet = monitors_.at(m_curMonitorIdx).bUpdateImageData;
    }


    return bRet;
}

bool DuplicatorCapture::CaptureOneFrame( MonitorInfo& monitor, int index) {

    HRESULT hr;
    monitor.bUpdateImageData = false;
    if (monitor.bCapture) {
        if (monitor.renderDevice->isInit()&& monitor.duplication) {
            IDXGIResource*  resource;
            ID3D11Texture2D*  tex;
            DXGI_OUTDUPL_FRAME_INFO frameInfo;
            hr = monitor.duplication->AcquireNextFrame(0, &frameInfo, &resource);
            if (hr == S_OK) {
                monitor.bUpdateImageData = true;
                resource->QueryInterface(__uuidof(ID3D11Texture2D),(void**)&tex);
                if (m_bCombine) {
                    if (m_bSameDevice) {
                        monitor.renderDevice->RenderDupTexToTarget(tex, false,false);
                    }
                    else {
                        if (index == 1) {
                            monitor.renderDevice->RenderDupTexToTarget(tex, true,false);
                        }
                        if (index == 0) {
                            monitor.renderDevice->RenderDupTexToTarget(tex, false,false);
                        }
                    }
                }
                else {
                    monitor.renderDevice->RenderDupTexToTarget(tex, true,true);
                }
                monitor.duplication->ReleaseFrame();

                return true;
            }
        }
    }

    return false;
}

void DuplicatorCapture::RenderToWindow()
{
    DirectX11Texture2D* target = NULL;
    if (m_bCombine) {
        target = monitors_.at(0).renderDevice->GetCurrentTarget(true);
        monitors_.at(0).renderDevice->RenderDupTexToWindow(target,target->m_width,target->m_height);
    }

    else {
        target = monitors_.at(m_curMonitorIdx).renderDevice->GetCurrentTarget();
        monitors_.at(m_curMonitorIdx).renderDevice->RenderDupTexToWindow(target, target->m_width, target->m_height);
    }
}

void DuplicatorCapture::LoadCaptureImageYUV(void** ptrY, int* strideY, void** ptrU, int* strideU, void** ptrV, int* strideV, int* width, int* height)
{
    DirectX11Surface2D* YPlane = NULL;
    DirectX11Surface2D* UPlane = NULL;
    DirectX11Surface2D* VPlane = NULL;
    if (m_bCombine) {
        monitors_.at(0).renderDevice->GetConvertCurfaceYUV(&YPlane,&UPlane,&VPlane);
    }

    else {
        monitors_.at(m_curMonitorIdx).renderDevice->GetConvertCurfaceYUV(&YPlane,&UPlane,&VPlane);
    }

    if (YPlane && ptrY) {
        YPlane->DownLoadSurfaceData();
        *ptrY = YPlane->m_data;
        if (strideY)
            *strideY = YPlane->m_linesize;
        if (width)
            *width = YPlane->m_width;
        if (height)
            *height = YPlane->m_height;
    }

    if (UPlane && ptrU) {
        UPlane->DownLoadSurfaceData();
        *ptrU = UPlane->m_data;
        if (strideU)
            *strideU = UPlane->m_linesize;
    }

    if (VPlane && ptrV) {
        VPlane->DownLoadSurfaceData();
        *ptrV = VPlane->m_data;
        if (strideV)
            *strideV = VPlane->m_linesize;
    }

    return;
}

void DuplicatorCapture::UnloadCaptureImageRGBA()
{
    
}
//RGB
void DuplicatorCapture::LoadCaptureImageRGBA(void* ptr, int linesize) {
    DirectX11Surface2D* surface = NULL;
    if (m_bCombine) {
        surface = monitors_.at(0).renderDevice->GetCurrentSurface();
        surface->DownLoadSurfaceData((uint8_t*)ptr,linesize);
    }

    else {
        surface = monitors_.at(m_curMonitorIdx).renderDevice->GetCurrentSurface();
        surface->DownLoadSurfaceData((uint8_t*)ptr, linesize);
    }

    return;
}

void DuplicatorCapture::LoadCaptureImageRGBA(void** ptr, int* stride, int* width, int* height)
{
    DirectX11Surface2D* surface = NULL;
    if (m_bCombine) {
        surface = monitors_.at(0).renderDevice->GetCurrentSurface();
        surface->DownLoadSurfaceData();
    }

    else {
        surface = monitors_.at(m_curMonitorIdx).renderDevice->GetCurrentSurface();
        surface->DownLoadSurfaceData();
    }

    if (ptr && surface)
    {
        *ptr = surface->m_data;
    }

    if (stride)
    {
        *stride = surface->m_linesize;
    }
    if (width) {
        *width = surface->m_width;
    }
    if (height) {
        *height = surface->m_height;
    }

}