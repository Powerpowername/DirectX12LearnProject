#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <wrl.h>
#include <string>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <memory>
#include "directx/d3dx12.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")				// 链接 DXGI DLL
#pragma comment(lib,"dxguid.lib")			// 链接 DXGI 必要的设备 GUID
#pragma comment(lib,"d3dcompiler.lib")		// 链接 DX12 需要的着色器编译 DLL
#pragma comment(lib,"windowscodecs.lib")	// 链接 WIC DLL

using namespace Microsoft;
using namespace Microsoft::WRL;		// 使用 wrl.h 里面的命名空间，我们需要用到里面的 Microsoft::WRL::ComPtr COM智能指针
using namespace DirectX;			// DirectX 命名空间
// 命名空间 DX12TextureHelper 包含了帮助我们转换纹理图片格式的结构体与函数
namespace DX12TextureHelper
{
	// 纹理转换用，不是 DX12 所支持的格式，DX12 没法用

	// Standard GUID -> DXGI 格式转换结构体
	struct WICTranslate
	{
		GUID wic;
		DXGI_FORMAT format;
	};

	// WIC 格式与 DXGI 像素格式的对应表，该表中的格式为被支持的格式
	static WICTranslate g_WICFormats[] =
	{
		{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },
		{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },
		{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },
		{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
		{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },
		{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
		{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
		{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
		{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },
		{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM }
	};

	// GUID -> Standard GUID 格式转换结构体
	struct WICConvert
	{
		GUID source;
		GUID target;
	};

	// WIC 像素格式转换表
	static WICConvert g_WICConvert[] =
	{
		// 目标格式一定是最接近的被支持的格式
		{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf },		// DXGI_FORMAT_R16_FLOAT
		{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat },	// DXGI_FORMAT_R32_FLOAT
		{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 },		// DXGI_FORMAT_B5G5R5A1_UNORM
		{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 },	// DXGI_FORMAT_R10G10B10A2_UNORM
		{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT

		{ GUID_WICPixelFormat128bppRGBAFloat,       GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat64bppRGBAHalf,         GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBA,             GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat32bppRGBA,             GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat32bppBGRA,             GUID_WICPixelFormat32bppBGRA },			// DXGI_FORMAT_B8G8R8A8_UNORM
		{ GUID_WICPixelFormat32bppBGR,              GUID_WICPixelFormat32bppBGR },			// DXGI_FORMAT_B8G8R8X8_UNORM
		{ GUID_WICPixelFormat32bppRGBA1010102XR,    GUID_WICPixelFormat32bppRGBA1010102XR },// DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
		{ GUID_WICPixelFormat32bppRGBA1010102,      GUID_WICPixelFormat32bppRGBA1010102 },	// DXGI_FORMAT_R10G10B10A2_UNORM
		{ GUID_WICPixelFormat16bppBGRA5551,         GUID_WICPixelFormat16bppBGRA5551 },		// DXGI_FORMAT_B5G5R5A1_UNORM
		{ GUID_WICPixelFormat16bppBGR565,           GUID_WICPixelFormat16bppBGR565 },		// DXGI_FORMAT_B5G6R5_UNORM
		{ GUID_WICPixelFormat32bppGrayFloat,        GUID_WICPixelFormat32bppGrayFloat },	// DXGI_FORMAT_R32_FLOAT
		{ GUID_WICPixelFormat16bppGrayHalf,         GUID_WICPixelFormat16bppGrayHalf },		// DXGI_FORMAT_R16_FLOAT
		{ GUID_WICPixelFormat16bppGray,             GUID_WICPixelFormat16bppGray },			// DXGI_FORMAT_R16_UNORM
		{ GUID_WICPixelFormat8bppGray,              GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat8bppAlpha,             GUID_WICPixelFormat8bppAlpha }			// DXGI_FORMAT_A8_UNORM
	};


	// 查表确定兼容的最接近格式是哪个
	bool GetTargetPixelFormat(const GUID* pSourceFormat, GUID* pTargetFormat)
	{
		*pTargetFormat = *pSourceFormat;
		for (size_t i = 0; i < _countof(g_WICConvert); ++i)
		{
			if (InlineIsEqualGUID(g_WICConvert[i].source, *pSourceFormat))
			{
				*pTargetFormat = g_WICConvert[i].target;
				return true;
			}
		}
		return false;		// 找不到，就返回 false
	}

	// 查表确定最终对应的 DXGI 格式是哪一个
	DXGI_FORMAT GetDXGIFormatFromPixelFormat(const GUID* pPixelFormat)
	{
		for (size_t i = 0; i < _countof(g_WICFormats); ++i)
		{
			if (InlineIsEqualGUID(g_WICFormats[i].wic, *pPixelFormat))
			{
				return g_WICFormats[i].format;
			}
		}
		return DXGI_FORMAT_UNKNOWN;		// 找不到，就返回 UNKNOWN
	}
}

class CallBackWrapper
{
public:
	inline static std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> Broker_Func;
	static LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return Broker_Func(hwnd, msg, wParam, lParam);
	}
};

// 摄像机类
class Camera
{
private:

	XMVECTOR EyePosition = XMVectorSet(4, 3, 4, 1);			// 摄像机在世界空间下的位置
	XMVECTOR FocusPosition = XMVectorSet(0, 1, 1, 1);		// 摄像机在世界空间下观察的焦点位置
	XMVECTOR UpDirection = XMVectorSet(0, 1, 0, 0);			// 世界空间垂直向上的向量

	// 摄像机观察方向的单位向量，用于前后移动
	XMVECTOR ViewDirection = XMVector3Normalize(FocusPosition - EyePosition);

	// 焦距，摄像机原点与焦点的距离，XMVector3Length 表示对向量取模
	float FocalLength = XMVectorGetX(XMVector3Length(FocusPosition - EyePosition));

	// 摄像机向右方向的单位向量，用于左右移动，XMVector3Cross 求两向量叉乘
	XMVECTOR RightDirection = XMVector3Normalize(XMVector3Cross(ViewDirection, UpDirection));

	POINT LastCursorPoint = {};								// 上一次鼠标的位置

	float FovAngleY = XM_PIDIV4;							// 垂直视场角
	float AspectRatio = 4.0 / 3.0;							// 投影窗口宽高比
	float NearZ = 0.1;										// 近平面到原点的距离
	float FarZ = 1000;										// 远平面到原点的距离

	XMMATRIX ModelMatrix;									// 模型矩阵，模型空间 -> 世界空间
	XMMATRIX ViewMatrix;									// 观察矩阵，世界空间 -> 观察空间
	XMMATRIX ProjectionMatrix;								// 投影矩阵，观察空间 -> 齐次裁剪空间

	XMMATRIX MVPMatrix;										// MVP 矩阵，类外需要用公有方法 GetMVPMatrix 获取

public:

	Camera()	// 摄像机的构造函数
	{
		// 模型矩阵，这里我们让模型旋转 30° 就行，注意这里只是一个示例，后文我们会将它移除，每个模型都应该拥有相对独立的模型矩阵
		ModelMatrix = XMMatrixRotationY(30.0f);
		// 观察矩阵，注意前两个参数是点，第三个参数才是向量
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		// 投影矩阵 (注意近平面和远平面距离不能 <= 0!)
		ProjectionMatrix = XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ);
	}

	// 摄像机前后移动，参数 Stride 是移动速度 (步长)，正数向前移动，负数向后移动
	void Walk(float Stride)
	{
		EyePosition += Stride * ViewDirection;
		FocusPosition += Stride * ViewDirection;
	}

	// 摄像机左右移动，参数 Stride 是移动速度 (步长)，正数向左移动，负数向右移动
	void Strafe(float Stride)
	{
		EyePosition += Stride * RightDirection;
		FocusPosition += Stride * RightDirection;
	}

	// 鼠标在屏幕空间 y 轴上移动，相当于摄像机以向右的向量 RightDirection 向上向下旋转，人眼往上下看
	void RotateByY(float angleY)
	{
		// 以向右向量为轴构建旋转矩阵，旋转 ViewDirection 和 UpDirection
		XMMATRIX R = XMMatrixRotationAxis(RightDirection, -angleY);

		UpDirection = XMVector3TransformNormal(UpDirection, R);
		ViewDirection = XMVector3TransformNormal(ViewDirection, R);

		// 利用 ViewDirection 观察向量、FocalLength 焦距，更新焦点位置
		FocusPosition = EyePosition + ViewDirection * FocalLength;
	}

	// 鼠标在屏幕空间 x 轴上移动，相当于摄像机绕世界空间的 y 轴向左向右旋转，人眼往左右看
	void RotateByX(float angleX)
	{
		// 以世界坐标系下的 y 轴 (0,1,0,0) 构建旋转矩阵，三个向量 ViewDirection, UpDirection, RightDirection 都要旋转
		XMMATRIX R = XMMatrixRotationY(angleX);

		UpDirection = XMVector3TransformNormal(UpDirection, R);
		ViewDirection = XMVector3TransformNormal(ViewDirection, R);
		RightDirection = XMVector3TransformNormal(RightDirection, R);

		// 利用 ViewDirection 观察向量、FocalLength 焦距，更新焦点位置
		FocusPosition = EyePosition + ViewDirection * FocalLength;
	}

	// 更新上一次的鼠标位置
	void UpdateLastCursorPos()
	{
		GetCursorPos(&LastCursorPoint);
	}

	// 当鼠标左键长按并移动时，旋转摄像机视角
	void CameraRotate()
	{
		POINT CurrentCursorPoint = {};
		GetCursorPos(&CurrentCursorPoint);	// 获取当前鼠标位置

		// 根据鼠标在屏幕坐标系的 x,y 轴的偏移量，计算摄像机旋转角
		float AngleX = XMConvertToRadians(0.25 * static_cast<float>(CurrentCursorPoint.x - LastCursorPoint.x));
		float AngleY = XMConvertToRadians(0.25 * static_cast<float>(CurrentCursorPoint.y - LastCursorPoint.y));

		// 旋转摄像机
		RotateByY(AngleY);
		RotateByX(AngleX);

		UpdateLastCursorPos();		// 旋转完毕，更新上一次的鼠标位置
	}

	// 更新 MVP 矩阵
	void UpdateMVPMatrix()
	{
		// 主要是更新观察矩阵
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		MVPMatrix = ModelMatrix * ViewMatrix * ProjectionMatrix;
	}

	// 获取 MVP 矩阵
	inline XMMATRIX& GetMVPMatrix()
	{
		// 每次返回前，都更新一次
		UpdateMVPMatrix();
		return MVPMatrix;
	}
};

struct VERTEX
{
	XMFLOAT4 position;			// 顶点在模型坐标系的坐标
	XMFLOAT2 texcoordUV;		// 顶点纹理 UV 坐标
};
// 模型类，这是个抽象类，有两个纯虚函数，派生类需要实现下面两个纯虚函数才能创建实例
class Model
{ 
protected:
	XMMATRIX ModelMatrix = XMMatrixIdentity();
	ComPtr<ID3D12Resource> m_VertexResource;
	ComPtr<ID3D12Resource> m_IndexResource;
	ComPtr<ID3D12Resource> m_ModelMatrixResource;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView[2];
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	// 纹理名 - GPU 句柄映射表，用于索引纹理，设置根参数
	std::unordered_map<std::string, D3D12_GPU_DESCRIPTOR_HANDLE> Texture_GPUHandle_Map;
	void AppendTextureKey(std::string&& TextureName)
	{
		Texture_GPUHandle_Map[TextureName] = {};
	}

public:
	// 类外获取模型需要的纹理，返回映射表的只读引用
	const std::unordered_map<std::string, D3D12_GPU_DESCRIPTOR_HANDLE>& RequestForTextureMap()
	{
		return Texture_GPUHandle_Map;
	}

	// 模型获取类外已经创建纹理 SRV 描述符的 SRVHandle
	void SetTextureGPUHandle(std::string TextureName, D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle)
	{
		Texture_GPUHandle_Map[TextureName] = GPUHandle;
	}

	// 获取模型矩阵
	XMMATRIX GetModelMatrix()
	{
		return ModelMatrix;
	}

	// 设置模型矩阵
	void SetModelMatrix(XMMATRIX Matrix)
	{
		ModelMatrix = Matrix;
	}

	// 创建资源与描述符，这个是纯虚函数，实例类需要实现
	virtual void CreateResourceAndDescriptor(ComPtr<ID3D12Device4>& pD3D12Device) = 0;

	// 绘制模型，这个也是纯虚函数，实例类需要实现
	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) = 0;
};

// 全遮挡固体方块类 (抽象类)，继承自模型类，只定义 CreateResourceAndDescriptor 这个函数，DrawModel 仍然需要派生类实现
class SoildBlock : public Model
{ 
	inline static VERTEX VertexArray[24] =
	{
		// 正面
		{{0,1,0,1},{0,0}},
		{{1,1,0,1},{1,0}},
		{{1,0,0,1},{1,1}},
		{{0,0,0,1},{0,1}},

		// 背面
		{{1,1,1,1},{0,0}},
		{{0,1,1,1},{1,0}},
		{{0,0,1,1},{1,1}},
		{{1,0,1,1},{0,1}},

		// 左面
		{{0,1,1,1},{0,0}},
		{{0,1,0,1},{1,0}},
		{{0,0,0,1},{1,1}},
		{{0,0,1,1},{0,1}},

		// 右面
		{{1,1,0,1},{0,0}},
		{{1,1,1,1},{1,0}},
		{{1,0,1,1},{1,1}},
		{{1,0,0,1},{0,1}},

		// 上面
		{{0,1,1,1},{0,0}},
		{{1,1,1,1},{1,0}},
		{{1,1,0,1},{1,1}},
		{{0,1,0,1},{0,1}},

		// 下面
		{{0,0,0,1},{0,0}},
		{{1,0,0,1},{1,0}},
		{{1,0,1,1},{1,1}},
		{{0,0,1,1},{0,1}}
	};
	// 注意这里的 UINT == UINT32，后面填的格式 (步长) 必须是 DXGI_FORMAT_R32_UINT，否则会出错
	inline static UINT IndexArray[36] =
	{
		// 正面
		0,1,2,0,2,3,
		// 背面
		4,5,6,4,6,7,
		// 左面
		8,9,10,8,10,11,
		// 右面
		12,13,14,12,14,15,
		// 上面
		16,17,18,16,18,19,
		// 下面
		20,21,22,20,22,23
	};


public:
	virtual void CreateResourceAndDescriptor(ComPtr<ID3D12Device4>& pD3D12Device) override
	{
		// 临时设置 XMFLOAT4X4 类型的模型矩阵，XMFLOAT4X4 擅长存储与传递，XMMATRIX 擅长并行运算
		XMFLOAT4X4 _temp_ModelMatrix = {};
		XMStoreFloat4x4(&_temp_ModelMatrix, ModelMatrix);

		// 用于批量复制模型矩阵的 vector，vector 的底层是一块连续内存，memcpy 复制连续内存有 CPU 优化，能快很多
		std::vector<XMFLOAT4X4> _temp_ModelMatrixGroup;
		// 批量填充 ModelMatrix 到 ModelMatrixGroup
		_temp_ModelMatrixGroup.assign(24, _temp_ModelMatrix);

		//创建模型矩阵资源
		CD3DX12_RESOURCE_DESC UploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(XMFLOAT4X4) * 24);
		CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE,
			&UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ModelMatrixResource));
		//修改资源大小，创建顶点资源
		UploadResourceDesc.Width = sizeof(VERTEX) * 24;
		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE,
			&UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexResource));
		//修改资源大小，创建索引资源
		UploadResourceDesc.Width = sizeof(UINT) * 36;
		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE,
			&UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexResource));

		//存储顶点资源
		BYTE* TransmitPointer = nullptr;
		m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
		memcpy(TransmitPointer, _temp_ModelMatrixGroup.data(), sizeof(XMFLOAT4X4) * 24);
		m_VertexResource->Unmap(0, nullptr);
		//存储索引资源
		m_IndexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
        memcpy(TransmitPointer, IndexArray, sizeof(UINT) * 36);
        m_IndexResource->Unmap(0, nullptr);
		//存储矩阵资源
		m_ModelMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
		memcpy(TransmitPointer, &_temp_ModelMatrixGroup[0], 24 * sizeof(XMFLOAT4X4));
		m_ModelMatrixResource->Unmap(0, nullptr);

		VertexBufferView[0].BufferLocation = m_VertexResource->GetGPUVirtualAddress();
		VertexBufferView[0].SizeInBytes = 24 * sizeof(VERTEX);
		VertexBufferView[0].StrideInBytes = sizeof(VERTEX);

		VertexBufferView[1].BufferLocation = m_ModelMatrixResource->GetGPUVirtualAddress();
		VertexBufferView[1].SizeInBytes = 24 * sizeof(XMFLOAT4X4);
		VertexBufferView[1].StrideInBytes = sizeof(XMFLOAT4X4);

		IndexBufferView.BufferLocation = m_IndexResource->GetGPUVirtualAddress();
		IndexBufferView.SizeInBytes = 36 * sizeof(UINT);
		IndexBufferView.Format = DXGI_FORMAT_R32_UINT;


	}
};

