#pragma once

#include <stdio.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3d11_1.h>
#include <vector>
#include <string>

#include "linmath.h"
#define RAD(val) ((val)*0.0174532925199432957692369076848f)

#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

//from obs default shader  "#pragma pack_matrix(row_major)\n" #pragma pack_matrix(col_major)
static const char* vertexshaderRGBA =
"uniform float4x4 ViewProj;\n"
"struct VertInOut {\n"
"	float4 pos : SV_Position;\n"
"	float2 uv : TEXCOORD0;\n"
"};\n"
"VertInOut VSDefault(VertInOut vert_in)\n"
"{\n"
"	VertInOut vert_out;\n"
"	vert_out.pos = mul(float4(vert_in.pos.xyz, 1.0), ViewProj);\n"
"	vert_out.uv  = vert_in.uv;\n"
"	return vert_out;\n"
"}\n"
"VertInOut main(VertInOut vert_in,VertInOut vert_in2)\n"
"{\n"
"	return VSDefault(vert_in);\n"
"}\n";


static const char* pixelshaderRGBA =
"uniform Texture2D image;\n"
"SamplerState def_sampler{\n"
"	Filter = Linear;\n"
"	AddressU = Clamp;\n"
"	AddressV = Clamp;\n"
"};\n"
"struct VertInOut {\n"
"	float4 pos : SV_Position;\n"
"	float2 uv : TEXCOORD0;\n"
"};\n"
"float4 PSDrawBare(VertInOut vert_in)\n"
"{\n"
"	return image.Sample(def_sampler, vert_in.uv);\n"
"}\n"
"float4 main(VertInOut vert_in) : SV_Target\n"
"{\n"
"	return PSDrawBare(vert_in);\n"
"}\n";

// from obs RGB(BGR) To I420 Shader
static const char* vertexShaderRGBAToI420Y =
"struct FragPos {\n"
"	float4 pos : SV_Position;\n"
"};\n"

"FragPos VSPos(uint id : SV_VertexID)\n"
"{\n"
"	float idHigh = float(id >> 1);\n"
"	float idLow = float(id & uint(1));\n"
"	float x = idHigh * 4.0 - 1.0;\n"
"	float y = idLow * 4.0 - 1.0;\n"
"	FragPos vert_out;\n"
"	vert_out.pos = float4(x, y, 0.0, 1.0);\n"
"	return vert_out;\n"
"}\n"
"FragPos main(uint id : SV_VertexID)\n"
"{\n"
"	return VSPos(id);\n"
"}\n"
;

static const char* pixelShaderRGBAToI420Y =
"uniform Texture2D image; \n"
"uniform float4 color_vec0; \n"
"struct FragPos {\n"
"	float4 pos : SV_Position;\n"
"};\n"

"float PS_Y(FragPos frag_in)\n"
"{\n"
"	float3 rgb = image.Load(int3(frag_in.pos.xy, 0)).rgb;\n"
"	float y = dot(color_vec0.xyz, rgb) + color_vec0.w;\n"
"	return y;\n"
"}\n"
"float main(FragPos frag_in) : SV_Target\n"
"{\n"
"	return PS_Y(frag_in);\n"
"}\n"
;


//Plane_U_Left
static const char* vertexShaderRGBAToI420UV = 
"uniform float width_i;\n"

"struct VertTexPosWide {\n"
"	float3 uuv : TEXCOORD0;\n"
"	float4 pos : SV_Position;\n"
"};\n"

"VertTexPosWide VSTexPos_Left(uint id : SV_VertexID)\n"
"{\n"
"	float idHigh = float(id >> 1);\n"
"	float idLow = float(id & uint(1));\n"

"	float x = idHigh * 4.0 - 1.0;\n"
"	float y = idLow * 4.0 - 1.0;\n;"

"	float u_right = idHigh * 2.0;\n"
"	float u_left = u_right - width_i;\n"
"	float v =  (1.0 - idLow * 2.0);\n"

"	VertTexPosWide vert_out;\n"
"	vert_out.uuv = float3(u_left, u_right, v);\n"
"	vert_out.pos = float4(x, y, 0.0, 1.0);\n"
"	return vert_out;\n"
"}\n"

"VertTexPosWide main(uint id : SV_VertexID)\n"
"{\n"
"	return VSTexPos_Left(id);\n"
"}\n"
;

