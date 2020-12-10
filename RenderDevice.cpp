#include "stdafx.h"
#include "RenderDevice.h"


DirectX11Texture2D::DirectX11Texture2D(const std::string& name, RenderDevice* render) :
    m_name(name), m_renderDevice(render){
}

DirectX11Texture2D::~DirectX11Texture2D(){
    destroyTexture2D();
}


bool DirectX11Texture2D::createTexture2DFromSurface(DirectX11Surface2D* surface){
    if (!surface)
        return false;

    m_width = surface->m_width;
    m_height = surface->m_height;
    m_dxgiFormat = surface->m_dxgiFormat;
    
    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
    desc.ArraySize = 1;
    desc.Width = m_width;
    desc.Height = m_height;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Format = m_dxgiFormat;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags =  D3D11_CPU_ACCESS_WRITE ;

    m_renderDevice->m_device->CreateTexture2D(&desc, 0, &m_texture);
    if (!m_texture)
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = m_dxgiFormat;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    m_renderDevice->m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_shaderResView);
    if (!m_shaderResView)
        return false;

    if (!surface->DownLoadSurfaceData())
        return false;

    TextureSetImage(surface->m_data,surface->m_linesize ,false);
    return true;
}


bool DirectX11Texture2D::createTexture2D(int width, int height, unsigned flags, DXGI_FORMAT format, bool gdi){
    m_width = width;
    m_height = height;
    m_flag = flags;
    m_dxgiFormat = format;
    bRenderTarget = ((flags & FLAG_RENDER_TARGET) != 0);
    bGDICompatible = gdi;
    bDynamic = ((flags & FLAG_DYNAMIC) != 0);

    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
    desc.ArraySize = 1;
    desc.Width = m_width;
    desc.Height = m_height;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Format = m_dxgiFormat;
    desc.Usage = bDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = bDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    if (bRenderTarget || bGDICompatible)
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

    if (bGDICompatible)
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    m_renderDevice->m_device->CreateTexture2D(&desc, 0, &m_texture);
    if (!m_texture)
        return false;

    if (bGDICompatible)
        m_texture->QueryInterface(__uuidof(IDXGISurface1), (void**)&m_gdiSurface);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = m_dxgiFormat;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    m_renderDevice->m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_shaderResView);
    if (!m_shaderResView)
        return false;

    if (bRenderTarget){
        D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
        renderDesc.Format = m_dxgiFormat;
        renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderDesc.Texture2D.MipSlice = 0;

        m_renderDevice->m_device->CreateRenderTargetView(m_texture, &renderDesc, &m_renderTarget);
        if (!m_renderTarget)
            return false;
    }

    return true;
}

void DirectX11Texture2D::TextureSetImage(const uint8_t* data, uint32_t linesize, bool flip){
    uint8_t* ptr;
    uint32_t linesize_out;
    uint32_t row_copy;
    int32_t height;
    int32_t y;

    height = m_height;

    if (!TextureMap( &ptr, &linesize_out))
        return;

    row_copy = (linesize < linesize_out) ? linesize : linesize_out;

    if (flip) {
        for (y = height - 1; y >= 0; y--)
            memcpy(ptr + (uint32_t)y * linesize_out,
                data + (uint32_t)(height - y - 1) * linesize,
                row_copy);

    }
    else if (linesize == linesize_out) {
        memcpy(ptr, data, row_copy * height);

    }
    else {
        for (y = 0; y < height; y++)
            memcpy(ptr + (uint32_t)y * linesize_out,
                data + (uint32_t)y * linesize, row_copy);
    }

    TextureUnMap();

    return;
}

bool DirectX11Texture2D::TextureMap(uint8_t** ptr, uint32_t* linesize){
    D3D11_MAPPED_SUBRESOURCE map;
    HRESULT hr;

    hr = m_renderDevice->m_deviceContext->Map(m_texture, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &map);
    if (FAILED(hr))
        return false;

    *ptr = (uint8_t*)map.pData;
    *linesize = map.RowPitch;

    return true;
}

void DirectX11Texture2D::TextureUnMap(){
    m_renderDevice->m_deviceContext->Unmap(m_texture, 0);
}

void DirectX11Texture2D::destroyTexture2D(){
    if (m_texture) {
        m_texture->Release();
        m_texture = NULL;
    }

    if (m_shaderResView) {
        m_shaderResView->Release();
        m_shaderResView = NULL;
    }

    if (m_renderTarget) {
        m_renderTarget->Release();
        m_renderTarget = NULL;
    }

    if (m_gdiSurface) {
        m_gdiSurface->Release();
        m_gdiSurface = NULL;
    }
    return ;
}

DirectX11Surface2D::DirectX11Surface2D(const std::string& name, RenderDevice* render) :
    m_name(name), m_renderDevice(render){
}

DirectX11Surface2D::~DirectX11Surface2D(){
    destroySurface2D();
}

bool DirectX11Surface2D::createSurface2D(int width, int height, DXGI_FORMAT format){
    m_width = width;
    m_height = height;
    m_dxgiFormat = format;

    D3D11_TEXTURE2D_DESC td;
    memset(&td, 0, sizeof(td));
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = format;
    td.SampleDesc.Count = 1;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    td.Usage = D3D11_USAGE_STAGING;
    m_renderDevice->m_device->CreateTexture2D(&td, NULL, &m_texture);
    if (!m_texture)
        return false;

    m_data = NULL;
    return true;
}

bool DirectX11Surface2D::DownLoadSurfaceData(uint8_t* dataOut, int linesizeOut){
    if (!dataOut)
        return false;

    uint32_t linesize = 0;
    uint8_t* data = NULL;
  
    if (DirectX11StageSurfaceMap(&data, &linesize))
    {
        if (linesize == linesizeOut)
        {
            memcpy(dataOut, data, linesize * m_height);
        }
        else {
            uint8_t* in = data;
            uint8_t* out = dataOut;
            int copylinesize = linesize > linesizeOut ? linesizeOut : linesize;
            for (size_t y = 0; y < m_height; y++) {
                memcpy(out, in, copylinesize);
                in += linesize;
                out += linesizeOut;
            }
        }
        DirectX11StageSurfaceUnmap();

        return true;
    }

    return false;
}

void DirectX11Surface2D::FreeSurfaceData()
{
    if (m_data)
    {
        free(m_data);
        m_data = NULL;
    }
}

bool DirectX11Surface2D::DownLoadSurfaceData(){
    uint32_t linesize = 0;
    uint8_t* data = NULL;
   
    if (DirectX11StageSurfaceMap(&data, &linesize))
    {
        if (linesize * m_height > m_linesize* m_height || !m_data){
            if(m_data)
                free(m_data);
            m_data =(uint8_t*) malloc(linesize * m_height);
        }
        m_linesize = linesize;
        memcpy(m_data, data, linesize * m_height);
        DirectX11StageSurfaceUnmap();
        return true;
    }
    
    return false;
}

bool DirectX11Surface2D::DirectX11StageSurfaceMap(uint8_t** data, uint32_t* linesize){
    D3D11_MAPPED_SUBRESOURCE map;
    if (FAILED(m_renderDevice->m_deviceContext->Map(m_texture, 0,
        D3D11_MAP_READ, 0, &map)))
        return false;

    *data = (uint8_t*)map.pData;
    *linesize = map.RowPitch;
    return true;
}


void DirectX11Surface2D::DirectX11StageSurfaceUnmap(){
    m_renderDevice->m_deviceContext->Unmap(m_texture, 0);
}

void DirectX11Surface2D::destroySurface2D(){
    if (m_texture) {
        m_texture->Release();
    }

    if (m_data) {
        free(m_data);
        m_data = NULL;
    }
    return ;
}


DirectX11VertexBuffer::DirectX11VertexBuffer(RenderDevice* device) :
    m_renderDevice(device){
    memset(&m_vertexData, 0, sizeof(m_vertexData));
}

DirectX11VertexBuffer::~DirectX11VertexBuffer(){
    destroyVertexBuffer();
}



