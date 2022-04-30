#include "Graphics.h"

bool Graphics::Initialize( HWND hwnd, int width, int height )
{
	if ( !InitializeDirectX( hwnd, width, height ) )
	{
		return false;
	}

	if ( !InitializeShaders() )
	{
		return false;
	}

	return true;
}

void Graphics::RenderFrame()
{
	float bgcolor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	this->deviceContext->ClearRenderTargetView( this->renderTargetView.Get(), bgcolor );
	this->swapchain->Present( 1, NULL );
}

bool Graphics::InitializeDirectX( HWND hwnd, int width, int height )
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdpaters();

	if ( adapters.size() < 1 )
	{
		ErrorLogger::Log( "No IDXGI Adapters found." );
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory( &scd, sizeof( DXGI_SWAP_CHAIN_DESC ) );

	scd.BufferDesc.Width = width;
	scd.BufferDesc.Height = height;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hwnd;
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain( adapters[0].pAdapter,	// IDXGI Adapter
										D3D_DRIVER_TYPE_UNKNOWN,
										NULL,	// FOR SOFTWARE DRIVER TYPE
										NULL,	// FLAGS FOR RUNTIME LAYERS
										NULL,	// FEATURE LEVELS ARRAY
										0,	// # OF FEATURE LEVELS IN ARRAY
										D3D11_SDK_VERSION,
										&scd,	// Swapchain description
										this->swapchain.GetAddressOf(),
										this->device.GetAddressOf(),
										NULL,	// Supported feature level
										this->deviceContext.GetAddressOf() );	// Device Context Address

	if ( FAILED( hr ) )
	{
		ErrorLogger::Log( hr, "Failed to create device and swapchain." );
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
	hr = this->swapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( backbuffer.GetAddressOf() ) );
	if ( FAILED( hr ) )	// If error occured
	{
		ErrorLogger::Log( hr, "GetBuffer Failed." );
		return false;
	}

	hr = this->device->CreateRenderTargetView( backbuffer.Get(), NULL, this->renderTargetView.GetAddressOf() );
	if ( FAILED( hr ) )	// If error occured
	{
		ErrorLogger::Log( hr, "Failed to Create render target view." );
		return false;
	}

	this->deviceContext->OMSetRenderTargets( 1, this->renderTargetView.GetAddressOf(), NULL );

	return true;
}

bool Graphics::InitializeShaders()
{
	std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
	if ( IsDebuggerPresent() == TRUE )
	{
#ifdef _DEBUG	// Debug Mode
#ifdef _WIN64	// x64
		shaderfolder = L"../bin/x64/Debug/";
#else	// x84 (Win32)
		shaderfolder = L"../bin/x86/Debug/";
#endif
#else	// Release Mode
#ifdef _WIN64	// x64
		shaderfolder = L"../bin/x64/Release/";
#else
		shaderfolder = L"../bin/x86/Release/";
#endif // _WIN64
#endif // _DEBUG
	}
#pragma endregion

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE( layout );

	if ( !vertexshader.Initialize( this->device, shaderfolder + L"vertexshader.cso", layout, numElements ) )
	{
		return false;
	}

	return true;
}
