
#include <Windows.h>

#include "glew.h"
#include "wglew.h"

#include <assert.h>

LRESULT CALLBACK
_fake_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProcA(hwnd, msg, wparam, lparam);
}

bool
error()
{
	GLenum err = glGetError();
	switch (err)
	{
	case GL_INVALID_ENUM:
		assert(false && "invalid enum value was passed");
		return false;

	case GL_INVALID_VALUE:
		assert(false && "invalid value was passed");
		return false;

	case GL_INVALID_OPERATION:
		assert(false && "invalid operation at the current state of opengl");
		return false;

	case GL_INVALID_FRAMEBUFFER_OPERATION:
		assert(false && "invalid framebuffer operation");
		return false;

	case GL_OUT_OF_MEMORY:
		assert(false && "out of memory");
		return false;

	case GL_STACK_UNDERFLOW:
		assert(false && "stack underflow");
		return false;

	case GL_STACK_OVERFLOW:
		assert(false && "stack overflow");
		return false;

	case GL_NO_ERROR:
	default:
		return true;
	}
}

struct win_gl
{
	HWND handle;
	HDC dc;
	HGLRC context;
};

win_gl
offline_win_create(int gl_major, int gl_minor)
{
	//Setup Window Class that we'll use down there
	WNDCLASSEXA wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEXA));
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = _fake_window_proc;
	wc.hInstance = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "hiddenWindowClass";

	RegisterClassExA(&wc);

	// 1 pixel window dimension since all the windows we'll be doing down there are hidden
	RECT wr = { 0, 0, LONG(1), LONG(1) };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	// The first step in creating Modern GL Context is to create Legacy GL Context
	// so we setup fake window and dc to create the legacy context so that we could
	// initialize GLEW which will use wglGetProcAddress to load Modern OpenGL implmentation
	// off the GPU driver
	HWND fake_wnd = CreateWindowExA(
		NULL,
		"hiddenWindowClass",
		"Fake Window",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		NULL,
		NULL);

	HDC fake_dc = GetDC(fake_wnd);

	PIXELFORMATDESCRIPTOR fake_pfd;
	ZeroMemory(&fake_pfd, sizeof(fake_pfd));
	fake_pfd.nSize = sizeof(fake_pfd);
	fake_pfd.nVersion = 1;
	fake_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_STEREO_DONTCARE;
	fake_pfd.iPixelType = PFD_TYPE_RGBA;
	fake_pfd.cColorBits = 32;
	fake_pfd.cAlphaBits = 8;
	fake_pfd.cDepthBits = 24;
	fake_pfd.iLayerType = PFD_MAIN_PLANE;

	int fake_pfdid = ChoosePixelFormat(fake_dc, &fake_pfd);
	assert(fake_pfdid);

	bool result = SetPixelFormat(fake_dc, fake_pfdid, &fake_pfd);
	assert(result);

	HGLRC fake_ctx = wglCreateContext(fake_dc);
	assert(fake_ctx);

	result = wglMakeCurrent(fake_dc, fake_ctx);
	assert(result);


	// At last GLEW initialized
	GLenum glew_result = glewInit();
	assert(glew_result == GLEW_OK);

	// now create the hidden window and dc which will be attached to opengl context
	win_gl win{};
	win.handle = CreateWindowExA(
		NULL,
		"hiddenWindowClass",
		"GL Context Window",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		NULL,
		NULL);
	win.dc = GetDC(win.handle);
	assert(win.handle);

	// setup the modern pixel format in order to create the modern GL Context
	const int pixel_attribs[] = { WGL_DRAW_TO_WINDOW_ARB,
								 GL_TRUE,
								 WGL_SUPPORT_OPENGL_ARB,
								 GL_TRUE,
								 WGL_DOUBLE_BUFFER_ARB,
								 GL_TRUE,
								 WGL_PIXEL_TYPE_ARB,
								 WGL_TYPE_RGBA_ARB,
								 WGL_ACCELERATION_ARB,
								 WGL_FULL_ACCELERATION_ARB,
								 WGL_COLOR_BITS_ARB,
								 32,
								 WGL_ALPHA_BITS_ARB,
								 8,
								 WGL_DEPTH_BITS_ARB,
								 24,
								 WGL_STENCIL_BITS_ARB,
								 8,
								 WGL_SAMPLE_BUFFERS_ARB,
								 GL_TRUE,
								 WGL_SAMPLES_ARB,
								 2,
								 0,
								 0 };

	int pixel_format_id;
	UINT num_formats;
	bool status = wglChoosePixelFormatARB(win.dc, pixel_attribs, NULL, 1, &pixel_format_id, &num_formats);
	assert(status && num_formats > 0);

	PIXELFORMATDESCRIPTOR pixel_format{};
	DescribePixelFormat(win.dc, pixel_format_id, sizeof(pixel_format), &pixel_format);
	SetPixelFormat(win.dc, pixel_format_id, &pixel_format);

	// now we are in a position to create the modern opengl context
	int context_attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB,
							 gl_major,
							 WGL_CONTEXT_MINOR_VERSION_ARB,
							 gl_minor,
							 WGL_CONTEXT_PROFILE_MASK_ARB,
							 WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
							 0 };

	win.context = wglCreateContextAttribsARB(win.dc, 0, context_attribs);
	assert(win.context);

	result = wglMakeCurrent(win.dc, win.context);
	assert(result);

	result = wglDeleteContext(fake_ctx);
	assert(result);

	result = ReleaseDC(fake_wnd, fake_dc);
	assert(result);

	result = DestroyWindow(fake_wnd);
	assert(result);

	return win;
}

int
main(int argc, char** argv)
{
	//create offline window with attached 4.5 opengl context
	win_gl win = offline_win_create(4, 5);

	//All the following in the state of the current opengl context (win.context)
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	//fb
	GLuint offline_fb, offline_tex;
	glGenFramebuffers(1, &offline_fb);

	//tex
	int view_size[2]{ 512,512 };
	glGenTextures(1, &offline_tex);
	glBindTexture(GL_TEXTURE_2D, offline_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, view_size[0], view_size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, NULL);

	//render offline to fb
	glBindFramebuffer(GL_FRAMEBUFFER, offline_fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offline_tex, 0);
	{
		//for some reason the right read is after the second draw..double buffering? later
		unsigned char* data = new unsigned char[4 * view_size[0] * view_size[1]];
		for(int draws = 0; draws < 2; ++draws)
		{
			//draw
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(1, 0, 0, 1);
			glViewport(0, 0, view_size[0], view_size[1]);

			//read
			glReadPixels(0, 0, view_size[0], view_size[1], GL_RGBA, GL_UNSIGNED_BYTE, data);
		};
		delete data;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	return 0;
}