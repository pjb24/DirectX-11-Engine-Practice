#include "Shaders.h"

bool VertexShader::Initialize( Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderpath )
{
	HRESULT hr = D3DReadFileToBlob( shaderpath.c_str(), this->shader_buffer.GetAddressOf() );
	if ( FAILED( hr ) )
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log( hr, errorMsg );

		return false;
	}

	hr = device->CreateVertexShader( this->shader_buffer->GetBufferPointer(), this->shader_buffer->GetBufferSize(), NULL, this->shader.GetAddressOf() );
	if ( FAILED( hr ) )
	{
		std::wstring errorMsg = L"Failed to create vertex shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log( hr, errorMsg );

		return false;
	}

	return true;
}

ID3D11VertexShader* VertexShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* VertexShader::GetBuffer()
{
	return this->shader_buffer.Get();
}
