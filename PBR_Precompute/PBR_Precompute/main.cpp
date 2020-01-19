
#include <Windows.h>

#include "glew.h"
#include "wglew.h"

#include <assert.h>

#include "Gfx.h"
#include "glgpu.h"
#include "image.h"

#include <vector>

using namespace math;
using namespace glgpu;
using namespace io;
using namespace geo;

struct win_gl
{
	HWND handle;
	HDC dc;
	HGLRC context;
};

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

constexpr static Vertex unit_cube[36] =
{
	//back
	Vertex{-1.0f, -1.0f, -1.0f},
	Vertex{1.0f, 1.0f, -1.0f},
	Vertex{1.0f, -1.0f, -1.0f},
	Vertex{1.0f, 1.0f, -1.0f},
	Vertex{-1.0f, -1.0f, -1.0f},
	Vertex{-1.0f, 1.0f, -1.0},

	//front
	Vertex{-1.0f, -1.0f, 1.0},
	Vertex{1.0f, -1.0f, 1.0f},
	Vertex{1.0f, 1.0f, 1.0f,},
	Vertex{1.0f, 1.0f, 1.0f,},
	Vertex{-1.0f, 1.0f, 1.0f},
	Vertex{-1.0f, -1.0f, 1.0},

	//left
	Vertex{-1.0f, 1.0f, 1.0f},
	Vertex{-1.0f, 1.0f, -1.0f},
	Vertex{-1.0f, -1.0f, -1.0f},
	Vertex{-1.0f, -1.0f, -1.0f},
	Vertex{-1.0f, -1.0f, 1.0},
	Vertex{-1.0f, 1.0f, 1.0f},

	//right
	Vertex{1.0f, 1.0f, 1.0f,},
	Vertex{1.0f, -1.0f, -1.0},
	Vertex{1.0f, 1.0f, -1.0f},
	Vertex{1.0f, -1.0f, -1.0f},
	Vertex{1.0f, 1.0f, 1.0f,},
	Vertex{1.0f, -1.0f, 1.0f},

	//bottom
	Vertex{-1.0f, -1.0f, -1.0f},
	Vertex{1.0f, -1.0f, -1.0f},
	Vertex{1.0f, -1.0f, 1.0f},
	Vertex{1.0f, -1.0f, 1.0f},
	Vertex{-1.0f, -1.0f, 1.0f},
	Vertex{-1.0f, -1.0f, -1.0f},

	//top
	Vertex{-1.0f, 1.0f, -1.0f},
	Vertex{1.0f, 1.0f, 1.0f,},
	Vertex{1.0f, 1.0f, -1.0f},
	Vertex{1.0f, 1.0f, 1.0f,},
	Vertex{-1.0f, 1.0f, -1.0f},
	Vertex{-1.0f, 1.0f, 1.0f}
};

