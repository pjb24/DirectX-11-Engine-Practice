#include "Sprite.h"

Sprite::~Sprite()
{
	setReadFlag(false);
	WaitForSingleObject(m_readThreadHandle, INFINITE);

	setDecodeFlag(false);
	WaitForSingleObject(m_decodeThreadHandle, INFINITE);

	int _packetSize = m_packets.size();
	for (size_t i = 0; i < _packetSize; i++)
	{
		popPacket(m_packet);
		av_packet_unref(m_packet);
	}
	av_packet_free(&m_packet);

	int _frameSize = m_frames.size();
	for (size_t i = 0; i < _frameSize; i++)
	{
		popFrame(m_frame);
		av_frame_unref(m_frame);
	}
	av_frame_free(&m_frame);

	avcodec_free_context(&m_pCodecContext);

	avformat_free_context(m_pFormatContext);
}

bool Sprite::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d, Color color)
{
	this->deviceContext = deviceContext;
	if (deviceContext == nullptr)
	{
		return false;
	}
	this->cb_vs_vertexshader_2d = &cb_vs_vertexshader_2d;

	//texture = std::make_unique<Texture>(device, color);
	texture = std::make_unique<Texture>(device, 1920, 1080);

	std::vector<Vertex2D> vertexData =
	{
		Vertex2D(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f),	//Top Left
		Vertex2D(1.0f, -1.0f, 0.0f, 1.0f, 0.0f),	//Top Right
		Vertex2D(-1.0f, 1.0f, 0.0f, 0.0f, 1.0f),	//Bottom Lefts
		Vertex2D(1.0f, 1.0f, 0.0f, 1.0f, 1.0f),	//Bottom Right
	};

	std::vector<DWORD> indexData =
	{
		0, 1, 2,
		2, 1, 3
	};

	HRESULT hr = vertices.Initialize(device, vertexData.data(), vertexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for sprite.");

	hr = indices.Initialize(device, indexData.data(), indexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for sprite.");

	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);

	SetScale(width, height);

	return true;
}

void Sprite::Draw(XMMATRIX orthoMatrix)
{
	XMMATRIX wvpMatrix = worldMatrix * orthoMatrix;
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader_2d->GetAddressOf());
	cb_vs_vertexshader_2d->data.wvpMatrix = wvpMatrix;
	cb_vs_vertexshader_2d->ApplyChanges();

	popFrame(m_frame);
	if (m_frame != nullptr && m_frame->data[0] != 0)
	{
		UpdateTexture(deviceContext, m_frame->data[0], m_frame->linesize[0], m_frame->data[1], m_frame->linesize[1], m_frame->data[2], m_frame->linesize[2]);
		av_frame_unref(m_frame);
		if (GetRenderReadyFlag() == false)
		{
			setRenderReadyFlag(true);
		}
	}
	else
	{
		OutputDebugString(L"Frame is NULL \n");
	}

	if (GetRenderReadyFlag() == true)
	{
		ID3D11ShaderResourceView* textures[] = { texture->GetTextureResourceYView(), texture->GetTextureResourceUView(), texture->GetTextureResourceVView() };

		deviceContext->PSSetShaderResources(0, 3, textures);

		const UINT offsets = 0;
		deviceContext->IASetVertexBuffers(0, 1, vertices.GetAddressOf(), vertices.StridePtr(), &offsets);
		deviceContext->IASetIndexBuffer(indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
		deviceContext->DrawIndexed(indices.IndexCount(), 0, 0);
	}
}

float Sprite::GetWidth()
{
	return scale.x;
}

float Sprite::GetHeight()
{
	return scale.y;
}

bool Sprite::UpdateTexture(ID3D11DeviceContext* deviceContext, uint8_t* a_pData1, int a_linesize1, uint8_t* a_pData2, int a_linesize2, uint8_t* a_pData3, int a_linesize3)
{
	return texture->UpdateTexture(deviceContext, a_pData1, a_linesize1, a_pData2, a_linesize2, a_pData3, a_linesize3);
}

void Sprite::UpdateMatrix()
{
	worldMatrix = XMMatrixScaling(scale.x, scale.y, 1.0f) * XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x + scale.x / 2.0f, pos.y + scale.y / 2.0f, pos.z);
}