static const char* pixelShaderRGBAToI420U =
"uniform Texture2D image;\n"
"uniform float4 color_vec1;\n"

"SamplerState def_sampler{\n"
"	Filter = Linear;\n"

"	AddressU = Clamp;\n"

"	AddressV = Clamp;\n"

"};\n"

"struct FragTexWide {\n"
"	float3 uuv : TEXCOORD0;\n"
"};\n"

"float PS_U_Wide(FragTexWide frag_in)\n"
"{\n"
"	float3 rgb_left = image.Sample(def_sampler, frag_in.uuv.xz).rgb;\n"
"	float3 rgb_right = image.Sample(def_sampler, frag_in.uuv.yz).rgb;\n"
"	float3 rgb = (rgb_left + rgb_right) * 0.5;\n"
"	float u = dot(color_vec1.xyz, rgb) + color_vec1.w;\n"
"	return u;\n"
"}\n"

"float main(FragTexWide frag_in) : SV_Target\n"
"{\n"
"	return PS_U_Wide(frag_in);\n"
"}\n"
;


static const char* pixelShaderRGBAToI420V =
"uniform Texture2D image;\n"
"uniform float4 color_vec2;\n"
"SamplerState def_sampler{\n"
"	Filter = Linear;\n"

"	AddressU = Clamp;\n"

"	AddressV = Clamp;\n"

"};\n"

"struct FragTexWide {\n"
"	float3 uuv : TEXCOORD0;\n"
"};\n"

"float PS_V_Wide(FragTexWide frag_in)\n"
"{\n"
"	float3 rgb_left = image.Sample(def_sampler, frag_in.uuv.xz).rgb;\n"
"	float3 rgb_right = image.Sample(def_sampler, frag_in.uuv.yz).rgb;\n"
"	float3 rgb = (rgb_left + rgb_right) * 0.5;\n"
"	float v = dot(color_vec2.xyz, rgb) + color_vec2.w;\n"
"	return v;\n"
"}\n"

"float main(FragTexWide frag_in) : SV_Target\n"
"{\n"
"	return PS_V_Wide(frag_in);\n"
"}\n"
;

#define FLAG_BUILD_MIPMAPS (1 << 0)
#define FLAG_DYNAMIC (1 << 1)
#define FLAG_RENDER_TARGET (1 << 2)

enum CombDirection {
	Combine_NULL = -1,
	Combine_UpDown,
	Combine_DownUp,
	Combine_LeftRight,
	Combine_RightLeft
};

class RenderDevice;
class DirectX11Texture2D;
class DirectX11Surface2D;
class DirectX11VertexBuffer;
class DirectX11SwainChain;
class DirectX11Shader;


typedef struct DirectX11Rect
{
	int x;
	int y;
	int w;
	int h;

}DirectX11Rect;

class DirectX11Texture2D
{
public:
	DirectX11Texture2D(const std::string& name, RenderDevice* render);
	~DirectX11Texture2D();

	bool createTexture2D(int width, int height, unsigned flag, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, bool gdi = false);
	void TextureSetImage(const uint8_t *data,uint32_t linesize,bool flip);
	bool createTexture2DFromSurface(DirectX11Surface2D* surface);

private:
	void destroyTexture2D();
	bool TextureMap(uint8_t** ptr, uint32_t* linesize);
	void TextureUnMap();

public:
	std::string m_name = "1280x720_87";
	RenderDevice* m_renderDevice = NULL;
	bool bGDICompatible = false;
	DXGI_FORMAT m_dxgiFormat = DXGI_FORMAT_UNKNOWN;
	bool bRenderTarget = false;
	bool bDynamic = false;
	int m_width = 1280;
	int m_height = 720;
	int m_flag = FLAG_DYNAMIC;
	ID3D11ShaderResourceView* m_shaderResView = NULL;
	ID3D11Texture2D* m_texture = NULL;
	ID3D11RenderTargetView* m_renderTarget = NULL;
	IDXGISurface1* m_gdiSurface = NULL;
};

class DirectX11Surface2D {
public:
	DirectX11Surface2D(const std::string& name, RenderDevice* render);
	~DirectX11Surface2D();
	bool createSurface2D(int width, int height, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM);

