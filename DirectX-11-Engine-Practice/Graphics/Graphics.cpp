#include "Graphics.h"

bool Graphics::Initialize( HWND hwnd, int width, int height )
{
	this->windowWidth = width;
	this->windowHeight = height;
	this->fpsTimer.Start();

	if ( !InitializeDirectX( hwnd ) )
	{
		return false;
	}

	if ( !InitializeShaders() )
	{
		return false;
	}

	if ( !InitializeScene() )
	{
		return false;
	}

	return true;
}

void Graphics::RenderFrame()
{
	float bgcolor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	this->deviceContext->ClearRenderTargetView( this->renderTargetView.Get(), bgcolor );

	this->deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	this->deviceContext->RSSetState( this->rasterizerState.Get() );
	this->deviceContext->PSSetSamplers( 0, 1, this->samplerState.GetAddressOf() );
	
	//sprite
	this->deviceContext->IASetInputLayout(this->vertexshader_2d.GetInputLayout());
	this->deviceContext->PSSetShader(this->pixelshader_2d.GetShader(), NULL, 0);
	this->deviceContext->VSSetShader(vertexshader_2d.GetShader(), NULL, 0);

	if (sprites.size() > 1)
	{
		if (sprites.back()->GetRenderReadyFlag() == true)
		{
			std::vector<Sprite*>::iterator it = sprites.begin();
			delete* it;
			sprites.erase(it);
		}
	}

	for (std::vector<Sprite*>::iterator it = sprites.begin(); it != sprites.end(); it++)
	{
		(*it)->Draw(camera2D.GetWorldMatrix() * camera2D.GetOrthoMatrix());
	}

	//Draw Text
	static int fpsCounter = 0;
	static std::string fpsString = "FPS: 0";
	fpsCounter += 1;
	if ( this->fpsTimer.GetMilisecondsElapsed() > 1000.0 )
	{
		fpsString = "FPS: " + std::to_string( fpsCounter );
		fpsCounter = 0;
		this->fpsTimer.Restart();
	}
	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), StringHelper::StringToWide(fpsString).c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteFont->DrawString(spriteBatch.get(), StringHelper::StringToWide(std::to_string(sprites.size())).c_str(), DirectX::XMFLOAT2(0, 20), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();

	this->swapchain->Present( 0, NULL );
}

bool Graphics::InitializeDirectX( HWND hwnd )
{
	try
	{
		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

		if ( adapters.size() < 1 )
		{
			ErrorLogger::Log( "No IDXGI Adapters found." );
			return false;
		}

		DXGI_SWAP_CHAIN_DESC scd = { 0 };

		scd.BufferDesc.Width = this->windowWidth;
		scd.BufferDesc.Height = this->windowHeight;
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

		COM_ERROR_IF_FAILED( hr, "Failed to create device and swapchain." );

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
		hr = this->swapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( backbuffer.GetAddressOf() ) );
		COM_ERROR_IF_FAILED( hr, "GetBuffer Failed." );

		hr = this->device->CreateRenderTargetView( backbuffer.Get(), NULL, this->renderTargetView.GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to Create render target view." );

		this->deviceContext->OMSetRenderTargets( 1, this->renderTargetView.GetAddressOf(), nullptr );

		// Create & set the Viewport
		CD3D11_VIEWPORT viewport( 0.0f, 0.0f, static_cast<float>( this->windowWidth ), static_cast<float>( this->windowHeight ) );
		this->deviceContext->RSSetViewports( 1, &viewport );

		// Create Rasterizer State
		CD3D11_RASTERIZER_DESC rasterizerDesc( D3D11_DEFAULT );
		hr = this->device->CreateRasterizerState( &rasterizerDesc, this->rasterizerState.GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create rasterizer state." );

		spriteBatch = std::make_unique<DirectX::SpriteBatch>( this->deviceContext.Get() );
		spriteFont = std::make_unique<DirectX::SpriteFont>( this->device.Get(), L"../Data/Fonts/comic_sans_ms_16.spritefont" );

		// Create sampler description for sampler state
		CD3D11_SAMPLER_DESC sampDesc( D3D11_DEFAULT );
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;	// X Coordinate
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;	// Y Coordinate
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;	// Z Coordinate
		hr = this->device->CreateSamplerState( &sampDesc, this->samplerState.GetAddressOf() );	// Create sampler state.
		COM_ERROR_IF_FAILED( hr, "Failed to create sampler state." );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}
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

	//2d shaders
	D3D11_INPUT_ELEMENT_DESC layout2D[] =
	{
		{ "POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements2D = ARRAYSIZE(layout2D);

	if (!vertexshader_2d.Initialize(this->device, shaderfolder + L"vertexshader_2d.cso", layout2D, numElements2D))
	{
		return false;
	}

	if (!pixelshader_2d.Initialize(this->device, shaderfolder + L"pixelshader_2d_2.cso"))
	{
		return false;
	}

	return true;
}

bool Graphics::InitializeScene()
{
	try
	{
		HRESULT hr;
		// Initialize Constant Buffer(s)
		hr = this->cb_vs_vertexshader_2d.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		camera2D.SetProjectionValues(windowWidth, windowHeight, 0.0f, 1.0f);
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}
	return true;
}

bool Graphics::CreateSprite(std::string a_url)
{
	Sprite* sprite = new Sprite();

	if (!sprite->Initialize(this->device.Get(), this->deviceContext.Get(), windowWidth / 2, windowHeight / 2, cb_vs_vertexshader_2d, Color(200,0,0)))
	{
		return false;
	}

	sprite->SetUrl(a_url);

	HANDLE _handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)sprite->StartThread, sprite, 0, NULL);
	if (_handle != nullptr)
	{
		CloseHandle(_handle);
	}

	sprites.push_back(sprite);

	return true;
}

void Graphics::DeleteSprite()
{
	if (sprites.size() > 0)
	{
		if (sprites.back()->GetRenderReadyFlag() == true)
		{
			std::vector<Sprite*>::iterator it = sprites.begin();
			delete* it;
			sprites.erase(sprites.begin());
		}
	}
}
