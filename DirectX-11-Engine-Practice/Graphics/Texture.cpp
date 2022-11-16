#include "Texture.h"
#include "../ErrorLogger.h"

Texture::Texture(ID3D11Device* device, const Color& color)
{
	this->Initialize1x1ColorTexture(device, color);
}

Texture::Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height)
{
	this->InitializeColorTexture(device, colorData, width, height);
}

Texture::Texture(ID3D11Device* device, UINT width, UINT height)
{
	this->InitializeColorTexture(device, nullptr, width, height);
}

bool Texture::UpdateTexture(ID3D11DeviceContext* deviceContext, uint8_t* a_pData1, int a_linesize1, uint8_t* a_pData2, int a_linesize2, uint8_t* a_pData3, int a_linesize3)
{
	HRESULT hr = S_OK;
	PBYTE p = NULL;
	BYTE* ptr = NULL;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = deviceContext->Map(textureY.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to map constant buffer.");
		return false;
	}
	p = (PBYTE)mappedResource.pData;
	ptr = a_pData1;
	// for 시작 - 이미지의 높이만큼 반복
	for (int i = 0; i < this->height; i++)
	{
		memcpy(p, &ptr[i * a_linesize1], this->width);
		p = p + mappedResource.RowPitch;
	}
	deviceContext->Unmap(textureY.Get(), 0);

	hr = deviceContext->Map(textureU.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to map constant buffer.");
		return false;
	}
	p = (PBYTE)mappedResource.pData;
	ptr = a_pData2;
	for (int i = 0; i < this->height / 2; i++)
	{
		memcpy(p, &ptr[i * a_linesize2], this->width / 2);
		p = p + mappedResource.RowPitch;
	}
	deviceContext->Unmap(textureU.Get(), 0);

	hr = deviceContext->Map(textureV.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to map constant buffer.");
		return false;
	}
	p = (PBYTE)mappedResource.pData;
	ptr = a_pData3;
	for (int i = 0; i < this->height / 2; i++)
	{
		memcpy(p, &ptr[i * a_linesize3], this->width / 2);
		p = p + mappedResource.RowPitch;
	}
	deviceContext->Unmap(textureV.Get(), 0);

	return true;
}

ID3D11ShaderResourceView* Texture::GetTextureResourceYView()
{
	return this->textureViewY.Get();
}

ID3D11ShaderResourceView** Texture::GetTextureResourceYViewAddress()
{
	return this->textureViewY.GetAddressOf();
}

ID3D11ShaderResourceView* Texture::GetTextureResourceUView()
{
	return this->textureViewU.Get();
}

ID3D11ShaderResourceView** Texture::GetTextureResourceUViewAddress()
{
	return this->textureViewU.GetAddressOf();
}

ID3D11ShaderResourceView* Texture::GetTextureResourceVView()
{
	return this->textureViewV.Get();
}

ID3D11ShaderResourceView** Texture::GetTextureResourceVViewAddress()
{
	return this->textureViewV.GetAddressOf();
}

void Texture::Initialize1x1ColorTexture(ID3D11Device* device, const Color& colorData)
{
	InitializeColorTexture(device, &colorData, 1, 1);
}

void Texture::InitializeColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height)
{
	ID3D11Texture2D* p2DTextureY = nullptr;
	ID3D11Texture2D* p2DTextureU = nullptr;
	ID3D11Texture2D* p2DTextureV = nullptr;

	CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_R8_UNORM, width, height);
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MipLevels = 1;

	this->width = width;
	this->height = height;

	HRESULT hr = S_OK;
	
	hr = device->CreateTexture2D(&textureDesc, NULL, &p2DTextureY);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize texture from color data.");
	textureY = static_cast<ID3D11Texture2D*>(p2DTextureY);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDescY(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	srvDescY.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(textureY.Get(), &srvDescY, textureViewY.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create shader resource view from texture generated from color data.");

	textureDesc.Width = width / 2;
	textureDesc.Height = height / 2;

	hr = device->CreateTexture2D(&textureDesc, NULL, &p2DTextureU);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize texture from color data.");
	textureU = static_cast<ID3D11Texture2D*>(p2DTextureU);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDescU(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	srvDescU.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(textureU.Get(), &srvDescU, textureViewU.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create shader resource view from texture generated from color data.");

	hr = device->CreateTexture2D(&textureDesc, NULL, &p2DTextureV);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize texture from color data.");
	textureV = static_cast<ID3D11Texture2D*>(p2DTextureV);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDescV(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	srvDescV.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(textureV.Get(), &srvDescV, textureViewV.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create shader resource view from texture generated from color data.");
}
