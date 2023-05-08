/**
 * 该头文件包含了一些有助于 DirectX 3D 开发的工具
 * 主要分为以下部分:
 * * 通用部分(D3DHelper)
 * * 渲染用部分(D3DHelper::Render)
 * * 几何体部分(D3DHelper::Geometry)
 * ...
 *
 * 关于命名:
 * 采用匈牙利命名法，同时根据变量的作用域类型进行命名的前缀进行规划。
 *
 * - 变量作用域类型:
 * * 一般变量。指函数体内的临时变量
 * * 特殊变量。指结构体或类中的成员变量，同时函数的形式参数与实际参数也可以使用该规范
 *
 * - 变量前缀:
 * ** 一般变量前缀
 * * f:     float 浮点数
 * * i:     int 整型
 * * b:     byte 字节
 * * p(lp): (long) pointer (长)指针，也可以代表数组
 *
 * ** 特殊变量前缀
 * * n:     number 常量数字, 即在首次赋值后，其保存值在之后基本不变
 * * c:     count 计数
 * * i:     int   数字, 其值在程序运行中不断改变
 * * ix:    index 索引
 * * em:    enum 枚举
 * * p(lp): (long) pointer (长)指针, 也可以代表数组
 * * vecX:  vector 向量, X 代表维度
 * * mat:   matrix 矩阵
 *
 * ** 关于成员结构体变量或成员类变量:
 * 均根据其结构体或类的作用进行命名。
 *
 * - 函数与宏:
 * ** 常量宏
 * 保持 宏前缀-宏名称-宏名称... 的样式进行命名, 并且所有都为大写字母
 * 如: DXEXCEPTION_MAXSTRING 宏前缀为 DXEXCEPTION, 表明该宏是为 DxException 设立的；名称为 MaxString, 表明该宏是一般异常的字符最大缓冲区的字节数。
 *
 * ** 函数
 * 根据函数的作用进行命名，并且每个单词的首字母大写。
 * 函数宏与之类似。
 * 如: CreateDefaultBuffer 函数, Create 表明该函数为创建/生成函数，其生成对象为 Direct3D 中的 Default Buffer(默认堆)
 *
 *
 *
 */

#pragma once
#ifndef _D3DHELPER_H
#define _D3DHELPER_H

#include "D3DBase.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "GeometryGenerator.h"

#define DXEXCEPTION_MAXSTRING 256
#define DXEXCEPTION_MAXSTRINGEX 512
#define MAX_NUM_LIGHT 16

class DXException
{
public:
    DXException(HRESULT);
    DXException(HRESULT, const char *, unsigned int);
    DXException(HRESULT, ID3DBlob **, const char *, unsigned int);
    ~DXException();

    TCHAR *ToString();

private:
    TCHAR e[DXEXCEPTION_MAXSTRINGEX];
};

#define ThrowIfFailed(hr)                          \
    if (FAILED(hr))                                \
    {                                              \
        throw DXException(hr, __FILE__, __LINE__); \
    }
#define ThrowIfFailedEx(hr, error)                        \
    if (FAILED(hr))                                       \
    {                                                     \
        throw DXException(hr, error, __FILE__, __LINE__); \
    }

#define D3DHelper_CalcConstantBufferBytesSize(byteSize) ((byteSize + 255) & ~255)

namespace D3DHelper
{
    /// @brief 编译 HLSL 着色器
    /// @param lpszShaderFile 磁盘中文件名称
    /// @param lpszDefines     宏定义字符串
    /// @param lpszEntryPoint  着色器入口点
    /// @param lpszTarget      着色器版本
    /// @return                着色器二进制代码
    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(LPCWSTR lpszShaderFile, const D3D_SHADER_MACRO *lpszDefines, const char *lpszEntryPoint, const char *lpszTarget);

