#pragma once
#include "GameObject2D.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include <string>
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Vertex.h"
#include <queue>
#include <thread>
#include <mutex>
#include "../Timer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

class Sprite : public GameObject2D
{
public:
	~Sprite();
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d, Color color);
	void Draw(XMMATRIX orthoMatrix);	//2d camera orthogonal matrix
	float GetWidth();
	float GetHeight();

	bool UpdateTexture(ID3D11DeviceContext* deviceContext, uint8_t* a_pData1, int a_linesize1, uint8_t* a_pData2, int a_linesize2, uint8_t* a_pData3, int a_linesize3);

	bool RtspConnect();

	bool GetRenderReadyFlag();

	static void StartThread(Sprite* a_this);

	void SetUrl(std::string a_url);

private:
	void UpdateMatrix() override;

	ConstantBuffer<CB_VS_vertexshader_2d>* cb_vs_vertexshader_2d = nullptr;
	XMMATRIX worldMatrix = XMMatrixIdentity();

	std::unique_ptr<Texture> texture;
	ID3D11DeviceContext* deviceContext = nullptr;

	IndexBuffer indices;
	VertexBuffer<Vertex2D> vertices;
	//--------------------------------
	bool initializeCodec(int a_streamIndex);
	static void readThread(Sprite* a_this);
	static void decodeThread(Sprite* a_this);

	AVFormatContext* m_pFormatContext = nullptr;
	AVCodecContext* m_pCodecContext = nullptr;
	AVCodec* m_pCodec = nullptr;

	int m_streamIndex[AVMEDIA_TYPE_NB] = { -1, -1, -1, -1, -1};
	//--------------------------------
	void setReadFlag(bool a_readFlag);
	bool getReadFlag();
	std::mutex m_readFlagMutex;
	bool m_readFlag = false;
	//--------------------------------
	void setDecodeFlag(bool a_decodeFlag);
	bool getDecodeFlag();
	std::mutex m_decodeFlagMutex;
	bool m_decodeFlag = false;
	//--------------------------------
	void pushPacket(AVPacket* a_packet);
	void popPacket(AVPacket*& a_packet);
	std::queue<AVPacket*> m_packets;
	std::mutex m_packetMutex;
	AVPacket* m_packet = nullptr;
	//--------------------------------
	void pushFrame(AVFrame* a_frame);
	void popFrame(AVFrame*& a_frame);
	std::queue<AVFrame*> m_frames;
	std::mutex m_frameMutex;
	AVFrame* m_frame = nullptr;
	//--------------------------------
	void setRenderReadyFlag(bool a_renderReadyFlag);
	std::mutex m_renderReadyFlagMutex;
	bool m_renderReadyFlag = false;
	//--------------------------------
	HANDLE m_readThreadHandle = nullptr;
	HANDLE m_decodeThreadHandle = nullptr;
	//--------------------------------
	std::string m_url;
	//--------------------------------
	Timer m_readTimer;
	Timer m_decodeTimer;
};