std::vector<Image>
hdr_to_cubemap(const io::Image& img, vec2f view_size, bool mipmap)
{
	//create hdr texture
	texture hdr = texture2d_create(img, IMAGE_FORMAT::HDR);

	//convert HDR equirectangular environment map to cubemap
	//create 6 views that will be rendered to the cubemap using equarectangular shader
	//don't use ortho projection as this will make z in NDC the same so your captures will look like duplicated
	//1.00000004321 is tan(45 degrees)
	Mat4f proj = proj_prespective_matrix(100, 0.1, 1, -1, 1, -1, 1.00000004321);
	Mat4f views[6] =
	{
		view_lookat_matrix(vec3f{-0.001f,  0.0f,  0.0f}, vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f,  0.0f}),
		view_lookat_matrix(vec3f{0.001f,  0.0f,  0.0f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f,  0.0f}),
		view_lookat_matrix(vec3f{0.0f, -0.001f,  0.0f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f,  0.0f,  1.0f}),
		view_lookat_matrix(vec3f{0.0f,  0.001f,  0.0f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f,  0.0f,  -1.0f}),
		view_lookat_matrix(vec3f{0.0f,  0.0f, -0.001f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f,  0.0f}),
		view_lookat_matrix(vec3f{0.0f,  0.0f,  0.001f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f,  0.0f})
	};

	//create env cubemap
	//(HDR should a 32 bit for each channel to cover a wide range of colors,
	//they make the exponent the alpha and each channel remains 8 so 16 bit for each -RGB-)
	cubemap cube_map = cubemap_create(view_size, INTERNAL_TEXTURE_FORMAT::RGB16F, EXTERNAL_TEXTURE_FORMAT::RGB, DATA_TYPE::FLOAT, mipmap);

	//float framebuffer to render to
	GLuint fbo, rbo;
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, view_size[0], view_size[1]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	//setup
	program prog = program_create("shaders/cube.vertex", "shaders/equarectangular_to_cubemap.pixel");
	program_use(prog);
	texture2d_bind(hdr, TEXTURE_UNIT::UNIT_0);

	//render offline to the output cubemap texs
	glViewport(0, 0, view_size[0], view_size[1]);
	vao cube_vao = vao_create();
	buffer cube_vs = vertex_buffer_create(unit_cube, 36);

	std::vector<io::Image> imgs(6);
	for (int i = 0; i < 6; ++i)
	{
		imgs[i].data = new unsigned char[4 * view_size[0] * view_size[1]];
		imgs[i].width = view_size[0];
		imgs[i].height = view_size[1];
		imgs[i].channels = 4;
	}

	for (unsigned int i = 0; i < 6; ++i)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, (GLuint)cube_map, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		uniformmat4f_set(prog, "vp", proj * views[i]);
		vao_bind(cube_vao, cube_vs, NULL);
		draw_strip(36);
		vao_unbind();
		glReadPixels(0, 0, view_size[0], view_size[1], GL_RGBA, GL_UNSIGNED_BYTE, imgs[i].data);
	}

	texture2d_unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);

	//free
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &fbo);
	vao_delete(cube_vao);
	buffer_delete(cube_vs);
	program_delete(prog);
	texture_free(hdr);

	return imgs;
}

void
hdr_faces_extract(const char* hdr_path, vec2f view_size)
{
	frame_start();
	color_clear(1, 0, 0);

	//draw
	Image img = image_read(hdr_path, io::IMAGE_FORMAT::HDR);
	auto imgs = hdr_to_cubemap(img, view_size, false);
	image_free(img);

	io::image_write(imgs[0], std::string("right.png").c_str(), io::IMAGE_FORMAT::PNG);
	io::image_write(imgs[1], std::string("left.png").c_str(), io::IMAGE_FORMAT::PNG);
	io::image_write(imgs[2], std::string("top.png").c_str(), io::IMAGE_FORMAT::PNG);
	io::image_write(imgs[3], std::string("bottom.png").c_str(), io::IMAGE_FORMAT::PNG);
	io::image_write(imgs[4], std::string("back.png").c_str(), io::IMAGE_FORMAT::PNG);
	io::image_write(imgs[5], std::string("front.png").c_str(), io::IMAGE_FORMAT::PNG);

	for (int i = 0; i < 6; ++i)
		image_free(imgs[i]);
}

std::vector<Image>
cubemap_pp(cubemap input, cubemap output, program postprocessor, Unifrom_Float uniform, vec2f view_size, int mipmap_level)
{
	//convert HDR equirectangular environment map to cubemap
	//create 6 views that will be rendered to the cubemap using equarectangular shader
	//1.00000004321 is tan(45 degrees)
	Mat4f proj = proj_prespective_matrix(100, 0.1, 1, -1, 1, -1, 1.00000004321);
	Mat4f views[6] =
	{
		view_lookat_matrix(vec3f{-0.001f,  0.0f,  0.0f}, vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, -1.0f,  0.0f}),
		view_lookat_matrix(vec3f{0.001f,  0.0f,  0.0f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, -1.0f,  0.0f}),
		view_lookat_matrix(vec3f{0.0f, -0.001f,  0.0f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f,  0.0f,  1.0f}),
		view_lookat_matrix(vec3f{0.0f,  0.001f,  0.0f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f,  0.0f,  -1.0f}),
		view_lookat_matrix(vec3f{0.0f,  0.0f, -0.001f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, -1.0f,  0.0f}),
		view_lookat_matrix(vec3f{0.0f,  0.0f,  0.001f},  vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, -1.0f,  0.0f})
	};

	GLuint fbo, rbo;
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, view_size[0], view_size[1]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	//convolute
	program_use(postprocessor);
	cubemap_bind(input, TEXTURE_UNIT::UNIT_0);
	uniform1i_set(postprocessor, "env_map", TEXTURE_UNIT::UNIT_0);

	//assign float uniforms (move to arrays)
	uniform1f_set(postprocessor, uniform.uniform, uniform.value);

	//render offline to the output cubemap texs
	glViewport(0, 0, view_size[0], view_size[1]);
	vao cube_vao = vao_create();
	buffer cube_vs = vertex_buffer_create(unit_cube, 36);

	std::vector<io::Image> imgs(6);
	for (int i = 0; i < 6; ++i)
	{
		imgs[i].data = new unsigned char[4 * view_size[0] * view_size[1]];
		imgs[i].width = view_size[0];
		imgs[i].height = view_size[1];
		imgs[i].channels = 4;
	}
	
	for (unsigned int i = 0; i < 6; ++i)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, (GLuint)output, mipmap_level);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		uniformmat4f_set(postprocessor, "vp", proj * views[i]);
		vao_bind(cube_vao, cube_vs, NULL);
		draw_strip(36);
		vao_unbind();
		glReadPixels(0, 0, view_size[0], view_size[1], GL_RGBA, GL_UNSIGNED_BYTE, imgs[i].data);
	}

	texture2d_unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);

	//free
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &fbo);
	vao_delete(cube_vao);
	buffer_delete(cube_vs);

	return imgs;
}