// 台阶方块类 (抽象类)，继承自模型类，只定义 CreateResourceAndDescriptor 这个函数，DrawModel 仍然需要派生类实现
class SoildStair : public Model
{
protected:
	inline static VERTEX VertexArray[40] =
	{
		// 台阶底面
		{{0,0,0,1},{0,0}},
		{{1,0,0,1},{1,0}},
		{{1,0,1,1},{1,1}},
		{{0,0,1,1},{0,1}},

		// 台阶背面
		{{1,1,1,1},{0,0}},
		{{0,1,1,1},{1,0}},
		{{0,0,1,1},{1,1}},
		{{1,0,1,1},{0,1}},

		// 台阶正面
		{{0,0.5,0,1},{0,0.5}},
		{{1,0.5,0,1},{1,0.5}},
		{{1,0,0,1},{1,1}},
		{{0,0,0,1},{0,1}},

		{{0,1,0.5,1},{0,0}},
		{{1,1,0.5,1},{1,0}},
		{{1,0.5,0.5,1},{1,0.5}},
		{{0,0.5,0.5,1},{0,0.5}},

		// 台阶顶面
		{{0,0.5,0.5,1},{0,0.5}},
		{{1,0.5,0.5,1},{1,0.5}},
		{{1,0.5,0,1},{1,1}},
		{{0,0.5,0,1},{0,1}},

		{{0,1,1,1},{0,0}},
		{{1,1,1,1},{1,0}},
		{{1,1,0.5,1},{1,0.5}},
		{{0,1,0.5,1},{0,0.5}},

		// 台阶左面
		{{0,1,1,1},{0,0}},
		{{0,1,0.5,1},{0.5,0}},
		{{0,0,0.5,1},{0.5,1}},
		{{0,0,1,1},{0,1}},

		{{0,0.5,0.5,1},{0.5,0.5}},
		{{0,0.5,0,1},{1,0.5}},
		{{0,0,0,1},{1,1}},
		{{0,0,0.5,1},{0.5,1}},

		// 台阶右面
		{{1,1,0.5,1},{0.5,0}},
		{{1,1,1,1},{1,0}},
		{{1,0,1,1},{1,1}},
		{{1,0,0.5,1},{0.5,1}},

		{{1,0.5,0,1},{0,0.5}},
		{{1,0.5,0.5,1},{0.5,0.5}},
		{{1,0,0.5,1},{0.5,1}},
		{{1,0,0,1},{0,1}}
	};

