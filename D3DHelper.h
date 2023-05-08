/**
 * ��ͷ�ļ�������һЩ������ DirectX 3D �����Ĺ���
 * ��Ҫ��Ϊ���²���:
 * * ͨ�ò���(D3DHelper)
 * * ��Ⱦ�ò���(D3DHelper::Render)
 * * �����岿��(D3DHelper::Geometry)
 * ...
 *
 * ��������:
 * ������������������ͬʱ���ݱ��������������ͽ���������ǰ׺���й滮��
 *
 * - ��������������:
 * * һ�������ָ�������ڵ���ʱ����
 * * ���������ָ�ṹ������еĳ�Ա������ͬʱ��������ʽ������ʵ�ʲ���Ҳ����ʹ�øù淶
 *
 * - ����ǰ׺:
 * ** һ�����ǰ׺
 * * f:     float ������
 * * i:     int ����
 * * b:     byte �ֽ�
 * * p(lp): (long) pointer (��)ָ�룬Ҳ���Դ�������
 *
 * ** �������ǰ׺
 * * n:     number ��������, �����״θ�ֵ���䱣��ֵ��֮���������
 * * c:     count ����
 * * i:     int   ����, ��ֵ�ڳ��������в��ϸı�
 * * ix:    index ����
 * * em:    enum ö��
 * * p(lp): (long) pointer (��)ָ��, Ҳ���Դ�������
 * * vecX:  vector ����, X ����ά��
 * * mat:   matrix ����
 *
 * ** ���ڳ�Ա�ṹ��������Ա�����:
 * ��������ṹ���������ý���������
 *
 * - �������:
 * ** ������
 * ���� ��ǰ׺-������-������... ����ʽ��������, �������ж�Ϊ��д��ĸ
 * ��: DXEXCEPTION_MAXSTRING ��ǰ׺Ϊ DXEXCEPTION, �����ú���Ϊ DxException �����ģ�����Ϊ MaxString, �����ú���һ���쳣���ַ���󻺳������ֽ�����
 *
 * ** ����
 * ���ݺ��������ý�������������ÿ�����ʵ�����ĸ��д��
 * ��������֮���ơ�
 * ��: CreateDefaultBuffer ����, Create �����ú���Ϊ����/���ɺ����������ɶ���Ϊ Direct3D �е� Default Buffer(Ĭ�϶�)
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
    /// @brief ���� HLSL ��ɫ��
    /// @param lpszShaderFile �������ļ�����
    /// @param lpszDefines     �궨���ַ���
    /// @param lpszEntryPoint  ��ɫ����ڵ�
    /// @param lpszTarget      ��ɫ���汾
    /// @return                ��ɫ�������ƴ���
    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(LPCWSTR lpszShaderFile, const D3D_SHADER_MACRO *lpszDefines, const char *lpszEntryPoint, const char *lpszTarget);

    /// @brief ����Ĭ�϶Ѹ�������
    /// @param pDevice      D3D12 �豸
    /// @param pComList     �����б�
    /// @param pData        �ϴ�������
    /// @param nByteSize    ���ݵ��ֽ���
    /// @param pUploader    δ��ʼ�����ϴ���
    /// @return             Ĭ�϶�
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pComList, const void *pData, UINT nByteSize, ID3D12Resource **pUploader);
	
	/// @brief ���� DDS ����
	/// @param pDevice 		D3D12 �豸
	/// @param pComList     �����б�
	/// @param lpszFileName �����ļ���
	/// @param pUploader    δ��ʼ�����ϴ���
	/// @return 			����������Դ
    Microsoft::WRL::ComPtr<ID3D12Resource> LoadDDSFromFile(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pComList, LPCWSTR lpszFileName, ID3D12Resource** pUploader);
    
	/// @brief �ϴ��Ѹ�����
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
		/// @brief ���ʽṹ��
		struct Material
		{
			std::string Name;						// ������
			int nCBMaterialIndex;					// ��������Ϣ�ڳ���������������
			int nDiffuseSrvHeapIndex;				// ������������ SRV ���е�����
			int iFramesDirty;					    // �������ʶ
			
			// ������ɫ���ʵĳ�������������
			DirectX::XMFLOAT4 vec4DiffuseAlbedo;	// �����䷴����
			DirectX::XMFLOAT3 vec3FresnelR0;		// ��������R(0��)
			float nRoughness;						// ����ֲڶ�
			DirectX::XMFLOAT4X4 matTransform;
		};
		
		/// @brief ��Դ��������
		struct Light
		{
			DirectX::XMFLOAT3 vec3Strength;			// ��Դ��ɫ
			float nFalloffStart;					// ��Դ����˥����ʼ��Χ(���Դ/�۹�ƹ�Դ)
			DirectX::XMFLOAT3 vec3Direction;		// ��Դ����(�����Դ/�۹�ƹ�Դ)
			float nFalloffEnd;						// ��Դ����˥��������Χ(���Դ/�۹�ƹ�Դ)
			DirectX::XMFLOAT3 vec3Position;			// ��Դλ��(���Դ/�۹�ƹ�Դ)
			float nSpotPower;						// �۹������
		};
	};
	
    namespace Constant
	{
		/// @brief ���ʳ�������
		struct MaterialConstant
		{
			DirectX::XMFLOAT4 vec4DiffuseAlbedo;	// �����䷴����
			DirectX::XMFLOAT3 vec3FresnelR0;		// ��������R(0��)
			float nRoughness;						// ����ֲڶ�
			DirectX::XMFLOAT4X4 matTransform;
		};
		
		/// @brief ������������
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

        /// @brief ����������
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
		
        /// @brief ��Ⱦ���������ṹ��
        struct SubmeshGeometry
        {
            UINT nIndexCount;						// ������������
            UINT nStartIndexLocation;				// ����������ֵ
            INT nBaseVertexLocation;				// �����ֵ

            DirectX::BoundingBox Bounds;			// ��������ײ������
        };

        /// @brief ��Ⱦ���ݽṹ��
        struct MeshGeometry
        {
            std::string Name;

            Microsoft::WRL::ComPtr<ID3DBlob> pCPUVertexBuffer; // �������ݸ���
            Microsoft::WRL::ComPtr<ID3DBlob> pCPUIndexBuffer;  // �������ݸ���

            Microsoft::WRL::ComPtr<ID3D12Resource> pGPUVertexBuffer;      // ��������Ĭ�϶�
            Microsoft::WRL::ComPtr<ID3D12Resource> pGPUIndexBuffer;       // ��������Ĭ�϶�
            Microsoft::WRL::ComPtr<ID3D12Resource> pUploaderVertexBuffer; // ���������ϴ���
            Microsoft::WRL::ComPtr<ID3D12Resource> pUploaderIndexBuffer;  // ���������ϴ���

            UINT nVertexByteStride;                           // �������Բ���
            UINT nVertexBufferByteSize;                       // ���������ֽ���
            UINT nIndexBufferByteSize;                        // ���������ֽ���
            DXGI_FORMAT emIndexFormat = DXGI_FORMAT_R16_UINT; // ������������

            std::unordered_map<std::string, SubmeshGeometry> DrawArgs; // �������б�

            /// @brief ��ȡ���㻺����ͼ
            /// @return
            D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
            {
                D3D12_VERTEX_BUFFER_VIEW view;
                view.BufferLocation = pGPUVertexBuffer->GetGPUVirtualAddress();
                view.SizeInBytes = nVertexBufferByteSize;
                view.StrideInBytes = nVertexByteStride;
                return view;
            }

            /// @brief ��ȡ����������ͼ
            /// @return
            D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
            {
                D3D12_INDEX_BUFFER_VIEW view;
                view.BufferLocation = pGPUIndexBuffer->GetGPUVirtualAddress();
                view.Format = emIndexFormat;
                view.SizeInBytes = nIndexBufferByteSize;
                return view;
            }

            /// @brief �ͷ��ϴ��ѡ��ϴ�������Ĭ�϶Ѻ󼴿ɵ���
            void DisposeUploader()
            {
                pUploaderVertexBuffer = nullptr;
                pUploaderIndexBuffer = nullptr;
            }
        };
		
        /// @brief ֡��Դ�ṹ��
        struct FrameResource
        {
			enum FrameResourceType		// ֡��Դ����
			{
				CBTYPE_VERTEX,			// ����֡��Դ
				CBTYPE_MATERIAL			// ����֡��Դ
			};	
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator; // ���������
            D3DHelper::UploadBuffer CBScene;                                  // ��������
            D3DHelper::UploadBuffer CBObject;                                 // ��������
            D3DHelper::UploadBuffer CBVertex;                                 // ��������
			D3DHelper::UploadBuffer CBMaterial;								  // ��������
            UINT iFence;

            FrameResource() : iFence(0){}

            /// @brief ��ʼ��֡��Դ�ĳ���������
            /// @param pDevice D3D12 �豸ָ��
            /// @param nSceneCount �����������ݿ������
            /// @param nObjectCount ���������ݿ������
            void InitConstantBuffer(ID3D12Device *pDevice, UINT nSceneCount, UINT nObjectCount)
            {
                CBScene.Init(pDevice, D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::SceneConstant)), nSceneCount);
                CBObject.Init(pDevice, D3DHelper_CalcConstantBufferBytesSize(sizeof(Constant::ObjectConstant)), nObjectCount);
            }
			
			/// @brief ��ʼ��֡��Դ��������������
            /// @param pDevice D3D12 �豸ָ��
			/// @param type ����������
            /// @param nSceneCount �����������ݿ������
            /// @param nObjectCount ���������ݿ������
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
	/// @brief ��Ⱦ�������ṹ��
	struct RenderItem
	{
		// ʹ�����ʶ(Dirty Flag)����ʾ������������Ѿ������仯
		// ����ÿ��֡��Դ����һ������������ �������Ǳ����ÿһ��֡��Դ�е����ݽ��и���
		// ���޸ı�ʶʱ��Ӧ��ʹ�ñ�ʶΪ��ǰ�����֡��Դ�������� iFrameDirty = ֡��Դ����
		// ��ȷ��ÿһ��֡��Դ�еĳ������ݵõ�����
		int iFramesDirty;

		// ������Ⱦ����ֲ��ռ����������ռ�ľ���
		// �������˶����������ռ��еĳ���λ�úʹ�С
		DirectX::XMFLOAT4X4 matWorld;
		
		// ������Ⱦ�������������任�ľ���
		DirectX::XMFLOAT4X4 matTexTransform;
		
		// �����ݱ�ʶ��ǰ��Ⱦ���Ӧ��֡��Դ�ĳ�������������
		UINT nCBObjectIndex;

		// ����Ⱦ��󶨵ļ��������ݡ�һ�����������ݿ��Ա������Ⱦ���
		Resource::MeshGeometry *pGeo;
		
		// ����Ⱦ��󶨵Ĳ������ݡ�
		Light::Material* pMaterial;

		// ͼԪ��������
		D3D12_PRIMITIVE_TOPOLOGY emPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// ��Ⱦ�ò���
		UINT nIndexCount;         // ��������������
		UINT nStartIndexLocation; // ����λ����ʼֵ
		int nBaseVertexLocation;  // ����λ�û�ֵ
	};
			
};
#endif