	bool DownLoadSurfaceData();
	void FreeSurfaceData();

	bool DownLoadSurfaceData(uint8_t* data,int linesize);

private:
	void destroySurface2D();
	bool DirectX11StageSurfaceMap(uint8_t** data,uint32_t* linesize);
	void DirectX11StageSurfaceUnmap();

public:
	int m_width = 1280;
	int m_height = 720;
	std::string      m_name = "1280x720";
	RenderDevice* m_renderDevice = NULL;
	DXGI_FORMAT      m_dxgiFormat = DXGI_FORMAT_UNKNOWN;
	ID3D11Texture2D* m_texture = NULL;
    uint8_t*         m_data;
	uint32_t         m_linesize = 0;

};

// 纹理坐标数组
typedef struct TvertArray {
	size_t texUVElementSize; 
	float* data;
	size_t num;
}TvertArray;


typedef struct VertexData
{
	size_t    num;
	float* points;  //4*4*sizeof(float)
	size_t pointsElementSize;

	float* normals; //4*4*sizeof(float)
	size_t normalsElementSize;

	float* tangents;//
	size_t tangentsElementSize;

	uint32_t* colors;
	size_t colorsElementSize;

	size_t    num_tex;// 纹理的个数
	TvertArray* tvarray;
}VertexData;


typedef enum VertexType
{
    VertexType_NULL,
	VertexType_Point,
	VertexType_Normal,
	VertexType_Tangent,
	VertexType_Color
}VertexType;

class DirectX11VertexBuffer
{
public:
	DirectX11VertexBuffer(RenderDevice* device);
	~DirectX11VertexBuffer();
	bool createSpriteBuffer();
	void FlushVertexBuffer();
	void MakeBufferList(DirectX11Shader* shader, std::vector<ID3D11Buffer*>& buffers, std::vector<uint32_t>& strides);
	void SetVertexData(void* data,size_t size,VertexType type);
	void SetTexUVData(void* data, size_t sizePerUVMatrix, uint32_t num);
	
private:
	//elementSize:每个顶点元素的大小一般为sizeof(float)*4
	//numVertex:顶点的个数
	void InitBuffer(const size_t elementSize,const size_t numVertex,void* data,ID3D11Buffer** buffer);
	bool createVertexBuffer(VertexData* data, bool bDynamic = true);
	void destroyVertexBuffer();
	void freeVertexData(VertexData* data);
	void FlushVertexBuffer(VertexData* vertex);
	void FlushBuffer(ID3D11Buffer* buffer,void* ptr,size_t elementSize,size_t num);
	void PushBuffer(std::vector<ID3D11Buffer*>& buffers, std::vector<uint32_t>& strides, ID3D11Buffer* buffer, size_t elementSize, const char* name);
public:
	ID3D11Buffer* m_vertexBuffer = NULL;

	VertexData   *m_vertexData = NULL;
	RenderDevice* m_renderDevice = NULL;
	std::vector<ID3D11Buffer* > m_uvBuffers;
	std::vector<size_t> m_uvSizes;

	ID3D11Buffer* m_normalBuffer = NULL;
	ID3D11Buffer* m_colorBuffer = NULL;
	ID3D11Buffer* m_tangentBuffer = NULL;
	bool          m_bDynamic = true;
};

typedef struct  DirectX11SwainInfo
{
	void* hwnd;
	uint32_t cx, cy;
	DXGI_FORMAT format;
	uint32_t numBackbuffers;
}DirectX11SwainInfo;

class DirectX11SwainChain
{
public:
	DirectX11SwainChain(const std::string& str,RenderDevice* device);
	~DirectX11SwainChain();

	bool CreateSwainChain(DirectX11SwainInfo *info);
	bool ResizeSwapChain(uint32_t cx, uint32_t cy);

private:
	void destroySwainChain();
	bool InitTarget(uint32_t cx, uint32_t cy);
public:
	DirectX11Texture2D* m_target;
	RenderDevice* m_device = NULL;
	IDXGISwapChain* m_swapChain = NULL;
	HWND  m_hwnd = NULL;
	DXGI_FORMAT m_format;
	int m_numBackbuffers = 1;
	std::string m_name;
	uint32_t m_width;
	uint32_t m_height;
};