	inline static UINT IndexArray[60] =
	{
		// 台阶底面
		0,1,2,0,2,3,
		// 台阶背面
		4,5,6,4,6,7,
		// 台阶正面
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		// 台阶顶面
		16,17,18,16,18,19,
		20,21,22,20,22,23,
		// 台阶左面
		24,25,26,24,26,27,
		28,29,30,28,30,31,
		// 台阶右面
		32,33,34,32,34,35,
		36,37,38,36,38,39
	};

public:
	virtual void CreateResourceAndDescriptor(ComPtr<ID3D12Device4>& pD3D12Device) override
	{ 
		XMFLOAT4X4 _temp_ModelMatrix = {};
		XMStoreFloat4x4(&_temp_ModelMatrix, ModelMatrix);
		std::vector<XMFLOAT4X4> _temp_ModelMatrixGroup;
		_temp_ModelMatrixGroup.assign(40, _temp_ModelMatrix);
		//矩阵资源
		CD3DX12_RESOURCE_DESC UploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(XMFLOAT4X4) * 40);

		CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&m_ModelMatrixResource));
		//顶点资源
		UploadResourceDesc.Width = 40 * sizeof(VERTEX);
		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &UploadResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexResource));
		
		//索引资源
		UploadResourceDesc.Width = sizeof(UINT) * 60;
		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &UploadResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexResource));

		//将资源存储到上传堆中
		BYTE* TransmitPointer = nullptr;
		m_ModelMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
		memcpy(TransmitPointer, _temp_ModelMatrixGroup.data(), sizeof(XMFLOAT4X4) * 40);
		m_ModelMatrixResource->Unmap(0, nullptr);

		m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
		memcpy(TransmitPointer, VertexArray, 40 * sizeof(VERTEX));
		m_VertexResource->Unmap(0, nullptr);

		m_IndexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
		memcpy(TransmitPointer, IndexArray, 60 * sizeof(UINT));
		m_IndexResource->Unmap(0, nullptr);


		VertexBufferView[0].BufferLocation = m_VertexResource->GetGPUVirtualAddress();
        VertexBufferView[0].SizeInBytes = 40 * sizeof(VERTEX);
        VertexBufferView[0].StrideInBytes = sizeof(VERTEX);
        VertexBufferView[1].BufferLocation = m_ModelMatrixResource->GetGPUVirtualAddress();
        VertexBufferView[1].SizeInBytes = 40 * sizeof(XMFLOAT4X4);
        VertexBufferView[1].StrideInBytes = sizeof(XMFLOAT4X4);

        IndexBufferView.BufferLocation = m_IndexResource->GetGPUVirtualAddress();
        IndexBufferView.SizeInBytes = 60 * sizeof(UINT);
        IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	}
};