    /// @brief 创建默认堆辅助函数
    /// @param pDevice      D3D12 设备
    /// @param pComList     命令列表
    /// @param pData        上传的数据
    /// @param nByteSize    数据的字节数
    /// @param pUploader    未初始化的上传堆
    /// @return             默认堆
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pComList, const void *pData, UINT nByteSize, ID3D12Resource **pUploader);
	
	/// @brief 加载 DDS 纹理
	/// @param pDevice 		D3D12 设备
	/// @param pComList     命令列表
	/// @param lpszFileName 磁盘文件名
	/// @param pUploader    未初始化的上传堆
	/// @return 			返回纹理资源
    Microsoft::WRL::ComPtr<ID3D12Resource> LoadDDSFromFile(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pComList, LPCWSTR lpszFileName, ID3D12Resource** pUploader);
    
	/// @brief 上传堆辅助类
    class UploadBuffer
    {
    public:
        UploadBuffer();

        UploadBuffer(ID3D12Device *pDevice, UINT nElementByteSize, UINT nElementCount);
        UploadBuffer(const UploadBuffer &) = delete;
        UploadBuffer &operator=(const UploadBuffer &) = delete;
        ~UploadBuffer();

        void Init(ID3D12Device *pDevice, UINT nElementByteSize, UINT nElementCount);
        ID3D12Resource *Resource() const;
        void CopyData(int nElementIndex, const void *pData, const UINT nByteSize);

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer;
        BYTE *pBufferBegin = NULL;
        UINT nElementByteSize;
    };
	
	namespace Light
	{
		/// @brief 材质结构体
		struct Material
		{
			std::string Name;						// 材质名
			int nCBMaterialIndex;					// 本材质信息在常量缓冲区的索引
			int nDiffuseSrvHeapIndex;				// 漫反射纹理在 SRV 堆中的索引
			int iFramesDirty;					    // 数据脏标识
			
			// 用于着色材质的常量缓冲区数据
			DirectX::XMFLOAT4 vec4DiffuseAlbedo;	// 漫反射反照率
			DirectX::XMFLOAT3 vec3FresnelR0;		// 材质属性R(0°)
			float nRoughness;						// 表面粗糙度
			DirectX::XMFLOAT4X4 matTransform;
		};
		
		/// @brief 光源常量数据
		struct Light
		{
			DirectX::XMFLOAT3 vec3Strength;			// 光源颜色
			float nFalloffStart;					// 光源线性衰减起始范围(点光源/聚光灯光源)
			DirectX::XMFLOAT3 vec3Direction;		// 光源方向(方向光源/聚光灯光源)
			float nFalloffEnd;						// 光源线性衰减结束范围(点光源/聚光灯光源)
			DirectX::XMFLOAT3 vec3Position;			// 光源位置(点光源/聚光灯光源)
			float nSpotPower;						// 聚光灯因子
		};
	};
	
    namespace Constant
	{
		/// @brief 材质常量数据
		struct MaterialConstant
		{
			DirectX::XMFLOAT4 vec4DiffuseAlbedo;	// 漫反射反照率
			DirectX::XMFLOAT3 vec3FresnelR0;		// 材质属性R(0°)
			float nRoughness;						// 表面粗糙度
			DirectX::XMFLOAT4X4 matTransform;
		};
		
		/// @brief 场景常量数据
        struct SceneConstant
        {
            DirectX::XMFLOAT4X4 matView;
            DirectX::XMFLOAT4X4 matInvView;
            DirectX::XMFLOAT4X4 matProj;
            DirectX::XMFLOAT4X4 matInvProj;
            DirectX::XMFLOAT4X4 matViewProj;
            DirectX::XMFLOAT4X4 matInvViewProj;
            DirectX::XMFLOAT3 vec3EyePos;
            float fPerObjectPad1;

            DirectX::XMFLOAT2 vec2RenderTargetSize;
            DirectX::XMFLOAT2 vec2InvRenderTargetSize;
            float fNearZ;
            float fFarZ;
            float fTotalTime;
            float fDeltaTime;
			DirectX::XMFLOAT4 vec4AmbientLight;
            DirectX::XMFLOAT4 vec4FogColor;
            float fFogStart;
            float fFogRange;
            DirectX::XMFLOAT2 fPerScenePad2;

            Light::Light lights[MAX_NUM_LIGHT];
        };

        /// @brief 对象常量数据
        struct ObjectConstant
        {
            DirectX::XMFLOAT4X4 matWorld;
            DirectX::XMFLOAT4X4 matTexTransform;
        };

	};
	
	namespace Resource
	{
		struct Texture
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
            Microsoft::WRL::ComPtr<ID3D12Resource> pUploader;

            std::string Name;
            std::wstring FileName;
        };
		
        /// @brief 渲染数据描述结构体
        struct SubmeshGeometry
        {
            UINT nIndexCount;						// 顶点索引数量
            UINT nStartIndexLocation;				// 顶点索引基值
            INT nBaseVertexLocation;				// 顶点基值

            DirectX::BoundingBox Bounds;			// 几何体碰撞盒数据
        };

        /// @brief 渲染数据结构体
        struct MeshGeometry
        {
            std::string Name;

            Microsoft::WRL::ComPtr<ID3DBlob> pCPUVertexBuffer; // 顶点数据副本
            Microsoft::WRL::ComPtr<ID3DBlob> pCPUIndexBuffer;  // 索引数据副本

            Microsoft::WRL::ComPtr<ID3D12Resource> pGPUVertexBuffer;      // 顶点数据默认堆
            Microsoft::WRL::ComPtr<ID3D12Resource> pGPUIndexBuffer;       // 索引数据默认堆
            Microsoft::WRL::ComPtr<ID3D12Resource> pUploaderVertexBuffer; // 顶点数据上传堆
            Microsoft::WRL::ComPtr<ID3D12Resource> pUploaderIndexBuffer;  // 索引数据上传堆

            UINT nVertexByteStride;                           // 顶点属性步长
            UINT nVertexBufferByteSize;                       // 顶点数据字节数
            UINT nIndexBufferByteSize;                        // 索引数据字节数
            DXGI_FORMAT emIndexFormat = DXGI_FORMAT_R16_UINT; // 索引数据类型

            std::unordered_map<std::string, SubmeshGeometry> DrawArgs; // 几何体列表

            /// @brief 获取顶点缓冲视图
            /// @return
            D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
            {
                D3D12_VERTEX_BUFFER_VIEW view;
                view.BufferLocation = pGPUVertexBuffer->GetGPUVirtualAddress();
                view.SizeInBytes = nVertexBufferByteSize;
                view.StrideInBytes = nVertexByteStride;
                return view;
            }

            /// @brief 获取索引缓冲视图
            /// @return
            D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
            {
                D3D12_INDEX_BUFFER_VIEW view;
                view.BufferLocation = pGPUIndexBuffer->GetGPUVirtualAddress();
                view.Format = emIndexFormat;
                view.SizeInBytes = nIndexBufferByteSize;
                return view;
            }

            /// @brief 释放上传堆。上传数据至默认堆后即可调用
            void DisposeUploader()
            {
                pUploaderVertexBuffer = nullptr;
                pUploaderIndexBuffer = nullptr;
            }
        };
		
        /// @brief 帧资源结构体
        struct FrameResource
        {
			enum FrameResourceType		// 帧资源类型
			{
				CBTYPE_VERTEX,			// 顶点帧资源
				CBTYPE_MATERIAL			// 材质帧资源
			};	
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator; // 命令分配器
            D3DHelper::UploadBuffer CBScene;                                  // 场景数据
            D3DHelper::UploadBuffer CBObject;                                 // 对象数据
            D3DHelper::UploadBuffer CBVertex;                                 // 顶点数据
			D3DHelper::UploadBuffer CBMaterial;								  // 材质数据
            UINT iFence;

            FrameResource() : iFence(0){}

            /// @brief 初始化帧资源的常量缓冲区
            /// @param pDevice D3D12 设备指针
            /// @param nSceneCount 场景常量数据块的数量
            /// @param nObjectCount 对象常量数据块的数量
            void InitConstantBuffer(ID3D12Device *pDevice, UINT nSceneCount, UINT nObjectCount)
            {
                CBScene.Init(pDevice, D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::SceneConstant)), nSceneCount);
                CBObject.Init(pDevice, D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::ObjectConstant)), nObjectCount);
            }
			
			/// @brief 初始化帧资源其它常量缓冲区
            /// @param pDevice D3D12 设备指针
			/// @param type 缓冲区类型
            /// @param nSceneCount 场景常量数据块的数量
            /// @param nObjectCount 对象常量数据块的数量
			void InitOtherConstantBuffer(ID3D12Device* pDevice, FrameResourceType type, UINT nByteSize, UINT nCount)
			{
				switch(type)
				{
				case CBTYPE_VERTEX:
					CBVertex.Init(pDevice, nByteSize, nCount);
					break;
				case CBTYPE_MATERIAL:
					CBMaterial.Init(pDevice, D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::MaterialConstant)), nCount);
					break;
				}
			}
        };
		
	};
	
    namespace MathHelper
    {
        DirectX::XMFLOAT4X4 Identity4x4();
        
    };

    namespace CONSTANT_VALUE
    {
        static const UINT nCBObjectByteSize = D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::ObjectConstant));
        static const UINT nCBSceneByteSize  = D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::SceneConstant));
        static const UINT nCBMaterialByteSize = D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::MaterialConstant));
    };
};