bool Sprite::RtspConnect()
{
	int ret = 0;

	ret = avformat_open_input(&m_pFormatContext, m_url.c_str(), NULL, nullptr);
	if (ret != 0)
	{
		return false;
	}
	//----------------------------------------------------------------
	ret = avformat_find_stream_info(m_pFormatContext, NULL);
	if (ret < 0)
	{
		return false;
	}

	m_streamIndex[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(m_pFormatContext, AVMEDIA_TYPE_VIDEO, m_streamIndex[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
	m_streamIndex[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(m_pFormatContext, AVMEDIA_TYPE_AUDIO, m_streamIndex[AVMEDIA_TYPE_AUDIO], m_streamIndex[AVMEDIA_TYPE_VIDEO], NULL, 0);
	//----------------------------------------------------------------

	ret = this->initializeCodec(m_streamIndex[AVMEDIA_TYPE_VIDEO]);
	if (ret != true)
	{
		return false;
	}

	m_packet = av_packet_alloc();
	setReadFlag(true);
	m_readThreadHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)readThread, this, 0, NULL);

	m_frame = av_frame_alloc();
	setDecodeFlag(true);
	m_decodeThreadHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)decodeThread, this, 0, NULL);

	return true;
}

bool Sprite::initializeCodec(int a_streamIndex)
{
	int ret = 0;

	if (a_streamIndex < 0 || a_streamIndex >= m_pFormatContext->nb_streams)
	{
		return false;
	}

	m_pCodecContext = avcodec_alloc_context3(NULL);
	if (m_pCodecContext == nullptr)
	{
		return AVERROR(ENOMEM);
	}

	ret = avcodec_parameters_to_context(m_pCodecContext, m_pFormatContext->streams[a_streamIndex]->codecpar);
	if (ret < 0)
	{
		avcodec_free_context(&m_pCodecContext);
		return false;
	}

	m_pCodecContext->pkt_timebase = m_pFormatContext->streams[a_streamIndex]->time_base;

	m_pCodec = avcodec_find_decoder(m_pCodecContext->codec_id);
	if (m_pCodec == nullptr)
	{
		avcodec_free_context(&m_pCodecContext);
		return AVERROR(EINVAL);
	}

	ret = avcodec_open2(m_pCodecContext, m_pCodec, NULL);
	if (ret < 0)
	{
		avcodec_free_context(&m_pCodecContext);
		return false;
	}

	switch (m_pCodecContext->codec_type)
	{
	case AVMEDIA_TYPE_VIDEO:
	{

		break;
	}
	default:
	{
		break;
	}
	}

	return true;
}

void Sprite::readThread(Sprite* a_this)
{
	int ret = 0;
	AVPacket* _packet = nullptr;
	static int _readCount = 0;

	a_this->m_readTimer.Start();

	_packet = av_packet_alloc();
	if (_packet == nullptr)
	{
		//return false;
		return;
	}

	while (a_this->getReadFlag())
	{
		ret = av_read_frame(a_this->m_pFormatContext, _packet);
		if (ret < 0)
		{
			continue;
		}

		if (_packet->stream_index == a_this->m_streamIndex[AVMEDIA_TYPE_VIDEO])
		{
			a_this->pushPacket(_packet);
			_readCount++;
		}
		else
		{
			av_packet_unref(_packet);
		}

		//if (a_this->m_readTimer.GetMilisecondsElapsed() > 1000)
		//{
		//	static std::wstring _readString;
		//	_readString += L"Read Count: " + std::to_wstring(_readCount) + L"\n";
		//	OutputDebugString(_readString.c_str());
		//	_readCount = 0;
		//	a_this->m_readTimer.Restart();
		//}
	}

	av_packet_free(&_packet);

	//return true;
}