// 泥土 (实例类)，继承自全遮挡固体方块
class Dirt : public SoildBlock
{
public:
	// 构造函数，调用 AppendTextureKey 添加需要的纹理
	Dirt()
	{
		this->AppendTextureKey("dirt");
	}

	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) override
	{
		pCommandList->IASetIndexBuffer(&IndexBufferView);
		pCommandList->IASetVertexBuffers(0, 2, VertexBufferView);

		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["dirt"]);
		
		pCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
	}

};
// 橡木木板 (实例类)，继承自全遮挡固体方块
class Planks_Oak : public SoildBlock
{
public:
	// 构造函数，调用 AppendTextureKey 添加需要的纹理
	Planks_Oak()
	{
		this->AppendTextureKey("planks_oak");
	}

	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) override
	{
		pCommandList->IASetIndexBuffer(&IndexBufferView);
		pCommandList->IASetVertexBuffers(0, 2, VertexBufferView);
		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["planks_oak"]);

		pCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
	}


};
// 熔炉 (实例类)，继承自全遮挡固体方块
class Furnace : public SoildBlock
{
public:
	Furnace()
	{
		this->AppendTextureKey("furnace_front_off");
		this->AppendTextureKey("furnace_side");
		this->AppendTextureKey("furnace_top");
	}

	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) override
	{ 
		pCommandList->IASetIndexBuffer(&IndexBufferView);
		pCommandList->IASetVertexBuffers(0, 2, VertexBufferView);
		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["furnace_top"]);
		pCommandList->DrawIndexedInstanced(12, 1, 24, 0, 0);

		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["furnace_side"]);
		pCommandList->DrawIndexedInstanced(18, 1, 6, 0, 0);

		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["furnace_front_off"]);
		pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
};

// 工作台 (实例类)，继承自全遮挡固体方块
class Crafting_Table : public SoildBlock
{
public:
	Crafting_Table()
	{
		this->AppendTextureKey("crafting_table_front");
		this->AppendTextureKey("crafting_table_side");
		this->AppendTextureKey("crafting_table_top");
	}
	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) override
	{
		pCommandList->IASetVertexBuffers(0, 2, VertexBufferView);
        pCommandList->IASetIndexBuffer(&IndexBufferView);
		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["crafting_table_top"]);
		pCommandList->DrawIndexedInstanced(12, 1, 24, 0, 0);
		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["crafting_table_side"]);
		pCommandList->DrawIndexedInstanced(18, 1, 6, 0, 0);
		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["crafting_table_front"]);
		pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
};

class Log_Oak : public SoildBlock
{
public:
	Log_Oak()
	{
		this->AppendTextureKey("log_oak");
		this->AppendTextureKey("log_oak_top");
	}

	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) override
	{
		pCommandList->IASetVertexBuffers(0, 2, VertexBufferView);
		pCommandList->IASetIndexBuffer(&IndexBufferView);
		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["log_oak_top"]);
		pCommandList->DrawIndexedInstanced(12, 1, 24, 0, 0);
		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["log_oak"]);
		pCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0);

	}
};
// 草方块 (实例类)，继承自全遮挡固体方块
class Grass : public SoildBlock
{
public:
	Grass()
	{
		this->AppendTextureKey("grass_side");
		this->AppendTextureKey("grass_top");
		this->AppendTextureKey("dirt");
	}
	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) override
	{
		pCommandList->IASetIndexBuffer(&IndexBufferView);
		pCommandList->IASetVertexBuffers(0, 2, VertexBufferView);

		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["grass_top"]);
		pCommandList->DrawIndexedInstanced(6, 1, 24, 0, 0);

		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["dirt"]);
		pCommandList->DrawIndexedInstanced(6, 1, 30, 0, 0);

		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["grass_side"]);
		pCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0);
	}
};

// 橡木完整台阶 (实例类)，继承自完整台阶方块
class Planks_Oak_SoildStair : public SoildStair
{
public:
	Planks_Oak_SoildStair()
	{
		this->AppendTextureKey("planks_oak");
	}
	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) override
	{

		pCommandList->IASetIndexBuffer(&IndexBufferView);

		pCommandList->IASetVertexBuffers(0, 2, VertexBufferView);

		pCommandList->SetGraphicsRootDescriptorTable(1, Texture_GPUHandle_Map["planks_oak"]);
		pCommandList->DrawIndexedInstanced(60, 1, 0, 0, 0);
	}
};
// 模型管理器
class ModelManager
{
public:
	// 纹理映射表元素结构体
	struct TEXTURE_MAP_INFO
	{
		std::wstring TextureFilePath;			// 文件路径
		// 位于默认堆上纹理资源
		ComPtr<ID3D12Resource> DefaultHeapTextureResource;
		// 位于上传堆的纹理资源
		ComPtr<ID3D12Resource> UploadHeapTextureResource;

		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
	};
	// 纹理映射表
	std::unordered_map<std::string, TEXTURE_MAP_INFO> Texture_SRV_Map;
	// 模型组，存储 Model 类指针的 vector，注意存储的是指针，指针可以指向不同类的对象
	std::vector<Model*> ModelGroup;

public:

	// 构造函数，我们在构造函数上创建纹理映射表
	ModelManager()
	{
		Texture_SRV_Map["dirt"].TextureFilePath = L"resource/dirt.png";
		Texture_SRV_Map["grass_top"].TextureFilePath = L"resource/grass_top.png";
		Texture_SRV_Map["grass_side"].TextureFilePath = L"resource/grass_side.png";
		Texture_SRV_Map["log_oak"].TextureFilePath = L"resource/log_oak.png";
		Texture_SRV_Map["log_oak_top"].TextureFilePath = L"resource/log_oak_top.png";
		Texture_SRV_Map["furnace_front_off"].TextureFilePath = L"resource/furnace_front_off.png";
		Texture_SRV_Map["furnace_side"].TextureFilePath = L"resource/furnace_side.png";
		Texture_SRV_Map["furnace_top"].TextureFilePath = L"resource/furnace_top.png";
		Texture_SRV_Map["crafting_table_front"].TextureFilePath = L"resource/crafting_table_front.png";
		Texture_SRV_Map["crafting_table_side"].TextureFilePath = L"resource/crafting_table_side.png";
		Texture_SRV_Map["crafting_table_top"].TextureFilePath = L"resource/crafting_table_top.png";
		Texture_SRV_Map["planks_oak"].TextureFilePath = L"resource/planks_oak.png";
	}

