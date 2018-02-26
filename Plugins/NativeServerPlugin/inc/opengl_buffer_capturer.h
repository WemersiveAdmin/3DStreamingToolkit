#pragma once

#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>
#include <freeglut.h>
#include <glut.h>
#include "glext.h"

#include "buffer_capturer.h"

namespace StreamingToolkit
{
	// Provides OpenGL implementation of the BufferCapturer class.
	class OpenGLBufferCapturer : public BufferCapturer
	{
	public:
		OpenGLBufferCapturer(int width, int height);

		virtual ~OpenGLBufferCapturer() {}

		void SendFrame();

	private:
		UINT buffer_width;
		UINT buffer_height;
	};
}