typedef enum DirectX11ShaderType
{
	DirectX11Shader_NULL = 0,      //
	DirectX11Shader_Solid,         //solid rect obs          not  add
	DirectX11Shader_SolidColored,  //solid rect colored obs  not  add
	DirectX11Shader_RGBA,          //image render obs          add
	DirectX11Shader_RGBA2I420Y,    //I420 convert obs          add
	DirectX11Shader_RGBA2I420U,    //I420 convert obs          add
	DirectX11Shader_RGBA2I420V,    //I420 convert obs          add
	DirectX11Shader_RGBA2NV12      //NV12 convert obs        not  add
}DirectX11ShaderType;


class DirectX11Shader 
{
public:	
	DirectX11Shader(const std::string& str, RenderDevice* device);
	~DirectX11Shader();

	bool CreateShader(const char* vs,const char *ps, DirectX11ShaderType type);
	virtual void BuildLayout();

	void VertexShaderLoad();
	void PixelShaderLoad();
	void UpdateVertexConstantBuffer(mat4x4 matrix);
	void UpdatePixelConstantBuffer(vec4 vec);
	void UpdateVertexConstantBuffer(float width);

	void UpdateTextureResources(DirectX11Texture2D* tex,uint32_t index);
	void LoadVertexBufferData(DirectX11VertexBuffer *vertexBuffer);

private:
	bool destroyShader();
public:
	RenderDevice*          m_renderDevice = NULL;
	std::string            m_name;
	ID3D11Buffer*          m_constantVSBuffer = NULL;
	ID3D11InputLayout*     m_inputLayout = NULL;
	ID3D11VertexShader*    m_vertexShader = NULL;
	ID3D11Buffer*          m_constantPSBuffer = NULL;
	ID3D11PixelShader*     m_pixelShader = NULL;
	ID3D11SamplerState*    m_samplerState[8] = {0};
	DirectX11ShaderType    m_shaderType = DirectX11Shader_NULL;
	
	bool                   m_hasNormals = false;
	bool                   m_hasColors = false;
	bool                   m_hasTangents = false;
	uint32_t               m_nTexUnits = 0;
};


class RenderDevice
{
public:
	RenderDevice(DirectX11Rect *rect);
	~RenderDevice();

	bool isInit();
	bool RenderDeviceInit(ID3D11Device* device, ID3D11DeviceContext* device_context);
	
	bool RenderDupTexToWindow(DirectX11Texture2D* tex,uint32_t cx, uint32_t cy);
	void RenderDeviceShutdown();
	void SetCaptureRect(float posX, float posY, float sub_cx, float sub_cy);
	bool SetTargetParam(int width,int height, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM,bool bGdi = false);
	bool RenderDupTexToTarget(ID3D11Texture2D* texture, bool copyToSurface = false, bool convert = false);

	bool RenderCombine(DirectX11Texture2D* tex1,DirectX11Texture2D* tex2, float combineRatio, CombDirection flag);
	bool RenderCombine(DirectX11Texture2D* tex1, DirectX11Surface2D* surface, float combineRatio, CombDirection flag);

	void GetConvertCurfaceYUV(DirectX11Surface2D** YPlane,DirectX11Surface2D** UPlane,DirectX11Surface2D** VPlane);

	DirectX11Texture2D* GetCurrentTarget(bool bCombine = false);
	DirectX11Surface2D* GetCurrentSurface();
	

	//Direct
	void DeviceEnterContext();
	void DeviceLeaveContext();
	bool CreateSwainChain(HWND hwnd, uint32_t cx, uint32_t cy);

	typedef struct Backup_D3D11_State {
		int ViewPortCount;
		D3D11_VIEWPORT Viewports[16];
		D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
		ID3D11SamplerState* PSSamplerState;
		ID3D11VertexShader* VS;
		ID3D11PixelShader* PS;
		ID3D11Buffer* VertexBuffer;
		ID3D11Buffer* VSConstantBuffer;
		ID3D11InputLayout* InputLayout;
		ID3D11ShaderResourceView* PSShaderResource;
		ID3D11BlendState* blendstate;
	}Backup_D3D11_State;

	typedef struct cursor_data {
		DirectX11Texture2D* texture;
		HCURSOR current_cursor;
		POINT cursor_pos;
		long x_hotspot;
		long y_hotspot;
		bool visible;

		uint32_t last_cx;
		uint32_t last_cy;
	}cursor_data;