	// 创建方块，我们在这里写上创建方块的代码
	void CreateBlock()
	{
		// 两层泥土地基，y 是高度
		for (int x = 0; x < 10; x++)
		{
			for (int z = -4; z < 10; z++)
			{
				for (int y = -2; y < 0; y++)
				{
					Model* dirt = new Dirt();							// 创建对象指针，调用时会根据虚函数表调用不同的函数
					dirt->SetModelMatrix(XMMatrixTranslation(x, y, z));	// 设置不同的模型矩阵，XMMatrixTranslation 平移模型
					ModelGroup.push_back(dirt);							// 将新模型添加到模型组
				}
			}
		}

		// 一层草方块地基
		for (int x = 0; x < 10; x++)
		{
			for (int z = -4; z < 10; z++)
			{
				Model* grass = new Grass();
				grass->SetModelMatrix(XMMatrixTranslation(x, 0, z));
				ModelGroup.push_back(grass);
			}
		}

		// 4x4 木板房基

		for (int x = 3; x < 7; x++)
		{
			for (int z = 3; z < 7; z++)
			{
				Model* plank = new Planks_Oak();
				plank->SetModelMatrix(XMMatrixTranslation(x, 2, z));
				ModelGroup.push_back(plank);
			}
		}


		// 8 柱原木 

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(3, y, 2));
			ModelGroup.push_back(log_oak);
		}

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(2, y, 3));
			ModelGroup.push_back(log_oak);
		}

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(6, y, 2));
			ModelGroup.push_back(log_oak);
		}

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(7, y, 3));
			ModelGroup.push_back(log_oak);
		}

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(7, y, 6));
			ModelGroup.push_back(log_oak);
		}

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(6, y, 7));
			ModelGroup.push_back(log_oak);
		}

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(2, y, 6));
			ModelGroup.push_back(log_oak);
		}

		for (int y = 1; y < 7; y++)
		{
			Model* log_oak = new Log_Oak();
			log_oak->SetModelMatrix(XMMatrixTranslation(3, y, 7));
			ModelGroup.push_back(log_oak);
		}


		// 其他木板与门前台阶
		{
			Model* plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(4, 2, 2));
			ModelGroup.push_back(plank);

			plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(5, 2, 2));
			ModelGroup.push_back(plank);

			for (int y = 5; y < 7; y++)
			{
				for (int x = 4; x < 6; x++)
				{
					plank = new Planks_Oak();
					plank->SetModelMatrix(XMMatrixTranslation(x, y, 2));
					ModelGroup.push_back(plank);
				}
			}

			for (int y = 2; y < 4; y++)
			{
				for (int z = 4; z < 6; z++)
				{
					plank = new Planks_Oak();
					plank->SetModelMatrix(XMMatrixTranslation(2, y, z));
					ModelGroup.push_back(plank);
				}
			}

			for (int y = 2; y < 4; y++)
			{
				for (int x = 4; x < 6; x++)
				{
					plank = new Planks_Oak();
					plank->SetModelMatrix(XMMatrixTranslation(x, y, 7));
					ModelGroup.push_back(plank);
				}
			}

			for (int y = 2; y < 4; y++)
			{
				for (int z = 4; z < 6; z++)
				{
					plank = new Planks_Oak();
					plank->SetModelMatrix(XMMatrixTranslation(7, y, z));
					ModelGroup.push_back(plank);
				}
			}

			plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(2, 6, 4));
			ModelGroup.push_back(plank);

			plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(2, 6, 5));
			ModelGroup.push_back(plank);

			plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(4, 6, 7));
			ModelGroup.push_back(plank);

			plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(5, 6, 7));
			ModelGroup.push_back(plank);

			plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(7, 6, 4));
			ModelGroup.push_back(plank);

			plank = new Planks_Oak();
			plank->SetModelMatrix(XMMatrixTranslation(7, 6, 5));
			ModelGroup.push_back(plank);

			Model* stair = new Planks_Oak_SoildStair();
			stair->SetModelMatrix(XMMatrixTranslation(4, 2, 1));
			ModelGroup.push_back(stair);

			stair = new Planks_Oak_SoildStair();
			stair->SetModelMatrix(XMMatrixTranslation(5, 2, 1));
			ModelGroup.push_back(stair);

			stair = new Planks_Oak_SoildStair();
			stair->SetModelMatrix(XMMatrixTranslation(4, 1, 0));
			ModelGroup.push_back(stair);

			stair = new Planks_Oak_SoildStair();
			stair->SetModelMatrix(XMMatrixTranslation(5, 1, 0));
			ModelGroup.push_back(stair);
		}

		// 4x4 木板房顶

		for (int x = 3; x < 7; x++)
		{
			for (int z = 3; z < 7; z++)
			{
				Model* plank = new Planks_Oak();
				plank->SetModelMatrix(XMMatrixTranslation(x, 6, z));
				ModelGroup.push_back(plank);
			}
		}

		// 屋顶

		{
			// 第一层

			for (int x = 3; x < 7; x++)
			{
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(XMMatrixTranslation(x, 6, 1));
				ModelGroup.push_back(stair);
			}

			for (int x = 3; x < 7; x++)
			{
				// 旋转橡木台阶用的模型矩阵
				// 这里本来是可以不用 XMMatrixTranslation(-0.5, -0.5, -0.5) 平移到模型中心的
				// 因为作者本人 (我) 的设计失误，把模型坐标系原点建立在模型左下角了 (见上文的 VertexArray)
				// 导致还要先把原点平移到模型中心，旋转完再还原，增大计算量，这个是完全可以规避的
				// 读者可以自行修改 VertexArray，使方块以自身中心为原点建系，这样就可以直接 XMMatrixRotationY() 进行旋转了
				XMMATRIX transform = XMMatrixTranslation(-0.5, -0.5, -0.5);
				transform *= XMMatrixRotationY(XM_PI);						// 平移中心后，再旋转，否则会出错 (旋转角度是弧度)
				transform *= XMMatrixTranslation(0.5, 0.5, 0.5);			// 旋转完再还原
				transform *= XMMatrixTranslation(x, 6, 8);					// 再平移到对应的坐标
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(transform);
				ModelGroup.push_back(stair);
			}

			for (int z = 3; z < 7; z++)
			{
				XMMATRIX transform = XMMatrixTranslation(-0.5, -0.5, -0.5);
				transform *= XMMatrixRotationY(XM_PIDIV2);					// 旋转 90°
				transform *= XMMatrixTranslation(0.5, 0.5, 0.5);
				transform *= XMMatrixTranslation(1, 6, z);
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(transform);
				ModelGroup.push_back(stair);
			}

			for (int z = 3; z < 7; z++)
			{
				XMMATRIX transform = XMMatrixTranslation(-0.5, -0.5, -0.5);
				transform *= XMMatrixRotationY(XM_PI + XM_PIDIV2);			// 旋转 270°
				transform *= XMMatrixTranslation(0.5, 0.5, 0.5);
				transform *= XMMatrixTranslation(8, 6, z);
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(transform);
				ModelGroup.push_back(stair);
			}

			// 第二层

			for (int x = 3; x < 7; x++)
			{
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(XMMatrixTranslation(x, 7, 2));
				ModelGroup.push_back(stair);
			}

			for (int x = 3; x < 7; x++)
			{
				XMMATRIX transform = XMMatrixTranslation(-0.5, -0.5, -0.5);
				transform *= XMMatrixRotationY(XM_PI);
				transform *= XMMatrixTranslation(0.5, 0.5, 0.5);
				transform *= XMMatrixTranslation(x, 7, 7);
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(transform);
				ModelGroup.push_back(stair);
			}

			for (int z = 3; z < 7; z++)
			{
				XMMATRIX transform = XMMatrixTranslation(-0.5, -0.5, -0.5);
				transform *= XMMatrixRotationY(XM_PIDIV2);
				transform *= XMMatrixTranslation(0.5, 0.5, 0.5);
				transform *= XMMatrixTranslation(2, 7, z);
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(transform);
				ModelGroup.push_back(stair);
			}

			for (int z = 3; z < 7; z++)
			{
				XMMATRIX transform = XMMatrixTranslation(-0.5, -0.5, -0.5);
				transform *= XMMatrixRotationY(XM_PI + XM_PIDIV2);
				transform *= XMMatrixTranslation(0.5, 0.5, 0.5);
				transform *= XMMatrixTranslation(7, 7, z);
				Model* stair = new Planks_Oak_SoildStair();
				stair->SetModelMatrix(transform);
				ModelGroup.push_back(stair);
			}

			// 补上屋顶空位

			for (int x = 3; x < 7; x++)
			{
				for (int z = 3; z < 7; z++)
				{
					Model* plank = new Planks_Oak();
					plank->SetModelMatrix(XMMatrixTranslation(x, 7, z));
					ModelGroup.push_back(plank);
				}
			}
		}

		// 工作台和熔炉
		{
			Model* craft_table = new Crafting_Table();
			craft_table->SetModelMatrix(XMMatrixTranslation(3, 3, 6));
			ModelGroup.push_back(craft_table);

			Model* furnace = new Furnace();
			furnace->SetModelMatrix(XMMatrixTranslation(4, 3, 6));
			ModelGroup.push_back(furnace);

			furnace = new Furnace();
			furnace->SetModelMatrix(XMMatrixTranslation(5, 3, 6));
			ModelGroup.push_back(furnace);
		}

	}

	// 当一切准备就绪后，就可以正式创建模型资源，准备渲染了
	// 调用该函数的前提是: 依次完成 DX12Engine::CreateModelTextureResource (读取并创建纹理资源)，CreateBlock (创建方块，设置模型矩阵)
	void CreateModelResource(ComPtr<ID3D12Device4>& pD3D12Device)
	{
		//此处创建的资源会存在资源堆中，当配置好根参数后，就可以根据根参数使用资源堆中的资源
		for (auto& model : ModelGroup)
        {
            model->CreateResourceAndDescriptor(pD3D12Device);
			// 遍历模型自身的映射表，设置模型需要用到的纹理
			for (const auto& texture : model->RequestForTextureMap())
			{
				// 设置模型的 SRV 描述符
				model->SetTextureGPUHandle(texture.first, Texture_SRV_Map[texture.first].GPUHandle);
			}
        }
	}

	void RenderAllModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList)
	{
		// 遍历模型组
		for (const auto& model : ModelGroup)
		{
			model->DrawModel(pCommandList);
		}
	}
};