namespace D3DHelper
{
	/// @brief 渲染项描述结构体
	struct RenderItem
	{
		// 使用脏标识(Dirty Flag)来表示物体相关数据已经发生变化
		// 由于每个帧资源都有一个常量缓冲区 所以我们必须对每一个帧资源中的数据进行更新
		// 当修改标识时，应当使该标识为当前程序的帧资源数量，即 iFrameDirty = 帧资源数量
		// 以确保每一个帧资源中的常量数据得到更新
		int iFramesDirty;

		// 描述渲染对象局部空间相对于世界空间的矩阵
		// 它包含了对象的在世界空间中的朝向、位置和大小
		DirectX::XMFLOAT4X4 matWorld;
		
		// 描述渲染对象的纹理坐标变换的矩阵
		DirectX::XMFLOAT4X4 matTexTransform;
		
		// 该数据标识当前渲染项对应于帧资源的常量缓冲区索引
		UINT nCBObjectIndex;

		// 此渲染项绑定的几何体数据。一个几何体数据可以被多个渲染项绑定
		Resource::MeshGeometry *pGeo;
		
		// 此渲染项绑定的材质数据。
		Light::Material* pMaterial;

		// 图元拓扑类型
		D3D12_PRIMITIVE_TOPOLOGY emPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// 渲染用参数
		UINT nIndexCount;         // 顶点索引的数量
		UINT nStartIndexLocation; // 索引位置起始值
		int nBaseVertexLocation;  // 顶点位置基值
	};
			
};
#endif