bool DirectX11VertexBuffer::createSpriteBuffer(){
    VertexData* vertexData;
    
    vertexData = (VertexData*)malloc(sizeof(VertexData));
    memset(vertexData, 0, sizeof(VertexData));
    vertexData->num = 4;

    vertexData->pointsElementSize = sizeof(float) * 4;
    vertexData->points = (float*)malloc(vertexData->pointsElementSize * 4);
    
    vertexData->num_tex = 1;//
    vertexData->tvarray = (TvertArray*)malloc(sizeof(TvertArray) * 8);// 默认为8个纹理
    for (uint32_t i = 0; i < 8; ++i) {
        vertexData->tvarray[i].num = 4;
        vertexData->tvarray[i].texUVElementSize = sizeof(float) * 2;
        vertexData->tvarray[i].data = (float*)malloc(sizeof(float) * 2 * 4);
    }

    createVertexBuffer(vertexData,true);

    return true;
}

bool DirectX11VertexBuffer::createVertexBuffer(VertexData* data, bool bDynamic ){
    //freeVertexData(m_vertexData);
    m_vertexData = data;
    m_bDynamic = bDynamic;
    
    //sizeof(vec3)
    InitBuffer(data->pointsElementSize,data->num,data->points,&m_vertexBuffer);
    if(data->normals)
        InitBuffer(data->normalsElementSize, data->num, data->normals, &m_normalBuffer);

    if (data->tangents)
        InitBuffer(data->tangentsElementSize, data->num, data->tangents, &m_tangentBuffer);
    
    if (data->colors)
        InitBuffer(data->colorsElementSize, data->num, data->colors, &m_colorBuffer);

    for (size_t i = 0; i < data->num_tex; i++) {
        TvertArray* tverts = data->tvarray + i;

        ID3D11Buffer* buffer;
        InitBuffer(tverts->texUVElementSize, tverts->num,tverts->data, &buffer);

        m_uvBuffers.push_back(buffer);
        m_uvSizes.push_back(tverts->texUVElementSize);
    }

    return true;
}



void DirectX11VertexBuffer::MakeBufferList(DirectX11Shader* shader, std::vector<ID3D11Buffer*>& buffers, std::vector<uint32_t>& strides){
    PushBuffer(buffers, strides, m_vertexBuffer, m_vertexData->pointsElementSize, "point");

    if (shader->m_hasNormals)
        PushBuffer(buffers, strides, m_normalBuffer, m_vertexData->normalsElementSize,"normal");
    if (shader->m_hasColors)
        PushBuffer(buffers, strides, m_colorBuffer, m_vertexData->colorsElementSize,"color");
    if (shader->m_hasTangents)
        PushBuffer(buffers, strides, m_tangentBuffer, m_vertexData->tangentsElementSize,"tangent");
    if (shader->m_nTexUnits <= m_uvBuffers.size()) {
        for (size_t i = 0; i < shader->m_nTexUnits; i++) {
            buffers.push_back(m_uvBuffers[i]);
            strides.push_back((uint32_t)m_uvSizes[i]);
        }
    }
    
    return;
}

void DirectX11VertexBuffer::PushBuffer(std::vector<ID3D11Buffer*>& buffers, std::vector<uint32_t>& strides, ID3D11Buffer* buffer, size_t elementSize, const char* name){
    if (buffer) {
        buffers.push_back(buffer);
        strides.push_back((uint32_t)elementSize);
    }
}

void DirectX11VertexBuffer::FlushVertexBuffer(){
    FlushVertexBuffer(m_vertexData);
}


void DirectX11VertexBuffer::FlushVertexBuffer(VertexData* data){
    if (!m_bDynamic)
        return;

    if (data->points && m_vertexBuffer)
        FlushBuffer(m_vertexBuffer,data->points,data->pointsElementSize,data->num);

    if (data->normals && m_normalBuffer)
        FlushBuffer(m_normalBuffer,data->normals,data->normalsElementSize,data->num);

    if (data->tangents && m_tangentBuffer)
        FlushBuffer(m_tangentBuffer,data->tangents,data->tangentsElementSize,data->num);

    if (data->colors && m_colorBuffer)
        FlushBuffer(m_colorBuffer,data->colors,data->colorsElementSize,data->num);

    for (size_t i = 0; i < data->num_tex; i++) {
        TvertArray& tv = data->tvarray[i];
        FlushBuffer(m_uvBuffers[i], tv.data,
            tv.texUVElementSize,tv.num);
    }

    return;
}

void DirectX11VertexBuffer::FlushBuffer(ID3D11Buffer* buffer, void* ptr, size_t elementSize,size_t num){
    D3D11_MAPPED_SUBRESOURCE msr;
    HRESULT hr;

    if (FAILED(hr = m_renderDevice->m_deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD,
        0, &msr)))
        return;

    memcpy(msr.pData, ptr, elementSize * num);
    m_renderDevice->m_deviceContext->Unmap(buffer, 0);

    return;
}


void DirectX11VertexBuffer::SetVertexData(void* data, size_t size, VertexType type){
    if (type == VertexType_Point)
    {
        if (size < m_vertexData->pointsElementSize * m_vertexData->num)
            return;

        memcpy(m_vertexData->points, data, size);
        return;
    }
}

void DirectX11VertexBuffer::SetTexUVData(void* data, size_t size, uint32_t num){
    if (num != m_vertexData->num_tex)
        return;

    float* src = (float*)data;
    for (uint32_t i = 0; i < num; ++i){
        TvertArray* ptr = m_vertexData->tvarray + i;
        memcpy(ptr->data,src,ptr->texUVElementSize * ptr->num);
        src += ptr->texUVElementSize * ptr->num;
    }

    return;
}


void DirectX11VertexBuffer::InitBuffer(const size_t elementSize, const size_t numVertex, void* data, ID3D11Buffer** buffer){

    D3D11_BUFFER_DESC bd;
    D3D11_SUBRESOURCE_DATA srd;
    HRESULT hr;

    memset(&bd, 0, sizeof(bd));
    memset(&srd, 0, sizeof(srd));

    bd.Usage = m_bDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags = m_bDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = UINT(elementSize * numVertex);
    srd.pSysMem = data;

    hr = m_renderDevice->m_device->CreateBuffer(&bd, &srd, buffer);
    return;
}


void DirectX11VertexBuffer::freeVertexData(VertexData* data){
    uint32_t i;
    if (!data)
        return;

    free(data->points);
    free(data->normals);
    free(data->tangents);
    free(data->colors);
    for (i = 0; i < 8; i++)
        free(data->tvarray[i].data);
    free(data->tvarray);
    free(data);
    
    return;
}

void DirectX11VertexBuffer::destroyVertexBuffer(){
    if (m_vertexBuffer){
        m_vertexBuffer->Release();
        m_vertexBuffer = NULL;
    }
    if (m_normalBuffer){
        m_normalBuffer->Release();
        m_normalBuffer = NULL;
    }
    if (m_colorBuffer){
        m_colorBuffer->Release();
        m_colorBuffer = NULL;
    }
    if (m_tangentBuffer) {
        m_tangentBuffer->Release();
        m_tangentBuffer = NULL;
    }

    for (uint32_t i = 0; i < m_uvBuffers.size(); ++i){
        m_uvBuffers[i]->Release();
    }

    freeVertexData(m_vertexData);
    m_vertexData = NULL;
}


DirectX11SwainChain::DirectX11SwainChain(const std::string& str,RenderDevice* device):m_device(device),
m_name(str){
}

DirectX11SwainChain::~DirectX11SwainChain(){
    destroySwainChain();
}

bool DirectX11SwainChain::CreateSwainChain(DirectX11SwainInfo* info)
{
    DXGI_SWAP_CHAIN_DESC desc;
    memset(&desc,0,sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.BufferCount = info->numBackbuffers;
    m_numBackbuffers = info->numBackbuffers;
    desc.BufferDesc.Format = info->format;
    m_format = info->format;
    desc.BufferDesc.Width = info->cx;
    desc.BufferDesc.Height = info->cy;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = (HWND)info->hwnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = true;
    m_hwnd = (HWND)info->hwnd;
    HRESULT hr;
    hr = m_device->m_factory->CreateSwapChain(m_device->m_device,&desc,&m_swapChain);
    if (!m_swapChain)
        return false;

    /* Ignore Alt+Enter */
     m_device->m_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);

    m_target = new DirectX11Texture2D("swapTarget",m_device);
    m_target->bRenderTarget = true;
    m_target->m_dxgiFormat = info->format;

    m_width = info->cx;
    m_height = info->cy;

   return InitTarget(info->cx,info->cy);
}


