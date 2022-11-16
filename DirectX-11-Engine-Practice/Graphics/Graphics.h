#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "../Timer.h"
#include "Camera2D.h"
#include "Sprite.h"

class Graphics
{
public:
	bool Initialize( HWND hwnd, int width, int height );
	void RenderFrame();
	Camera2D camera2D;
	std::vector<Sprite*> sprites;

	bool CreateSprite(std::string a_url);
	void DeleteSprite();

private:
	bool InitializeDirectX( HWND hwnd );
	bool InitializeShaders();
	bool InitializeScene();

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

	VertexShader vertexshader_2d;
	PixelShader pixelshader_2d;
	ConstantBuffer<CB_VS_vertexshader_2d> cb_vs_vertexshader_2d;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	int windowWidth = 0;
	int windowHeight = 0;
	Timer fpsTimer;
};