// DX12 引擎
class DX12Engine
{
private:
	int WindowWidth = 640;		// 窗口宽度
	int WindowHeight = 480;		// 窗口高度
	HWND m_hwnd;				// 窗口句柄

	ComPtr<ID3D12Debug> m_D3D12DebugDevice;					// D3D12 调试层设备
	UINT m_DXGICreateFactoryFlag = NULL;					// 创建 DXGI 工厂时需要用到的标志

	ComPtr<IDXGIFactory5> m_DXGIFactory;					// DXGI 工厂
	ComPtr<IDXGIAdapter1> m_DXGIAdapter;					// 显示适配器 (显卡)
	ComPtr<ID3D12Device4> m_D3D12Device;					// D3D12 核心设备

	ComPtr<ID3D12CommandQueue> m_CommandQueue;				// 命令队列
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;		// 命令分配器
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;		// 命令列表

	ComPtr<IDXGISwapChain3> m_DXGISwapChain;				// DXGI 交换链
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;					// RTV 描述符堆
	ComPtr<ID3D12Resource> m_RenderTarget[3];				// 渲染目标缓冲区数组，每一副渲染缓冲对应一个窗口缓冲区
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle;					// RTV 描述符句柄
	UINT RTVDescriptorSize = 0;								// RTV 描述符的大小
	UINT FrameIndex = 0;									// 帧索引，表示当前渲染的第 i 帧 (第 i 个渲染目标)

	ComPtr<ID3D12Fence> m_Fence;							// 围栏
	UINT64 FenceValue = 0;									// 用于围栏等待的围栏值
	HANDLE RenderEvent = NULL;								// GPU 渲染事件
	CD3DX12_RESOURCE_BARRIER barrier = {};				


	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;					// DSV 描述符堆
	D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle;					// DSV 描述符句柄
	ComPtr<ID3D12Resource> m_DepthStencilBuffer;			// DSV 深度模板缓冲资源

	// DSV 资源的格式
	// 深度模板缓冲只支持四种格式:
	// DXGI_FORMAT_D24_UNORM_S8_UINT	(每个像素占用四个字节 32 位，24 位无符号归一化浮点数留作深度值，8 位整数留作模板值)
	// DXGI_FORMAT_D32_FLOAT_S8X24_UINT	(每个像素占用八个字节 64 位，32 位浮点数留作深度值，8 位整数留作模板值，其余 24 位保留不使用)
	// DXGI_FORMAT_D16_UNORM			(每个像素占用两个字节 16 位，16 位无符号归一化浮点数留作深度值，范围 [0,1]，不使用模板)
	// DXGI_FORMAT_D32_FLOAT			(每个像素占用四个字节 32 位，32 位浮点数留作深度值，不使用模板)
	// 这里我们选择最常用的格式 DXGI_FORMAT_D24_UNORM_S8_UINT
	DXGI_FORMAT DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


	ModelManager m_ModelManager;							// 模型管理器，帮助管理并渲染模型

	ComPtr<IWICImagingFactory> m_WICFactory;				// WIC 工厂
	ComPtr<IWICBitmapDecoder> m_WICBitmapDecoder;			// 位图解码器
	ComPtr<IWICBitmapFrameDecode> m_WICBitmapDecodeFrame;	// 由解码器得到的单个位图帧
	ComPtr<IWICFormatConverter> m_WICFormatConverter;		// 位图转换器
	ComPtr<IWICBitmapSource> m_WICBitmapSource;				// WIC 位图资源，用于获取位图数据
	UINT TextureWidth = 0;									// 纹理宽度
	UINT TextureHeight = 0;									// 纹理高度
	UINT BitsPerPixel = 0;									// 图像深度，图片每个像素占用的比特数
	UINT BytePerRowSize = 0;								// 纹理每行数据的真实字节大小，用于读取纹理数据、上传纹理资源
	DXGI_FORMAT TextureFormat = DXGI_FORMAT_UNKNOWN;		// 纹理格式

	ComPtr<ID3D12DescriptorHeap> m_SRVHeap;					// SRV 描述符堆

	UINT TextureSize = 0;									// 纹理的真实大小 (单位：字节)
	UINT UploadResourceRowSize = 0;							// 上传堆资源每行的大小 (单位：字节)
	UINT UploadResourceSize = 0;							// 上传堆资源的总大小 (单位：字节)