bool DirectX11SwainChain::ResizeSwapChain(uint32_t cx, uint32_t cy)
{
    RECT clientRect;
    HRESULT hr;
    delete m_target;
    m_target = new DirectX11Texture2D("swapTarget",m_device);

    m_target->bRenderTarget = true;
    m_target->m_dxgiFormat = m_format;
    if (cx == 0 || cy == 0) {
        GetClientRect(m_hwnd, &clientRect);
        if (cx == 0)
            cx = clientRect.right;
        if (cy == 0)
            cy = clientRect.bottom;
    }


    hr = m_swapChain->ResizeBuffers(m_numBackbuffers, cx, cy, m_target->m_dxgiFormat, 0);
    if (FAILED(hr))
        return false;

    m_width = cx;
    m_height = cy;
    InitTarget(cx, cy);
    return true;

}

void DirectX11SwainChain::destroySwainChain(){
    if (m_target) {
        delete m_target;
        m_target = NULL;
    }

    if (m_swapChain) {
        m_swapChain->Release();
        m_swapChain = NULL;
    }

    return ;
}

bool DirectX11SwainChain::InitTarget(uint32_t cx, uint32_t cy){
    m_target->m_width = cx;
    m_target->m_height = cy;

    HRESULT hr;
    hr = m_swapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&m_target->m_texture);
    if (hr != S_OK)
        return false;

    hr = m_device->m_device->CreateRenderTargetView(m_target->m_texture,
        NULL,&m_target->m_renderTarget);

    if (hr != S_OK)
        return false;

    return true;
}


DirectX11Shader::DirectX11Shader(const std::string& str, RenderDevice* device):
    m_name(str),
    m_renderDevice(device){
}

DirectX11Shader::~DirectX11Shader(){
}

void DirectX11Shader::BuildLayout(){
    return;
}