int
main(int argc, char** argv)
{
	//if (argc < 2)
		//printf("pass two paths, first is the diffuse map HDR image, second is the enviroment map HDR image");

	//offline stuff, extract faces from hdr
	const char* diffuse_hdr_path = "LA_diff.hdr";
	const char* env_hdr_path = "LA_spec.hdr";

	//create offline window with attached 4.5 opengl context
	win_gl win = offline_win_create(4, 5);

	//extract diffuse cubemap
	{
		//for some reason the right read is after the second draw..double buffering? (TODO), so that's a dummy first draw
		//color_clear(0, 1, 0);
		//hdr_faces_extract(diffuse_hdr_path, vec2f{ 512, 512 });
	}

	//extract 5 Lod reflections cubemaps
	{
		vec2f prefiltered_initial_size{ 512, 512};
		io::Image env = image_read(env_hdr_path, io::IMAGE_FORMAT::HDR);
		cubemap env_cmap = cubemap_hdr_create(env, vec2f{ 512, 512 }, true);
		cubemap specular_prefiltered_map = cubemap_create(prefiltered_initial_size, INTERNAL_TEXTURE_FORMAT::RGB16F, EXTERNAL_TEXTURE_FORMAT::RGB, DATA_TYPE::FLOAT, true);
		program prefiltering_prog = program_create("shaders/cube.vertex", "shaders/specular_prefiltering_convolution.pixel");
		unsigned int max_mipmaps = 5;
		for (unsigned int mip_level = 0; mip_level < max_mipmaps; ++mip_level)
		{
			float roughness = (float)mip_level / max_mipmaps;
			vec2f mipmap_size{ prefiltered_initial_size[0] * std::pow(0.5, mip_level) , prefiltered_initial_size[0] * std::pow(0.5, mip_level) };
			auto imgs = cubemap_pp(env_cmap, specular_prefiltered_map, prefiltering_prog, Unifrom_Float{ "roughness", roughness }, mipmap_size, mip_level);

			io::image_write(imgs[0], std::string("sright.png").c_str(), io::IMAGE_FORMAT::PNG);
			io::image_write(imgs[1], std::string("sleft.png").c_str(), io::IMAGE_FORMAT::PNG);
			io::image_write(imgs[2], std::string("stop.png").c_str(), io::IMAGE_FORMAT::PNG);
			io::image_write(imgs[3], std::string("sbottom.png").c_str(), io::IMAGE_FORMAT::PNG);
			io::image_write(imgs[4], std::string("sback.png").c_str(), io::IMAGE_FORMAT::PNG);
			io::image_write(imgs[5], std::string("sfront.png").c_str(), io::IMAGE_FORMAT::PNG);

			for (int i = 0; i < 6; ++i)
				image_free(imgs[i]);
		}
		program_delete(prefiltering_prog);
		cubemap_free(env_cmap);
		image_free(env);
	}
	return 0;
}