	ComPtr<ID3D12Resource> m_CBVResource;		// 常量缓冲资源，用于存放 MVP 矩阵，MVP 矩阵每帧都要更新，所以需要存储在常量缓冲区中
	struct CBuffer								// 常量缓冲结构体
	{
		XMFLOAT4X4 MVPMatrix;		// MVP 矩阵，用于将顶点数据从顶点空间变换到齐次裁剪空间
	};
	CBuffer* MVPBuffer = nullptr;	// 常量缓冲结构体指针，里面存储的是 MVP 矩阵信息，下文 Map 后指针会指向 CBVResource 的地址

	Camera m_FirstCamera;			// 第一人称摄像机

	ComPtr<ID3D12RootSignature> m_RootSignature;			// 根签名
	ComPtr<ID3D12PipelineState> m_PipelineStateObject;		// 渲染管线状态


	// 视口
	D3D12_VIEWPORT viewPort = D3D12_VIEWPORT{ 0, 0, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// 裁剪矩形
	D3D12_RECT ScissorRect = D3D12_RECT{ 0, 0, WindowWidth, WindowHeight };
	std::unique_ptr<BYTE[]> TextureData;
public:
	void InitWindow(HINSTANCE hins)
	{
		WNDCLASS  wc = {};
		wc.hInstance = hins;
		CallBackWrapper::Broker_Func = std::bind(&DX12Engine::CallBackFunc, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		wc.lpfnWndProc = CallBackWrapper::CallBackFunc;
		wc.lpszClassName = L"DX12Window";
		RegisterClass(&wc);

		m_hwnd = CreateWindow(wc.lpszClassName, L"Minecraft", WS_SYSMENU | WS_OVERLAPPED,
			10, 10, WindowWidth, WindowHeight,
			NULL, NULL, hins, NULL);

		ShowWindow(m_hwnd, SW_SHOW);
	}

	void CreateDebugDevice()
	{
		::CoInitialize(nullptr);	// 注意这里！DX12 的所有设备接口都是基于 COM 接口的，我们需要先全部初始化为 nullptr

#if defined(_DEBUG)
		// 获取调试层设备接口
		D3D12GetDebugInterface(IID_PPV_ARGS(&m_D3D12DebugDevice));
		// 开启调试层
		m_D3D12DebugDevice->EnableDebugLayer();
		// 开启调试层后，创建 DXGI 工厂也需要 Debug Flag
		m_DXGICreateFactoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
	}

	bool CreateDevice()
	{
		CreateDXGIFactory2(m_DXGICreateFactoryFlag, IID_PPV_ARGS(&m_DXGIFactory));
		// DX12 支持的所有功能版本，你的显卡最低需要支持 11.0
		const D3D_FEATURE_LEVEL dx12SupportLevel[] =
		{
			D3D_FEATURE_LEVEL_12_2,		// 12.2
			D3D_FEATURE_LEVEL_12_1,		// 12.1
			D3D_FEATURE_LEVEL_12_0,		// 12.0
			D3D_FEATURE_LEVEL_11_1,		// 11.1
			D3D_FEATURE_LEVEL_11_0		// 11.0
		};
		for (UINT i = 0; m_DXGIFactory->EnumAdapters1(i, &m_DXGIAdapter) != ERROR_NOT_FOUND; i++)
		{
			// 找到显卡，就创建 D3D12 设备，从高到低遍历所有功能版本，创建成功就跳出
			for (const auto& level : dx12SupportLevel)
			{
				// 创建 D3D12 核心层设备，创建成功就返回 true
				if (SUCCEEDED(D3D12CreateDevice(m_DXGIAdapter.Get(), level, IID_PPV_ARGS(&m_D3D12Device))))
				{
					DXGI_ADAPTER_DESC1 adap = {};
					m_DXGIAdapter->GetDesc1(&adap);
					OutputDebugStringW(adap.Description);		// 在输出窗口上输出创建 D3D12 设备所用的显卡名称
					return true;
				}
			}
		}
		if (m_D3D12Device == nullptr)
		{
			MessageBox(NULL, L"找不到任何能支持 DX12 的显卡，请升级电脑上的硬件！", L"错误", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	void CreateCommandComponent()
	{ 
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		m_D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
        m_D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		m_D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));
		m_CommandList->Close();
	}

	void CreateRenderTarget()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 3;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_D3D12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap));
		
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = 3;
        swapChainDesc.Width = WindowWidth;
        swapChainDesc.Height = WindowHeight;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
	    
		ComPtr<IDXGISwapChain1> _temp_swapchain;
		m_DXGIFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_hwnd, &swapChainDesc, nullptr, nullptr, &_temp_swapchain);
        _temp_swapchain.As(&m_DXGISwapChain);

		RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		RTVDescriptorSize = m_D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		for (int i = 0; i < 3; i++)
		{
			m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
			m_D3D12Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, RTVHandle);
            RTVHandle.ptr += RTVDescriptorSize;
		}
	}
	void CreateFence()
	{
		RenderEvent = CreateEvent(nullptr, false, true, nullptr);
		m_D3D12Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
	}

	void CreatDSVHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc{};
		DSVHeapDesc.NumDescriptors = 1;
		DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		m_D3D12Device->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_DSVHeap));

		DSVHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

	}

	void CreateDepthStencilBuffer()
	{
		CD3DX12_RESOURCE_DESC DSVResourceDesc{};
		DSVResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DSVFormat,
			WindowWidth,
			WindowHeight,
			1, 1, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
			D3D12_TEXTURE_LAYOUT_UNKNOWN
		);

		CD3DX12_CLEAR_VALUE DepthStencilBufferClearValue(
			DSVFormat,
			1.0f,
			0.0f
		);

		CD3DX12_HEAP_PROPERTIES DefaultProperties(D3D12_HEAP_TYPE_DEFAULT);

		m_D3D12Device->CreateCommittedResource(&DefaultProperties, D3D12_HEAP_FLAG_NONE,
			&DSVResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &DepthStencilBufferClearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));
	}

	void CreatDSV()
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc{};
        DSVDesc.Format = DSVFormat;
        DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
		m_D3D12Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &DSVDesc, DSVHandle);
	}

	void CreateSRVHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc{};
		SRVHeapDesc.NumDescriptors = m_ModelManager.Texture_SRV_Map.size();
        SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		m_D3D12Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap));

	}

	void StartCommandRecord()
	{
		m_CommandAllocator->Reset();
        m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
	}

	// 加载纹理到内存中
	bool LoadTextureFromFile(std::wstring TextureFilename)
	{
		// 如果还没创建 WIC 工厂，就新建一个 WIC 工厂实例。注意！WIC 工厂不可以重复释放与创建！
		if (m_WICFactory == nullptr) CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_WICFactory));

		// 创建图片解码器，并将图片读入到解码器中
		HRESULT hr = m_WICFactory->CreateDecoderFromFilename(TextureFilename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_WICBitmapDecoder);

		std::wostringstream output_str;		// 用于格式化字符串
		switch (hr)
		{
		case S_OK: break;	// 解码成功，直接 break 进入下一步即可

		case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):	// 文件找不到
			output_str << L"找不到文件 " << TextureFilename << L" ！请检查文件路径是否有误！";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;

		case HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT):	// 文件句柄正在被另一个应用进程占用
			output_str << L"文件 " << TextureFilename << L" 已经被另一个应用进程打开并占用了！请先关闭那个应用进程！";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;

		case WINCODEC_ERR_COMPONENTNOTFOUND:			// 找不到可解码的组件，说明这不是有效的图像文件
			output_str << L"文件 " << TextureFilename << L" 不是有效的图像文件，无法解码！请检查文件是否为图像文件！";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;

		default:			// 发生其他未知错误
			output_str << L"文件 " << TextureFilename << L" 解码失败！发生了其他错误，错误码：" << hr << L" ，请查阅微软官方文档。";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;
		}

		// 获取图片数据的第一帧，这个 GetFrame 可以用于 gif 这种多帧动图
		m_WICBitmapDecoder->GetFrame(0, &m_WICBitmapDecodeFrame);


		// 获取图片格式，并将它转化为 DX12 能接受的纹理格式
		// 如果碰到格式无法支持的错误，可以用微软提供的 画图3D 来转换，强力推荐!
		WICPixelFormatGUID SourceFormat = {};				// 源图格式
		GUID TargetFormat = {};								// 目标格式

		m_WICBitmapDecodeFrame->GetPixelFormat(&SourceFormat);						// 获取源图格式

		if (DX12TextureHelper::GetTargetPixelFormat(&SourceFormat, &TargetFormat))	// 获取目标格式
		{
			TextureFormat = DX12TextureHelper::GetDXGIFormatFromPixelFormat(&TargetFormat);	// 获取 DX12 支持的格式
		}
		else	// 如果没有可支持的目标格式
		{
			::MessageBox(NULL, L"此纹理不受支持!", L"提示", MB_OK);
			return false;
		}


		// 获取目标格式后，将纹理转换为目标格式，使其能被 DX12 使用
		m_WICFactory->CreateFormatConverter(&m_WICFormatConverter);		// 创建图片转换器
		// 初始化转换器，实际上是把位图进行了转换
		m_WICFormatConverter->Initialize(m_WICBitmapDecodeFrame.Get(), TargetFormat, WICBitmapDitherTypeNone,
			nullptr, 0.0f, WICBitmapPaletteTypeCustom);
		// 将位图数据继承到 WIC 位图资源，我们要在这个 WIC 位图资源上获取信息
		m_WICFormatConverter.As(&m_WICBitmapSource);



		m_WICBitmapSource->GetSize(&TextureWidth, &TextureHeight);		// 获取纹理宽高

		ComPtr<IWICComponentInfo> _temp_WICComponentInfo = {};			// 用于获取 BitsPerPixel 纹理图像深度
		ComPtr<IWICPixelFormatInfo> _temp_WICPixelInfo = {};			// 用于获取 BitsPerPixel 纹理图像深度
		m_WICFactory->CreateComponentInfo(TargetFormat, &_temp_WICComponentInfo);
		_temp_WICComponentInfo.As(&_temp_WICPixelInfo);
		_temp_WICPixelInfo->GetBitsPerPixel(&BitsPerPixel);				// 获取 BitsPerPixel 图像深度

		return true;
	}

	// 上取整算法，对 A 向上取整，判断至少要多少个长度为 B 的空间才能容纳 A，用于内存对齐
	inline UINT Ceil(UINT A, UINT B)
	{
		return (A + B - 1) / B;
	}

	void CreateUploadAndDefaultResource(ModelManager::TEXTURE_MAP_INFO& Info)
	{
		BytePerRowSize = TextureWidth * BitsPerPixel / 8;
		TextureSize = BytePerRowSize * TextureHeight;
		UploadResourceRowSize = Ceil(BytePerRowSize, 256) * 256;
		UploadResourceSize = UploadResourceRowSize * (TextureHeight - 1) + BytePerRowSize;

		CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC UploadHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(UploadResourceSize);

		m_D3D12Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &UploadHeapDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&Info.UploadHeapTextureResource));

		CD3DX12_HEAP_PROPERTIES HeapProps1(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC DefaultHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(UploadResourceSize);

		m_D3D12Device->CreateCommittedResource(
			&HeapProps1,
			D3D12_HEAP_FLAG_NONE,
			&DefaultHeapDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&Info.DefaultHeapTextureResource)
		);

	}
	void CopyTextureDataToDefaultResource(ModelManager::TEXTURE_MAP_INFO& Info)
	{
		TextureData = std::make_unique<BYTE[]>(TextureSize);
		m_WICBitmapSource->CopyPixels(nullptr, BytePerRowSize, TextureSize, TextureData.get());
		BYTE* TextureDataPtr = TextureData.get();


		D3D12_SUBRESOURCE_DATA textureData{};
		textureData.pData = TextureData.get();					// 指向纹理数据的指针
		textureData.RowPitch = BytePerRowSize;			//为对齐后每行的大小
		textureData.SlicePitch = TextureSize;					//童谣为对齐后的整个纹理数据大小，单位：字节

		m_CommandAllocator->Reset();
		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(Info.DefaultHeapTextureResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		m_CommandList->ResourceBarrier(1, &barrier);
		UpdateSubresources<1>(
			m_CommandList.Get(),
			Info.DefaultHeapTextureResource.Get(),
			Info.UploadHeapTextureResource.Get(),
			0, 0, 1, &textureData
		);//内部是先将数据复制到上传堆，然后再从上传堆复制到默认堆，自动调用CopyTextureRegion拷贝上上传堆到默认堆
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(Info.DefaultHeapTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		m_CommandList->ResourceBarrier(1, &barrier);
		//m_CommandList->Close(); 
	}

	// 关闭命令列表，启动命令队列，正式开始将纹理复制到默认堆资源中
	void StartCommandExecute()
	{
		m_CommandList->Close();
		// 用于传递命令用的临时 ID3D12CommandList 数组
		ID3D12CommandList* _temp_cmdlists[] = { m_CommandList.Get() };

		// 提交复制命令！GPU 开始复制！
		m_CommandQueue->ExecuteCommandLists(1, _temp_cmdlists);

		// 将围栏预定值设定为下一帧，注意复制资源也需要围栏等待，否则会发生资源冲突
		FenceValue++;
		// 在命令队列 (命令队列在 GPU 端) 设置围栏预定值，此命令会加入到命令队列中
		// 命令队列执行到这里会修改围栏值，表示复制已完成，"击中"围栏
		m_CommandQueue->Signal(m_Fence.Get(), FenceValue);
		// 设置围栏的预定事件，当复制完成时，围栏被"击中"，激发预定事件，将事件由无信号状态转换成有信号状态
		m_Fence->SetEventOnCompletion(FenceValue, RenderEvent);
	}

	// 最终创建 SRV 着色器资源描述符，用于描述默认堆资源为一块纹理，创建完 SRV 描述符，会将描述符句柄存储到纹理映射表中
	void CreateSRV(ModelManager::TEXTURE_MAP_INFO& Info,D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle)
	{ 
        CD3DX12_SHADER_RESOURCE_VIEW_DESC SRVDescriptorDes = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(
			TextureFormat,
			1,
			0);
		m_D3D12Device->CreateShaderResourceView(Info.DefaultHeapTextureResource.Get(), &SRVDescriptorDes, CPUHandle);

		Info.CPUHandle = CPUHandle;
		Info.GPUHandle = GPUHandle;
	}

	void CreateModelTextureResource()
	{
		//将所有的纹理
		CreateSRVHeap();
		CD3DX12_CPU_DESCRIPTOR_HANDLE CurrentCPUHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());

	}
};