bool DirectX11Shader::CreateShader(const char* vertex_shader, const char* pixel_shader, DirectX11ShaderType type){
    ID3D10Blob* shaderVSBlob = NULL;
    ID3D10Blob* pErrorVS;
    D3DCompile(vertex_shader, strlen(vertex_shader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &shaderVSBlob, &pErrorVS);
    if (pErrorVS) {
        pErrorVS->GetBufferPointer();
        pErrorVS->Release();
    }

    if (shaderVSBlob == NULL)
        goto failed;
    
    if (m_renderDevice->m_device->CreateVertexShader((DWORD*)shaderVSBlob->GetBufferPointer(), shaderVSBlob->GetBufferSize(), NULL, &m_vertexShader) != S_OK)
        goto failed;
    
    ID3D10Blob* shaderPSBlob = NULL;
    ID3D10Blob* pErrorPS;
    D3DCompile(pixel_shader, strlen(pixel_shader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &shaderPSBlob, &pErrorPS);
    if (pErrorPS) {
        pErrorPS->GetBufferPointer();
        pErrorPS->Release();
    }

    if (shaderPSBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
        goto failed;

    if (m_renderDevice->m_device->CreatePixelShader((DWORD*)shaderPSBlob->GetBufferPointer(), shaderPSBlob->GetBufferSize(), NULL, &m_pixelShader) != S_OK)
        goto failed;
    
    m_shaderType = type;
    if (type == DirectX11Shader_RGBA){
        D3D11_INPUT_ELEMENT_DESC local_layout[] =
        {
            { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, 0,       D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,         1, 0,      D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        m_hasColors = false;
        m_hasNormals = false;
        m_hasTangents = false;
        m_nTexUnits = 1;

        if (m_renderDevice->m_device->CreateInputLayout(local_layout, 2, shaderVSBlob->GetBufferPointer(), shaderVSBlob->GetBufferSize(), &m_inputLayout) != S_OK)
            goto failed;


        D3D11_BUFFER_DESC bd;
        memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
        bd.ByteWidth = sizeof(float) * 4 * 4; /* align */
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        m_renderDevice->m_device->CreateBuffer(&bd, NULL,
            &m_constantVSBuffer);
        if (m_constantVSBuffer == NULL)
            goto failed;

        
        D3D11_SAMPLER_DESC sampDesc;
        memset(&sampDesc, 0, sizeof(sampDesc));
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;// D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;//Clamp
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0.0f;
        sampDesc.MinLOD = 0;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        HRESULT hr = m_renderDevice->m_device->CreateSamplerState(&sampDesc, &m_samplerState[0]);
        if (hr==S_FALSE)
            goto failed;

        if (shaderVSBlob){
            shaderVSBlob->Release();
            shaderVSBlob = NULL;
        }
        if (shaderPSBlob){
            shaderPSBlob->Release();
            shaderPSBlob = NULL;
        }
        return true;
    }

    if (type == DirectX11Shader_RGBA2I420Y ){
        m_hasColors = false;
        m_hasNormals = false;
        m_hasTangents = false;
        m_nTexUnits = 1;
        D3D11_BUFFER_DESC bd;
        memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
        bd.ByteWidth = sizeof(float) * 4; /* align */
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        m_renderDevice->m_device->CreateBuffer(&bd, NULL,
            &m_constantPSBuffer);
        if (m_constantPSBuffer == NULL)
            goto failed;

        if (shaderVSBlob) {
            shaderVSBlob->Release();
            shaderVSBlob = NULL;
        }
        if (shaderPSBlob){
            shaderPSBlob->Release();
            shaderPSBlob = NULL;
        }
        return true;
    }

    if (type == DirectX11Shader_RGBA2I420U || type == DirectX11Shader_RGBA2I420V){
        m_hasColors = false;
        m_hasNormals = false;
        m_hasTangents = false;
        m_nTexUnits = 1;
        D3D11_BUFFER_DESC bd;
        memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
        bd.ByteWidth = sizeof(float) * 4;
       
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        m_renderDevice->m_device->CreateBuffer(&bd, NULL,
            &m_constantVSBuffer);
        if (m_constantVSBuffer == NULL)
            goto failed;

        memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
        bd.ByteWidth = sizeof(float) * 4; /* align */
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        m_renderDevice->m_device->CreateBuffer(&bd, NULL,
            &m_constantPSBuffer);
        if (m_constantPSBuffer == NULL)
            goto failed;

        D3D11_SAMPLER_DESC sampDesc;
        memset(&sampDesc, 0, sizeof(sampDesc));
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.MaxAnisotropy = 1;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0.0f;
        sampDesc.MinLOD = 0;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        HRESULT hr = m_renderDevice->m_device->CreateSamplerState(&sampDesc, &m_samplerState[0]);
        if (shaderVSBlob){
            shaderVSBlob->Release();
            shaderVSBlob = NULL;
        }
        if (shaderPSBlob){
            shaderPSBlob->Release();
            shaderPSBlob = NULL;
        }

        return true;
    }
 
failed:
    if (shaderVSBlob){
        shaderVSBlob->Release();
        shaderVSBlob = NULL;
    }
    if (shaderPSBlob){
        shaderPSBlob->Release();
        shaderPSBlob = NULL;
    }

    destroyShader();
    return false;
}

void DirectX11Shader::VertexShaderLoad(){
    m_renderDevice->m_deviceContext->VSSetShader(m_vertexShader,0,0);
    m_renderDevice->m_deviceContext->IASetInputLayout(m_inputLayout);
    m_renderDevice->m_deviceContext->VSSetConstantBuffers(0,1,&m_constantVSBuffer);
}

void DirectX11Shader::PixelShaderLoad(){
    m_renderDevice->m_deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    m_renderDevice->m_deviceContext->PSSetConstantBuffers(0, 1, &m_constantPSBuffer);
    m_renderDevice->m_deviceContext->PSSetSamplers(0, 8, m_samplerState);
}

void DirectX11Shader::UpdateVertexConstantBuffer(mat4x4 matrix){
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    if (m_renderDevice->m_deviceContext->Map(m_constantVSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
        return ;
    int sizecopy = sizeof(mat4x4);
    memcpy(mapped_resource.pData, matrix, sizeof(mat4x4));

    m_renderDevice->m_deviceContext->Unmap(m_constantVSBuffer, 0);
}

void DirectX11Shader::UpdateVertexConstantBuffer(float width){
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    if (m_renderDevice->m_deviceContext->Map(m_constantVSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
        return;

    memcpy(mapped_resource.pData, &width, sizeof(float));

    m_renderDevice->m_deviceContext->Unmap(m_constantVSBuffer, 0);
}

void DirectX11Shader::UpdatePixelConstantBuffer(vec4 vec){
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    if (m_renderDevice->m_deviceContext->Map(m_constantPSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
        return;

    memcpy(mapped_resource.pData, vec, sizeof(vec4));

    m_renderDevice->m_deviceContext->Unmap(m_constantPSBuffer, 0);
    return;
}

void DirectX11Shader::LoadVertexBufferData(DirectX11VertexBuffer* vertexBuffer){
   
    if (!vertexBuffer)
        return;

    std::vector<ID3D11Buffer*> buffers;
    std::vector<uint32_t> strides;
    std::vector<uint32_t> offsets;
    vertexBuffer->MakeBufferList(this,buffers,strides);
    offsets.resize(buffers.size());
    m_renderDevice->m_deviceContext->IASetVertexBuffers(0,buffers.size(),buffers.data(),strides.data(),offsets.data());
    return;

}

void DirectX11Shader::UpdateTextureResources(DirectX11Texture2D* tex, uint32_t index){
    if (index < m_nTexUnits && index >= 0){
        m_renderDevice->m_deviceContext->PSSetShaderResources(index,1,&tex->m_shaderResView);
        m_renderDevice->m_deviceContext->PSSetSamplers(index,1,&m_samplerState[index]);
    }

    return;
}

bool DirectX11Shader::destroyShader(){
    if (m_constantVSBuffer){
        m_constantVSBuffer->Release();
        m_constantVSBuffer = NULL;
    }

    if (m_inputLayout){
        m_inputLayout->Release();
        m_inputLayout = NULL;
    }

    if (m_vertexShader){
        m_vertexShader->Release();
        m_vertexShader = NULL;
    }

    if (m_constantPSBuffer){
        m_constantPSBuffer->Release();
        m_constantPSBuffer = NULL;
    }

    if (m_pixelShader){
        m_pixelShader->Release();
        m_pixelShader = NULL;
    }

    for (uint32_t i = 0; i < 8; ++i) {
        if (m_samplerState[i]){
            m_samplerState[i]->Release();
            m_samplerState[i] = NULL;
        }
    }

    return true;
}


RenderDevice::RenderDevice(DirectX11Rect *rect):
    m_hwnd(NULL){
    m_screenRect = *rect;
    memset(&m_mutexDevice,0,sizeof(m_mutexDevice));
    mat4x4_identity(m_matCurrentModelMatrix);
    mat4x4_identity(m_matCurrentViewProjMatrix);

    return;
}

RenderDevice::~RenderDevice(){
    RenderDeviceShutdown();
}

bool RenderDevice::isInit(){
    return m_bInitial;
}

bool RenderDevice::RenderDeviceInit(ID3D11Device* device, ID3D11DeviceContext* device_context){
    IDXGIDevice* pDXGIDevice = NULL;
    IDXGIAdapter* pDXGIAdapter = NULL;
    IDXGIFactory* pFactory = NULL;

    if (device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
        if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
            if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK) {
                m_device = device;
                m_deviceContext = device_context;
                m_factory = pFactory;
            }


    if (!m_device || !m_deviceContext || !m_factory){
        return false;
    }

    if (pDXGIDevice)
        pDXGIDevice->Release();
    if (pDXGIAdapter)
        pDXGIAdapter->Release();

    m_rgbRenderShader = new DirectX11Shader("rgbRenderShader",this);
    m_rgbRenderShader->CreateShader(vertexshaderRGBA, pixelshaderRGBA,DirectX11ShaderType::DirectX11Shader_RGBA);

    m_rgbToI420ShaderY = new DirectX11Shader("rgbToI420ShaderY",this);
    m_rgbToI420ShaderY->CreateShader(vertexShaderRGBAToI420Y,pixelShaderRGBAToI420Y,DirectX11ShaderType::DirectX11Shader_RGBA2I420Y);

    m_rgbToI420ShaderU = new DirectX11Shader("rgbToI420ShaderU", this);
    m_rgbToI420ShaderU->CreateShader(vertexShaderRGBAToI420UV,pixelShaderRGBAToI420U, DirectX11ShaderType::DirectX11Shader_RGBA2I420U);

    m_rgbToI420ShaderV = new DirectX11Shader("rgbToI420ShaderV", this);
    m_rgbToI420ShaderV->CreateShader(vertexShaderRGBAToI420UV, pixelShaderRGBAToI420V, DirectX11ShaderType::DirectX11Shader_RGBA2I420V);


    m_spriteVertexBuffer = new DirectX11VertexBuffer(this);
    m_spriteVertexBuffer->createSpriteBuffer();

    memset(&m_cursorData,0,sizeof(cursor_data));

    InitializeCriticalSection(&m_mutexDevice);
    SetTargetParam(640,480);

    // Create Blend
    D3D11_BLEND_DESC blend_desc;
    memset(&blend_desc, 0, sizeof(blend_desc));
    for (int i = 0; i < 8; ++i)  {   
        // 混合方程：C = C(src)xF(src)(任意操作)C(dst)xFdst
        // 假定现在ps输出的像素颜色是Csrc = (Rs , Gs , Bs),后缓冲中的对应像素颜色值是Cdst = (Rd , Gd , Bd),
        // 
        blend_desc.RenderTarget[i].BlendEnable = true;
        blend_desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;  //混合方法 任意操作操作
        blend_desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA; // x操作 F=(1-rs,..)
        blend_desc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

        blend_desc.RenderTarget[i].RenderTargetWriteMask =
            D3D11_COLOR_WRITE_ENABLE_RED |
            D3D11_COLOR_WRITE_ENABLE_GREEN |
            D3D11_COLOR_WRITE_ENABLE_BLUE |
            0;
    }
    device->CreateBlendState(&blend_desc,&m_currentBlendState);
    if (!m_currentBlendState)
        return false;

    m_bInitial = true;

    
    return true;
}

bool RenderDevice::CreateSwainChain(HWND hwnd,uint32_t cx,uint32_t cy){
    m_hwnd = hwnd;
    if (m_hwnd) {
        m_swapChain = new DirectX11SwainChain("SwChain", this);
        DirectX11SwainInfo info;
        info.cx = cx;
        info.cy = cy;
        info.hwnd = m_hwnd;
        info.numBackbuffers = 2;
        info.format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM;
        return m_swapChain->CreateSwainChain(&info);

    }
    return false;
}


/*Float4X4 RenderDevice::UpdateViewProjMatrix(float left, float right, float top,
    float bottom, float zNear, float zFar){
    Float4X4 viewProj;
    float rml = right - left;
    float bmt = bottom - top;
    float fmn = zFar - zNear;
    
    viewProj.m[0][0] = 2.0f / rml;
    viewProj.m[0][1] = 0.0f;
    viewProj.m[0][2] = 0.0f;
    viewProj.m[0][3] = 0.0f;

    viewProj.m[1][0] = 0.0f;
    viewProj.m[1][1] = 2.0f / -bmt;
    viewProj.m[1][2] = 0.0f;
    viewProj.m[1][3] = 0.0f;

    viewProj.m[2][0] = 0.0f;
    viewProj.m[2][1] = 0.0f;
    viewProj.m[2][2] = -1 / fmn;
    viewProj.m[2][3] = 0.0f;

    viewProj.m[3][0] = (left + right) / -rml;
    viewProj.m[3][1] = (bottom + top) / bmt;
    viewProj.m[3][2] =  zNear / -fmn;
    viewProj.m[3][3] = 1.0f;

    
    return viewProj;
}
*/


void RenderDevice::SetCaptureRect(float posX0, float posY0, float sub_cx, float sub_cy){
    float posX = posX0 - m_screenRect.x;
    float posY = posY0 - m_screenRect.y;

    float u1 = (float)posX / (float)m_screenRect.w;
    float v1 = (float)posY / (float)m_screenRect.h;

    float u2 = (float)(posX + sub_cx) / (float)m_screenRect.w;
    float v2 = (float)(posY + sub_cy) / (float)m_screenRect.h;

    m_screenCaptureUV[0][0] = u1;
    m_screenCaptureUV[0][1] = v1;

    m_screenCaptureUV[1][0] = u2;
    m_screenCaptureUV[1][1] = v1;

    m_screenCaptureUV[2][0] = u1;
    m_screenCaptureUV[2][1] = v2;

    m_screenCaptureUV[3][0] = u2;
    m_screenCaptureUV[3][1] = v2;

    m_captureRect.x = posX0 ;
    m_captureRect.y = posY0 ;
    m_captureRect.w = sub_cx;
    m_captureRect.h = sub_cy;

    return;
}


DirectX11Surface2D* RenderDevice::GetSurfaceFromCache(uint32_t cx, uint32_t cy, DXGI_FORMAT format, ArraySurface& list){
    DirectX11Surface2D* surface = NULL;
    for (ArraySurface::iterator iter = list.begin(); iter != list.end(); ++iter){
        if ((*iter)->m_width == cx &&
            (*iter)->m_height == cy &&
            (*iter)->m_dxgiFormat == format){
            return *iter;
        }
    }

    surface = new DirectX11Surface2D("surface2D", this);
    surface->createSurface2D(cx,cy,format);
    list.push_back(surface);

    return surface;
}

DirectX11Texture2D* RenderDevice::GetTextureFromCache(uint32_t cx, uint32_t cy, DXGI_FORMAT format,int flag, ArrayTexture& list,bool bGdi){
    DirectX11Texture2D* tex = NULL;
    for (ArrayTexture::iterator iter = list.begin(); iter != list.end(); ++iter){
        if ((*iter)->m_width == cx &&
            (*iter)->m_height == cy &&
            (*iter)->m_dxgiFormat == format){
            return *iter;
        }
    }

    tex = new DirectX11Texture2D("texture2D", this);
    tex->createTexture2D(cx, cy, flag, format,bGdi);
    list.push_back(tex);
    
    return tex;
}

bool RenderDevice::SetTargetParam(int width, int height, DXGI_FORMAT format,bool bGdi){
    if (m_renderTargetWidth = width && m_renderTargetHeight == height)
        return true;

    DirectX11Texture2D* texTarget = GetTextureFromCache(width, height, format, FLAG_RENDER_TARGET, m_TexTargetList, bGdi);
    if (texTarget) {
        m_renderTargetWidth = width;
        m_renderTargetHeight = height;
        m_currentTarget = texTarget;

        //创建表面
        for (uint32_t i = 0; i < 2; ++i) {
            if (m_currentSurface[i]) {
                delete m_currentSurface[i];
                m_currentSurface[i] = NULL;
            }

            m_currentSurface[i] = new DirectX11Surface2D("surface2D", this);
            m_currentSurface[i]->createSurface2D(m_renderTargetWidth, m_renderTargetHeight, format);
        }

        // 创建I420 target
        for (uint32_t i = 0; i < 3; ++i) {
            if (m_convertTargetYUV[i]) {
                delete m_convertTargetYUV[i];
                m_convertTargetYUV[i] = NULL;
            }
        }
       

        m_convertTargetYUV[0] = new DirectX11Texture2D("convertTargetY",this);
        m_convertTargetYUV[0]->createTexture2D(m_renderTargetWidth, m_renderTargetHeight,FLAG_RENDER_TARGET, DXGI_FORMAT_R8_UNORM);

        m_convertTargetYUV[1] = new DirectX11Texture2D("convertTargetU", this);
        m_convertTargetYUV[1]->createTexture2D(m_renderTargetWidth/2, m_renderTargetHeight/2, FLAG_RENDER_TARGET, DXGI_FORMAT_R8_UNORM);

        m_convertTargetYUV[2] = new DirectX11Texture2D("convertTargetV", this);
        m_convertTargetYUV[2]->createTexture2D(m_renderTargetWidth/2, m_renderTargetHeight/2, FLAG_RENDER_TARGET, DXGI_FORMAT_R8_UNORM);

        // 创建I420 surface
        for (uint32_t i = 0; i < 2; ++i) {
            for (uint32_t j = 0; j < 3; ++j) {
                if (m_convertSurface[i][j]){
                    delete m_convertSurface[i][j];
                    m_convertSurface[i][j] = NULL;
                }
            }
        }

        m_convertSurface[0][0] = new DirectX11Surface2D("convertSurface",this);
        m_convertSurface[0][0]->createSurface2D(m_renderTargetWidth, m_renderTargetHeight, DXGI_FORMAT_R8_UNORM);
        m_convertSurface[1][0] = new DirectX11Surface2D("convertSurface",this);
        m_convertSurface[1][0]->createSurface2D(m_renderTargetWidth, m_renderTargetHeight, DXGI_FORMAT_R8_UNORM);
        for(uint32_t i = 0; i<2;++i)
            for (uint32_t j = 1; j < 3; ++j){
                m_convertSurface[i][j] = new DirectX11Surface2D("convertSurface", this);
                m_convertSurface[i][j]->createSurface2D(m_renderTargetWidth / 2, m_renderTargetHeight / 2, DXGI_FORMAT_R8_UNORM);

            }
        return true;
    }

    return false;
}

bool RenderDevice::RenderCombine(DirectX11Texture2D* tex1, DirectX11Surface2D* surface, float combineRatio, CombDirection flag){
    if (!tex1 || !surface)
        return false;

    DirectX11Texture2D* tex2Temp = new DirectX11Texture2D("textureSurface",this);
    tex2Temp->createTexture2DFromSurface(surface);

    RenderCombine(tex1,tex2Temp, combineRatio, flag);
    delete tex2Temp;
    tex2Temp = NULL;

    return true;
}

bool RenderDevice::RenderCombine(DirectX11Texture2D* tex1, DirectX11Texture2D* tex2, float combineRatio, CombDirection flag){
    if (!tex1 || !tex2){
        return false;
    }

    DirectX11Texture2D* texTarget = GetTextureFromCache(m_renderTargetWidth, m_renderTargetHeight, tex1->m_dxgiFormat, FLAG_RENDER_TARGET, m_TexCombineList, true);
    if (texTarget){
        m_currentCombineTarget = texTarget;
    }

    D3D11_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D11_VIEWPORT));
    vp.Width = m_renderTargetWidth;
    vp.Height = m_renderTargetHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;

    m_deviceContext->OMSetRenderTargets(1, &m_currentCombineTarget->m_renderTarget, NULL);
    m_deviceContext->RSSetViewports(1, &vp);
    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_deviceContext->ClearRenderTargetView(m_currentCombineTarget->m_renderTarget, clearColor);

    float combineRatioXY = combineRatio;
    DirectX11Texture2D* texTemp1 = tex1;
    DirectX11Texture2D* texTemp2 = tex2;
    if (flag == CombDirection::Combine_RightLeft || flag  == CombDirection::Combine_DownUp){
        DirectX11Texture2D* texTemp1 = tex2;
        DirectX11Texture2D* texTemp2 = tex1;
        combineRatioXY = 1 / combineRatio;
    }
   
    mat4x4_ortho(m_matCurrentViewProjMatrix, 0, texTemp1->m_width, texTemp1->m_height, 0, -100,100);
    mat4x4_identity(m_matCurrentModelMatrix);
   
    if (flag == CombDirection::Combine_RightLeft || flag == CombDirection::Combine_LeftRight){
        mat4x4_scale_aniso(m_matCurrentModelMatrix, m_matCurrentModelMatrix, combineRatioXY / (1 + combineRatioXY), 1, 1);
    }
    if (flag == Combine_DownUp || flag == Combine_UpDown)  {
        mat4x4_scale_aniso(m_matCurrentModelMatrix, m_matCurrentModelMatrix, 1, combineRatioXY / (1 + combineRatioXY), 1);
        
    }
    
    RenderSprite(texTemp1, 0, 0);

    mat4x4_ortho(m_matCurrentViewProjMatrix, 0, texTemp2->m_width, texTemp2->m_height, 0,  -100, 100);
    mat4x4 TransMatrix;
    mat4x4_identity(m_matCurrentModelMatrix);
    mat4x4_identity(TransMatrix);

    if (flag == CombDirection::Combine_RightLeft || flag == CombDirection::Combine_LeftRight){
        mat4x4_translate(TransMatrix, m_renderTargetWidth * (combineRatioXY / (1 + combineRatioXY)), 0, 0);
        mat4x4_scale_aniso(m_matCurrentModelMatrix, m_matCurrentModelMatrix, 1 / (1 + combineRatioXY), 1, 1);
    }

    if (flag == Combine_DownUp || flag == Combine_UpDown){
        mat4x4_translate(TransMatrix, 0, m_renderTargetHeight * (combineRatioXY / (1 + combineRatioXY)), 0);
        mat4x4_scale_aniso(m_matCurrentModelMatrix, m_matCurrentModelMatrix, 1, 1 / (1 + combineRatioXY), 1);
    }
    
    mat4x4_mul(m_matCurrentModelMatrix, TransMatrix,m_matCurrentModelMatrix);
    RenderSprite(texTemp2, 0, 0); 
   
    RenderConvertSprite(m_currentCombineTarget, m_convertTargetYUV[0], m_rgbToI420ShaderY, m_convertTargetYUV[0]->m_width, m_convertTargetYUV[0]->m_height);
    RenderConvertSprite(m_currentCombineTarget, m_convertTargetYUV[1], m_rgbToI420ShaderU, m_convertTargetYUV[1]->m_width, m_convertTargetYUV[1]->m_height);
    RenderConvertSprite(m_currentCombineTarget, m_convertTargetYUV[2], m_rgbToI420ShaderV, m_convertTargetYUV[2]->m_width, m_convertTargetYUV[2]->m_height);

    m_deviceContext->CopyResource(m_convertSurface[m_convertSurfaceIndex][0]->m_texture, m_convertTargetYUV[0]->m_texture);
    m_deviceContext->CopyResource(m_convertSurface[m_convertSurfaceIndex][1]->m_texture, m_convertTargetYUV[1]->m_texture);
    m_deviceContext->CopyResource(m_convertSurface[m_convertSurfaceIndex][2]->m_texture, m_convertTargetYUV[2]->m_texture);

    m_deviceContext->CopyResource(m_currentSurface[m_surfaceIndex]->m_texture, m_currentCombineTarget->m_texture);
    m_convertSurfaceIndex = ++m_convertSurfaceIndex % 2;

    return false;
}



bool RenderDevice::PointInRect(int posx, int posy, DirectX11Rect *r){
    return ((posx >= r->x) && (posx < (r->x + r->w)) &&
        (posy >= r->y) && (posy < (r->y + r->h))) ? true : false;
}

bool RenderDevice::RenderCursor(){ 
    CursorCapture(&m_cursorData);
    if (!m_cursorData.visible)
        return false;

    if (!PointInRect(m_cursorData.cursor_pos.x, m_cursorData.cursor_pos.y, &m_screenRect)){
        return false;
    }

    if (!PointInRect(m_cursorData.cursor_pos.x, m_cursorData.cursor_pos.y, &m_captureRect)){
        return false;
    }

    int transX = m_cursorData.cursor_pos.x - m_captureRect.x - m_cursorData.x_hotspot;
    int transY = m_cursorData.cursor_pos.y - m_captureRect.y - m_cursorData.y_hotspot;
  
    float scaleX = (float)m_captureRect.w / (float)m_renderTargetWidth;
    float scaleY = (float)m_captureRect.h / (float)m_renderTargetHeight;

    DirectX11Texture2D* texTemp1 = m_cursorData.texture;
    if (!texTemp1)
        return false;

    D3D11_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D11_VIEWPORT));
    vp.Width = texTemp1->m_width/scaleX;
    vp.Height = texTemp1->m_height/scaleY;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = (float)transX / scaleX;
    vp.TopLeftY = (float)transY / scaleY;
    m_deviceContext->RSSetViewports(1, &vp);

    ID3D11BlendState* BlendState = NULL;
    FLOAT                       BlendFactor[4];
    UINT                        SampleMask;
    m_deviceContext->OMGetBlendState(&BlendState, BlendFactor, &SampleMask);

    float f[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_deviceContext->OMSetBlendState(m_currentBlendState, f, 0xFFFFFFFF);

    mat4x4_ortho(m_matCurrentViewProjMatrix, 0,texTemp1->m_width, texTemp1->m_height, 0 ,-100,100);
    mat4x4_identity(m_matCurrentModelMatrix);
    RenderSprite(texTemp1 ,0, 0);
  
    m_deviceContext->OMSetBlendState(BlendState,BlendFactor, SampleMask);
    if (BlendState){
        BlendState->Release();
    }

    return true;
}



bool RenderDevice::RenderSprite(DirectX11Texture2D* texture, uint32_t width, uint32_t height){
    float fcx, fcy;
    VertexData* vertexData;

    if (!texture){
        return false;
    }

    fcx = width ? (float)width : (float)(texture)->m_width;
    fcy = height ? (float)height : (float)(texture)->m_height;

    float points[4][4];
    memset(points,0,sizeof(points));
    points[0][0] = 0.0f; points[0][1] = 0.0f;
    points[1][0] = fcx;points[1][1] = 0.0f;
    points[2][0] = 0.0f; points[2][1] = fcy;
    points[3][0] = fcx;points[3][1] = fcy;

    float uv[4][2];// start 0 end 1(u) start 0 end 1
    uv[0][0] = 0.0f;uv[0][1] = 0.0f;
    uv[1][0] = 1.0f; uv[1][1] = 0.0f;
    uv[2][0] = 0.0f; uv[2][1] = 1.0f;
    uv[3][0] = 1.0f;uv[3][1] = 1.0f;

    m_rgbRenderShader->VertexShaderLoad();
    m_rgbRenderShader->PixelShaderLoad();

    m_spriteVertexBuffer->SetVertexData(points,sizeof(float)*4*4,VertexType::VertexType_Point);
    m_spriteVertexBuffer->SetTexUVData(uv,sizeof(float)*4*2,1);
    m_spriteVertexBuffer->FlushVertexBuffer();

    //shader
    m_rgbRenderShader->LoadVertexBufferData(m_spriteVertexBuffer);
    m_rgbRenderShader->UpdateTextureResources(texture,0);

    mat4x4 matrixContant;
    mat4x4 matrixContantT;
    m_matCurrentModelMatrix[0][2] = -m_matCurrentModelMatrix[0][2];
    m_matCurrentModelMatrix[1][2] = -m_matCurrentModelMatrix[1][2];
    m_matCurrentModelMatrix[2][2] = -m_matCurrentModelMatrix[2][2];
    m_matCurrentModelMatrix[3][2] = -m_matCurrentModelMatrix[3][2];
    mat4x4_mul(matrixContant, m_matCurrentViewProjMatrix, m_matCurrentModelMatrix);
    mat4x4_transpose(matrixContantT, matrixContant);

    m_rgbRenderShader->UpdateVertexConstantBuffer(matrixContantT);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_deviceContext->Draw(4, 0);
    return true;
}


bool RenderDevice::RenderSubSprite(DirectX11Texture2D* texture,uint32_t sub_x, uint32_t sub_y, uint32_t sub_cx, uint32_t sub_cy, bool isScreenCap){
    float fcx, fcy;
    VertexData* vertexData;

    if (!texture)
         return false;
   

    fcx =  (float)(texture)->m_width;
    fcy =  (float)(texture)->m_height;

    float points[4][4];
    memset(points, 0, sizeof(points));
    points[0][0] = 0.0f; points[0][1] = 0.0f;
    points[1][0] = fcx; points[1][1] = 0.0f;
    points[2][0] = 0.0f; points[2][1] = fcy;
    points[3][0] = fcx; points[3][1] = fcy;

    float uv[4][2];// start 0 end 1(u) start 0 end 1
    float start_u, end_u;
    float start_v, end_v;

    start_u = (float)sub_x / fcx;
    end_u = ((float)sub_x + (float)sub_cx) / fcx;
    start_v = (float)sub_y / fcy;
    end_v = ((float)sub_y + (float)sub_cy) / fcy;
    
  
    uv[0][0] = start_u; uv[0][1] = start_v;
    uv[1][0] = end_u; uv[1][1] = start_v;
    uv[2][0] = start_u; uv[2][1] = end_v;
    uv[3][0] = end_u; uv[3][1] = end_v;

    m_rgbRenderShader->VertexShaderLoad();
    m_rgbRenderShader->PixelShaderLoad();

    m_spriteVertexBuffer->SetVertexData(points, sizeof(float) * 4 * 4, VertexType::VertexType_Point);
    m_spriteVertexBuffer->SetTexUVData(m_screenCaptureUV,sizeof(float )* 4 * 2,1);
    if(!isScreenCap)
        m_spriteVertexBuffer->SetTexUVData(uv, sizeof(float) * 4 * 2, 1);

    m_spriteVertexBuffer->FlushVertexBuffer();

    //shader
    m_rgbRenderShader->LoadVertexBufferData(m_spriteVertexBuffer);
    m_rgbRenderShader->UpdateTextureResources(texture, 0);

    mat4x4 matrixContant;
    mat4x4 matrixContantT;
    m_matCurrentModelMatrix[0][2] = -m_matCurrentModelMatrix[0][2];
    m_matCurrentModelMatrix[1][2] = -m_matCurrentModelMatrix[1][2];
    m_matCurrentModelMatrix[2][2] = -m_matCurrentModelMatrix[2][2];
    m_matCurrentModelMatrix[3][2] = -m_matCurrentModelMatrix[3][2];
    mat4x4_mul(matrixContant, m_matCurrentViewProjMatrix,m_matCurrentModelMatrix);
    mat4x4_transpose(matrixContantT, matrixContant);

    m_rgbRenderShader->UpdateVertexConstantBuffer(matrixContantT);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_deviceContext->Draw(4, 0);
    return true;
}


bool RenderDevice::RenderConvertSprite(DirectX11Texture2D* texture, DirectX11Texture2D* target, DirectX11Shader* pShader, uint32_t width, uint32_t height){

    D3D11_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D11_VIEWPORT));
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;

    m_deviceContext->OMSetRenderTargets(1, &target->m_renderTarget, NULL);
    m_deviceContext->RSSetViewports(1, &vp);

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_deviceContext->ClearRenderTargetView(target->m_renderTarget, clearColor);

    float width_i = 1.0f / (float)width;

    vec4 vec0 = {0.256788194f,0.504129171f ,0.0979057774f ,0.0627449304f};
    vec4 vec1 = { -0.148222953f, -0.290992796f,0.439215750f,0.501961052f};
    vec4 vec2 = { 0.439215571f, -0.367788255f, -0.0714273080f, 0.501960695f};

    pShader->VertexShaderLoad();
    pShader->PixelShaderLoad();

    float points[4][4];
    memset(points, 0, sizeof(points));

    float uv[4][2];
    memset(uv,0,sizeof(uv));

    m_spriteVertexBuffer->SetVertexData(points, sizeof(float) * 4 * 4, VertexType::VertexType_Point);
    m_spriteVertexBuffer->SetTexUVData(m_screenCaptureUV, sizeof(float) * 4 * 2, 1);
    m_spriteVertexBuffer->FlushVertexBuffer();

    pShader->LoadVertexBufferData(m_spriteVertexBuffer);
    pShader->UpdateTextureResources(texture,0);

    if (pShader->m_shaderType == DirectX11Shader_RGBA2I420Y) {
        pShader->UpdatePixelConstantBuffer(vec0);
    }

    if (pShader->m_shaderType == DirectX11Shader_RGBA2I420U) {
        pShader->UpdateVertexConstantBuffer(width_i);
        pShader->UpdatePixelConstantBuffer(vec1);
    }

    if (pShader->m_shaderType == DirectX11Shader_RGBA2I420V) {
        pShader->UpdateVertexConstantBuffer(width_i);
        pShader->UpdatePixelConstantBuffer(vec2);
    }

    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->Draw(3, 0);
    return true;
}

bool RenderDevice::RenderDupTexToWindow(DirectX11Texture2D* tex,uint32_t cx,uint32_t cy){
    if (!m_swapChain)
        return false;

    D3D11_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D11_VIEWPORT));
    vp.Width = m_swapChain->m_width;
    vp.Height = m_swapChain->m_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;

    m_deviceContext->OMSetRenderTargets(1, &m_swapChain->m_target->m_renderTarget, NULL);
    m_deviceContext->RSSetViewports(1, &vp);

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_deviceContext->ClearRenderTargetView(m_swapChain->m_target->m_renderTarget, clearColor);

    mat4x4_ortho(m_matCurrentViewProjMatrix,0,tex->m_width, tex->m_height,0,-1,1);
    mat4x4_identity(m_matCurrentModelMatrix);

    RenderSprite(tex,tex->m_width,tex->m_height);
    
    m_swapChain->m_swapChain->Present(0,0);

    return true;
}

bool RenderDevice::RenderDupTexToTarget(ID3D11Texture2D* texture,bool copy,bool convert){
    if (!m_device || !m_deviceContext)
        return false;

    D3D11_TEXTURE2D_DESC dd;
    texture->GetDesc(&dd);

    DirectX11Texture2D* texCopy = GetTextureFromCache(dd.Width, dd.Height, dd.Format, 0, m_TexTempList);
    m_deviceContext->CopyResource(texCopy->m_texture, texture);

    D3D11_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D11_VIEWPORT));
    vp.Width = m_renderTargetWidth;
    vp.Height = m_renderTargetHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;

    m_deviceContext->OMSetRenderTargets(1, &m_currentTarget->m_renderTarget, NULL);
    m_deviceContext->RSSetViewports(1, &vp);
    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_deviceContext->ClearRenderTargetView(m_currentTarget->m_renderTarget, clearColor);
    
    mat4x4_ortho(m_matCurrentViewProjMatrix, 0, texCopy->m_width, texCopy->m_height,0, -100, 100);
    mat4x4_identity(m_matCurrentModelMatrix);
    RenderSubSprite(texCopy, 0, 0, texCopy->m_width, texCopy->m_height);
    RenderCursor();

    if (convert) {
        RenderConvertSprite(m_currentTarget, m_convertTargetYUV[0], m_rgbToI420ShaderY, m_convertTargetYUV[0]->m_width, m_convertTargetYUV[0]->m_height);
        RenderConvertSprite(m_currentTarget, m_convertTargetYUV[1], m_rgbToI420ShaderU, m_convertTargetYUV[1]->m_width, m_convertTargetYUV[1]->m_height);
        RenderConvertSprite(m_currentTarget, m_convertTargetYUV[2], m_rgbToI420ShaderV, m_convertTargetYUV[2]->m_width, m_convertTargetYUV[2]->m_height);

        m_deviceContext->CopyResource(m_convertSurface[m_convertSurfaceIndex][0]->m_texture, m_convertTargetYUV[0]->m_texture);
        m_deviceContext->CopyResource(m_convertSurface[m_convertSurfaceIndex][1]->m_texture, m_convertTargetYUV[1]->m_texture);
        m_deviceContext->CopyResource(m_convertSurface[m_convertSurfaceIndex][2]->m_texture, m_convertTargetYUV[2]->m_texture);
        m_convertSurfaceIndex = ++m_convertSurfaceIndex % 2;
    }

    if (copy){
        m_deviceContext->CopyResource(m_currentSurface[m_surfaceIndex]->m_texture,m_currentTarget->m_texture);
        m_surfaceIndex = (++m_surfaceIndex) % 2;
        return true;
    }

    return true;
}

void RenderDevice::GetConvertCurfaceYUV(DirectX11Surface2D** Y, DirectX11Surface2D** U, DirectX11Surface2D** V){
   
    *Y = m_convertSurface[m_convertSurfaceIndex][0];
    *U = m_convertSurface[m_convertSurfaceIndex][1];
    *V = m_convertSurface[m_convertSurfaceIndex][2];

    return;
}

DirectX11Surface2D* RenderDevice::GetCurrentSurface(){
    DirectX11Surface2D* ret = NULL;
   
    if (m_currentSurface[m_surfaceIndex]){
        return m_currentSurface[m_surfaceIndex];
    }
    return NULL;
}

DirectX11Texture2D* RenderDevice::GetCurrentTarget(bool bCombine) {
    if (bCombine) {
        if (m_currentCombineTarget)
            return m_currentCombineTarget;
        else
            return NULL;
    }
    if (m_currentTarget)
        return m_currentTarget;

    return NULL;
}


void RenderDevice::RenderDeviceShutdown(){
    
    if (m_rgbRenderShader){
        delete m_rgbRenderShader;
        m_rgbRenderShader = NULL;
    }

    if (m_rgbToI420ShaderY){
        delete m_rgbToI420ShaderY;
        m_rgbToI420ShaderY = NULL;
    }

    if (m_rgbToI420ShaderU){
        delete m_rgbToI420ShaderU;
        m_rgbToI420ShaderU = NULL;
    }

    if (m_rgbToI420ShaderV){
        delete m_rgbToI420ShaderV;
        m_rgbToI420ShaderV = NULL;
    }

    if (m_spriteVertexBuffer){
        delete m_spriteVertexBuffer;
        m_spriteVertexBuffer = NULL;
    }

    if (m_currentBlendState) {
        m_currentBlendState->Release();
        m_currentBlendState = NULL;
    }


    for (ArrayTexture::iterator iter = m_TexTargetList.begin(); iter != m_TexTargetList.end(); ++iter)
        delete (*iter);
    
    m_TexTargetList.clear();

    for (ArrayTexture::iterator iter = m_TexTempList.begin(); iter != m_TexTempList.end(); ++iter)
        delete (*iter);
   
    m_TexTempList.clear();

    for (ArrayTexture::iterator iter = m_TexCursorList.begin(); iter != m_TexCursorList.end(); ++iter)
        delete (*iter);

    m_TexCursorList.clear();

    if (m_convertTargetYUV[0]) {
        delete m_convertTargetYUV[0];
        m_convertTargetYUV[0] = NULL;
    }

    if (m_convertTargetYUV[1]) {
        delete m_convertTargetYUV[1];
        m_convertTargetYUV[1] = NULL;
    }
    if (m_convertTargetYUV[2]) {
        delete m_convertTargetYUV[2];
        m_convertTargetYUV[2] = NULL;
    }

    if (m_currentSurface[0]) {
        delete m_currentSurface[0];
        m_currentSurface[0] = NULL;
    }
    if (m_currentSurface[1]) {
        delete m_currentSurface[1];
        m_currentSurface[1] = NULL;
    }

    for (uint32_t i = 0; i < 2; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            if (m_convertSurface[i][j]) {
                delete m_convertSurface[i][j];
                m_convertSurface[i][j] = NULL;
            }
        }
    }

    if (m_mutexDevice.LockSemaphore){
        DeleteCriticalSection(&m_mutexDevice);
        memset(&m_mutexDevice, 0, sizeof(m_mutexDevice));
    }

    m_bInitial = false;
    return;
}

void RenderDevice::DeviceEnterContext(){
    EnterCriticalSection(&m_mutexDevice);
}


void RenderDevice::DeviceLeaveContext(){
    LeaveCriticalSection(&m_mutexDevice);
}



uint8_t* RenderDevice::GetBitmapData(HBITMAP hbmp, BITMAP* bmp){
    if (GetObject(hbmp, sizeof(*bmp), bmp) != 0) {
        uint8_t* output;
        unsigned int size =
            (bmp->bmHeight * bmp->bmWidth * bmp->bmBitsPixel) / 8;

        output = (uint8_t*)malloc(size*2);
        GetBitmapBits(hbmp, size, output);
        return output;
    }

    return NULL;
}

uint8_t  RenderDevice::BitToAlpha(uint8_t* data, long pixel, bool invert){
    uint8_t pix_byte = data[pixel / 8];
    bool alpha = (pix_byte >> (7 - pixel % 8) & 1) != 0;

    if (invert) {
        return alpha ? 0xFF : 0;
    }
    else {
        return alpha ? 0 : 0xFF;
    }
}

bool RenderDevice::BitmapHasAlpha(uint8_t* data, long num_pixels){
    for (long i = 0; i < num_pixels; i++) {
        if (data[i * 4 + 3] != 0) {
            return true;
        }
    }

    return false;
}

void RenderDevice::ApplyMask(uint8_t* color, uint8_t* mask, long numPixels){
    for (long i = 0; i < numPixels; i++)
        color[i * 4 + 3] = BitToAlpha(mask, i, false);
}


uint8_t* RenderDevice::CopyFromColor(ICONINFO* ii, uint32_t* width, uint32_t* height){
    BITMAP bmp_color;
    BITMAP bmp_mask;
    uint8_t* color;
    uint8_t* mask;

    color = GetBitmapData(ii->hbmColor, &bmp_color);
    if (!color) {
        return NULL;
    }

    if (bmp_color.bmBitsPixel < 32) {
        free(color);
        return NULL;
    }

    mask = GetBitmapData(ii->hbmMask, &bmp_mask);
    if (mask) {
        long pixels = bmp_color.bmHeight * bmp_color.bmWidth;

        if (!BitmapHasAlpha(color, pixels))
            ApplyMask(color, mask, pixels);

        free(mask);
    }

    *width = bmp_color.bmWidth;
    *height = bmp_color.bmHeight;
    return color;
}

uint8_t* RenderDevice::CopyFromMask(ICONINFO* ii, uint32_t* width, uint32_t* height){
    uint8_t* output;
    uint8_t* mask;
    long pixels;
    long bottom;
    BITMAP bmp;

    mask = GetBitmapData(ii->hbmMask, &bmp);
    if (!mask) {
        return NULL;
    }

    bmp.bmHeight /= 2;

    pixels = bmp.bmHeight * bmp.bmWidth;
    output = (uint8_t*)malloc(pixels * 4 * 2);
    if (output)
        memset(output,0, pixels * 4 * 2);
    bottom = bmp.bmWidthBytes * bmp.bmHeight;

    for (long i = 0; i < pixels; i++) {
        uint8_t alpha = BitToAlpha(mask, i, false);
        uint8_t color = BitToAlpha(mask + bottom, i, true);

        if (!alpha) {
            output[i * 4 + 3] = color;
        }
        else {
            *(uint32_t*)&output[i * 4] = !!color ? 0xFFFFFFFF
                : 0xFF000000;
        }
    }

    free(mask);

    *width = bmp.bmWidth;
    *height = bmp.bmHeight;
    return output;
}

uint8_t* RenderDevice::CursorCaptureIconBitmap(ICONINFO* ii, uint32_t* width, uint32_t* height){
    uint8_t* output;

    output = CopyFromColor(ii, width, height);
    if (!output)
        output = CopyFromMask(ii, width, height);

    return output;
}

bool RenderDevice::CursorCaptureICon(cursor_data* data, HICON icon){
    uint8_t* bitmap;
    uint32_t height;
    uint32_t width;
    ICONINFO ii;

    if (!icon) {
        return false;
    }
    if (!GetIconInfo(icon, &ii)) {
        return false;
    }

    bitmap = CursorCaptureIconBitmap(&ii, &width, &height);
    if (bitmap) {
        if (data->last_cx != width || data->last_cy != height) {
            data->texture = GetTextureFromCache(width, height,DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM,FLAG_DYNAMIC,
                m_TexCursorList);
            data->last_cx = width;
            data->last_cy = height;
        }
        data->texture->TextureSetImage( bitmap, width * 4, false);
        free(bitmap);

        data->x_hotspot = ii.xHotspot;
        data->y_hotspot = ii.yHotspot;
    }

    DeleteObject(ii.hbmColor);
    DeleteObject(ii.hbmMask);
    return !!data->texture;
}


void RenderDevice::CursorCapture(cursor_data *data){
    CURSORINFO ci = { 0 };
    HICON icon;

    ci.cbSize = sizeof(ci);

    if (!GetCursorInfo(&ci)) {
        data->visible = false;
        return;
    }

    memcpy(&data->cursor_pos, &ci.ptScreenPos, sizeof(data->cursor_pos));

    if (data->current_cursor == ci.hCursor) {
        return;
    }

    icon = CopyIcon(ci.hCursor);
    data->visible = CursorCaptureICon(data,icon);
    data->current_cursor = ci.hCursor;
    if ((ci.flags & CURSOR_SHOWING) == 0)
        data->visible = false;
    DestroyIcon(icon);
}