	typedef std::vector<DirectX11Texture2D*> ArrayTexture;
	typedef std::vector<DirectX11Surface2D*> ArraySurface;

private:
	bool RenderCursor();
	//Float4X4 UpdateViewProjMatrix(float left, float right, float top,float bottom, float zNear = 100.0f, float zFar = -100.0f);

	DirectX11Texture2D* GetTextureFromCache(uint32_t cx, uint32_t cy, DXGI_FORMAT format, int flag, ArrayTexture& list,bool bGdi = false);
	DirectX11Surface2D* GetSurfaceFromCache(uint32_t cx, uint32_t cy, DXGI_FORMAT format,ArraySurface& list);

	bool RenderSprite(DirectX11Texture2D* texture,uint32_t width,uint32_t height);
	bool RenderSubSprite(DirectX11Texture2D* texture,uint32_t sub_x,uint32_t sub_y,uint32_t sub_cx,uint32_t sub_cy, bool isScreenCap = true);
	bool PointInRect(int posx, int posy, DirectX11Rect* r);
	bool RenderConvertSprite(DirectX11Texture2D* texture,  DirectX11Texture2D* target,DirectX11Shader* pShader,  uint32_t width,uint32_t height);

	// Capture Cursor from OBS
	uint8_t* GetBitmapData(HBITMAP hbmp, BITMAP* bmp);
	uint8_t  BitToAlpha(uint8_t* data, long pixel, bool invert);
	bool     BitmapHasAlpha(uint8_t* data, long num_pixels);
	void     ApplyMask(uint8_t* color,uint8_t* mask,long numPixels);
	uint8_t* CopyFromColor(ICONINFO* ii, uint32_t* width,uint32_t* height);
	uint8_t* CopyFromMask(ICONINFO* ii, uint32_t* width,uint32_t* height);
	uint8_t* CursorCaptureIconBitmap(ICONINFO* ii, uint32_t* width, uint32_t* height);
	bool CursorCaptureICon(cursor_data* data, HICON icon);
	void CursorCapture(cursor_data* data);
	
public:
	ID3D11Device*        m_device = NULL;
	ID3D11DeviceContext* m_deviceContext = NULL;
	IDXGIFactory*        m_factory = NULL;

private:
	//curent state 

	DirectX11Shader* m_rgbRenderShader = NULL;
	DirectX11Shader* m_rgbToI420ShaderY = NULL;
	DirectX11Shader* m_rgbToI420ShaderU = NULL;
	DirectX11Shader* m_rgbToI420ShaderV = NULL;

	DirectX11VertexBuffer* m_spriteVertexBuffer = NULL;
	

	int m_renderTargetWidth = 1280;
	int m_renderTargetHeight = 720;

	DirectX11Rect m_captureRect;

	DirectX11Texture2D* m_currentTarget = NULL;  //BGRA
	DirectX11Texture2D* m_currentTargetRGBA = NULL;
	DirectX11Texture2D* m_currentCombineTarget = NULL;
	
	//ConvertTarget 
	DirectX11Texture2D* m_convertTargetYUV[3]   = { NULL,NULL,NULL };
	DirectX11Surface2D* m_convertSurface[2][3]     = { NULL };
	uint32_t            m_convertSurfaceIndex = 0;

	DirectX11Surface2D* m_currentSurface[2] = {NULL, NULL};
	uint32_t            m_surfaceIndex = 0;


	ArrayTexture m_TexTargetList;
	ArrayTexture m_TexTempList;  // 临时Tex列表
	ArrayTexture m_TexCombineList;

	Backup_D3D11_State m_oldState;
	ID3D11BlendState* m_currentBlendState = NULL;

	// 鼠标纹理
	DirectX11Texture2D* m_curCursorTex = NULL;
	ArrayTexture  m_TexCursorList;

	CRITICAL_SECTION m_mutexDevice;

	mat4x4    m_matCurrentModelMatrix;
	mat4x4    m_matCurrentViewProjMatrix;

	float     m_screenCaptureUV[4][2];

	bool      m_bInitial = false;

	cursor_data   m_cursorData;
	DirectX11Rect m_screenRect;

	HWND m_hwnd = NULL;
	DirectX11SwainChain* m_swapChain = NULL;
};