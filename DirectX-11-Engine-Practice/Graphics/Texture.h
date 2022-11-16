#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Color.h"
#include <string>

class Texture
{
public:
	Texture(ID3D11Device* device, const Color& color);
	Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height);
	Texture(ID3D11Device* device, UINT width, UINT height);

	bool UpdateTexture(ID3D11DeviceContext* deviceContext, uint8_t* a_pData1, int a_linesize1, uint8_t* a_pData2, int a_linesize2, uint8_t* a_pData3, int a_linesize3);

	ID3D11ShaderResourceView* GetTextureResourceYView();
	ID3D11ShaderResourceView** GetTextureResourceYViewAddress();
	ID3D11ShaderResourceView* GetTextureResourceUView();
	ID3D11ShaderResourceView** GetTextureResourceUViewAddress();
	ID3D11ShaderResourceView* GetTextureResourceVView();
	ID3D11ShaderResourceView** GetTextureResourceVViewAddress();

private:
	void Initialize1x1ColorTexture(ID3D11Device* device, const Color& colorData);
	void InitializeColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height);

	Microsoft::WRL::ComPtr<ID3D11Resource> textureY = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Resource> textureU = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Resource> textureV = nullptr;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureViewY = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureViewU = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureViewV = nullptr;

	int width = 0;
	int height = 0;
};