void Sprite::decodeThread(Sprite* a_this)
{
	int ret = 0;
	bool _packetPending = false;
	AVFrame* _frame = nullptr;
	static int _decodeCount = 0;

	AVRational timebase = a_this->m_pFormatContext->streams[a_this->m_streamIndex[AVMEDIA_TYPE_VIDEO]]->time_base;
	AVRational frameRate = av_guess_frame_rate(a_this->m_pFormatContext, a_this->m_pFormatContext->streams[a_this->m_streamIndex[AVMEDIA_TYPE_VIDEO]], NULL);

	_frame = av_frame_alloc();
	if (_frame == nullptr)
	{
		return;
	}
	a_this->m_decodeTimer.Start();

	while (a_this->getDecodeFlag())
	{
		if (_packetPending == false)
		{
			a_this->popPacket(a_this->m_packet);
		}
		else
		{
			_packetPending = false;
		}

		// packetQueue size is zero
		if (a_this->m_packet == nullptr || a_this->m_packet->buf == 0)
		{
			continue;
		}

		ret = avcodec_send_packet(a_this->m_pCodecContext, a_this->m_packet);
		if (ret == AVERROR(EAGAIN))
		{
			// receive 후 send 다시 시도
			_packetPending = true;
		}
		else
		{
			av_packet_unref(a_this->m_packet);
		}

		ret = avcodec_receive_frame(a_this->m_pCodecContext, _frame);
		if (ret == AVERROR_EOF)
		{
			// End Of File
		}
		else if (ret == AVERROR(EAGAIN))
		{
			// send 후 receive 다시 시도
		}
		else
		{
			a_this->pushFrame(_frame);
			_decodeCount++;
		}

		//if (a_this->m_decodeTimer.GetMilisecondsElapsed() > 1000)
		//{
		//	static std::wstring _decodeString;
		//	_decodeString += L"Decode Count: " + std::to_wstring(_decodeCount) + L"\n";
		//	OutputDebugString(_decodeString.c_str());
		//	_decodeCount = 0;
		//	a_this->m_decodeTimer.Restart();
		//}
	}

	av_frame_free(&_frame);
}

void Sprite::setReadFlag(bool a_readFlag)
{
	std::lock_guard<std::mutex> lock(m_readFlagMutex);

	m_readFlag = a_readFlag;
}

bool Sprite::getReadFlag()
{
	std::lock_guard<std::mutex> lock(m_readFlagMutex);

	return m_readFlag;
}

void Sprite::setDecodeFlag(bool a_decodeFlag)
{
	std::lock_guard<std::mutex> lock(m_decodeFlagMutex);

	m_decodeFlag = a_decodeFlag;
}

bool Sprite::getDecodeFlag()
{
	std::lock_guard<std::mutex> lock(m_decodeFlagMutex);

	return m_decodeFlag;
}

void Sprite::pushPacket(AVPacket* a_packet)
{
	AVPacket* _packet = av_packet_alloc();

	av_packet_move_ref(_packet, a_packet);

	m_packetMutex.lock();
	m_packets.push(_packet);
	m_packetMutex.unlock();
}

void Sprite::popPacket(AVPacket*& a_packet)
{
	AVPacket* _packet = nullptr;

	m_packetMutex.lock();
	if (m_packets.size() == 0)
	{
		m_packetMutex.unlock();
		return;
	}

	_packet = m_packets.front();
	av_packet_move_ref(a_packet, _packet);
	m_packets.pop();
	av_packet_free(&_packet);
	m_packetMutex.unlock();
}

void Sprite::pushFrame(AVFrame* a_frame)
{
	AVFrame* _frame = av_frame_alloc();

	av_frame_move_ref(_frame, a_frame);

	m_frameMutex.lock();
	m_frames.push(_frame);
	m_frameMutex.unlock();
}

void Sprite::popFrame(AVFrame*& a_frame)
{
	AVFrame* _frame = nullptr;

	m_frameMutex.lock();
	if (m_frames.size() == 0)
	{
		m_frameMutex.unlock();
		return;
	}

	if (m_frames.size() > 2)
	{
		int _frameSize = m_frames.size();
		for (size_t i = 0; i < _frameSize - 1; i++)
		{
			_frame = m_frames.front();
			m_frames.pop();
			av_frame_free(&_frame);
		}
	}

	_frame = m_frames.front();
	av_frame_move_ref(a_frame, _frame);
	m_frames.pop();
	av_frame_free(&_frame);
	m_frameMutex.unlock();
}

void Sprite::setRenderReadyFlag(bool a_renderReadyFlag)
{
	std::lock_guard<std::mutex> lock(m_renderReadyFlagMutex);

	m_renderReadyFlag = a_renderReadyFlag;
}

bool Sprite::GetRenderReadyFlag()
{
	std::lock_guard<std::mutex> lock(m_renderReadyFlagMutex);

	return m_renderReadyFlag;
}

void Sprite::StartThread(Sprite* a_this)
{
	a_this->RtspConnect();

	a_this->SetPosition(XMFLOAT3(a_this->scale.x - a_this->GetWidth() / 2, a_this->scale.y - a_this->GetHeight() / 2, 0));
}

void Sprite::SetUrl(std::string a_url)
{
	m_url = a_url;
}
