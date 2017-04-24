﻿#include "pch.h"
#include "AppCallbacks.h"
#include "libyuv/convert.h"

using namespace WebRTCDirectXClientComponent;
using namespace Platform;

static uint8_t* s_videoYUVFrame = nullptr;
static uint8_t* s_videoRGBFrame = nullptr;
static uint8_t* s_videoDataY = nullptr;
static uint8_t* s_videoDataU = nullptr;
static uint8_t* s_videoDataV = nullptr;

AppCallbacks::AppCallbacks() :
	m_videoRenderer(nullptr),
	m_videoDecoder(nullptr)
{
}

AppCallbacks::~AppCallbacks()
{
	delete m_videoRenderer;
	delete m_videoDecoder;
	delete []s_videoYUVFrame;
	delete []s_videoRGBFrame;
	delete []s_videoDataY;
	delete []s_videoDataU;
	delete []s_videoDataV;
}

void AppCallbacks::Initialize(CoreApplicationView^ appView)
{
	m_deviceResources = std::make_shared<DX::DeviceResources>();
}

void AppCallbacks::SetWindow(CoreWindow^ window)
{
	m_deviceResources->SetWindow(window);
}

void AppCallbacks::Run()
{
	while (true)
	{
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(
			CoreProcessEventsOption::ProcessAllIfPresent);
	}
}

void AppCallbacks::OnFrame(
	uint32_t width,
	uint32_t height,
	const Array<uint8_t>^ dataY,
	uint32_t strideY,
	const Array<uint8_t>^ dataU,
	uint32_t strideU,
	const Array<uint8_t>^ dataV,
	uint32_t strideV)
{
	if (!s_videoRGBFrame)
	{
		m_videoRenderer = new VideoRenderer(m_deviceResources, width, height);
		s_videoRGBFrame = new uint8_t[width * height * 4];
	}

	libyuv::I420ToARGB(
		dataY->Data,
		strideY,
		dataU->Data,
		strideU,
		dataV->Data,
		strideV,
		s_videoRGBFrame,
		width * 4,
		width,
		height);
}

void AppCallbacks::OnEncodedFrame(
	uint32_t width,
	uint32_t height,
	const Array<uint8_t>^ encodedData)
{
	// Uses lazy initialization.
	if (!m_videoRenderer)
	{
		// Initializes the video renderer.
		m_videoRenderer = new VideoRenderer(m_deviceResources, width, height);

		// Initializes the video decoder.
		m_videoDecoder = new VideoDecoder();
		m_videoDecoder->Initialize(width, height);

		// Initalizes the temp buffers.
		int bufferSize = width * height * 4;
		int half_width = (width + 1) / 2;
		size_t size_y = static_cast<size_t>(width) * height;
		size_t size_uv = static_cast<size_t>(half_width) * ((height + 1) / 2);

		s_videoRGBFrame = new uint8_t[bufferSize];
		memset(s_videoRGBFrame, 0, bufferSize);

		s_videoYUVFrame = new uint8_t[bufferSize];
		memset(s_videoYUVFrame, 0, bufferSize);

		s_videoDataY = new uint8_t[size_y];
		memset(s_videoDataY, 0, size_y);

		s_videoDataU = new uint8_t[size_uv];
		memset(s_videoDataY, 0, size_uv);

		s_videoDataV = new uint8_t[size_uv];
		memset(s_videoDataY, 0, size_uv);
	}

	int decodedLength = 0;
	if (!m_videoDecoder->DecodeH264VideoFrame(
		encodedData->Data,
		encodedData->Length,
		width,
		height,
		&s_videoYUVFrame,
		&decodedLength))
	{
		int strideY = 0;
		int strideU = 0;
		int strideV = 0;

		ReadI420Buffer(
			width,
			height,
			s_videoYUVFrame,
			&s_videoDataY,
			&strideY,
			&s_videoDataU,
			&strideU,
			&s_videoDataV,
			&strideV);

		libyuv::I420ToARGB(
			s_videoDataY,
			strideY,
			s_videoDataU,
			strideU,
			s_videoDataV,
			strideV,
			s_videoRGBFrame,
			width * 4,
			width,
			height);

		m_videoRenderer->Render(s_videoRGBFrame);
		m_deviceResources->Present();
	}
}

void AppCallbacks::ReadI420Buffer(
	int width,
	int height,
	uint8* buffer,
	uint8** dataY,
	int* strideY,
	uint8** dataU,
	int* strideU,
	uint8** dataV,
	int* strideV)
{
	int half_width = (width + 1) / 2;
	size_t size_y = static_cast<size_t>(width) * height;
	size_t size_uv = static_cast<size_t>(half_width) * ((height + 1) / 2);
	*strideY = width;
	*strideU = half_width;
	*strideV = half_width;
	memcpy(*dataY, buffer, size_y);
	memcpy(*dataU, buffer + *strideY * height, size_uv);
	memcpy(*dataV, buffer + *strideY * height + *strideU * ((height + 1) / 2), size_uv);
}
