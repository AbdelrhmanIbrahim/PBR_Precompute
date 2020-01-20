#pragma once

#include "Vector.h"
#include "Matrix.h"

#include "Image.h"

#define HANDLE(NAME) typedef struct NAME##__ { int unused; } *NAME;
#define to_radian(degree) degree * 0.01745329251f
#define to_degree(radian) radian * 57.2957795131f

#include <cstddef>

namespace geo
{
	struct Vertex
	{
		math::vec3f pos;
		math::vec3f normal;
		math::vec2f uv;
	};
};

namespace glgpu
{
	//make an internal handle instead of casting to an address ofc
	HANDLE(program);
	HANDLE(buffer);
	HANDLE(texture);
	HANDLE(cubemap);
	HANDLE(vao);
	HANDLE(framebuffer);

	enum TEXTURE_UNIT
	{
		UNIT_0,
		UNIT_1,
		UNIT_2
	};

	enum class DEPTH_TEST
	{
		LE,
		L,
		G
	};

	enum class FRAMEBUFFER_ATTACHMENT
	{
		COLOR0,
		COLOR1,
		DEPTH_STENCIL
	};

	enum class EXTERNAL_TEXTURE_FORMAT
	{
		RG,
		RGB,
		RGBA,
		DEPTH_STENCIL
	};
	
	enum class INTERNAL_TEXTURE_FORMAT
	{
		RG16F,
		RGB,
		RGBA,
		RGB16F,
		DEPTH_STENCIL
	};

	enum class DATA_TYPE
	{
		UBYTE,
		FLOAT,
		UINT_24_8
	};

	enum class TARGET
	{
		TEXTURE_2D,
		CUBEMAP,
	};

	struct Unifrom_Float
	{
		const char* uniform;
		float value;
	};

	void
	graphics_init();

	program
	program_create(const char* vertex_shader_path, const char* pixel_shader_path);

	void
	program_use(program prog);

	void
	program_delete(program prog);

	buffer
	vertex_buffer_create(const geo::Vertex vertices[], std::size_t count);

	buffer
	index_buffer_create(unsigned int indices[], std::size_t count);

	void
	buffer_delete(buffer buf);

	vao
	vao_create();

	void
	vao_bind(vao v, buffer vbo, buffer ebo);

	void
	vao_unbind();

	void
	vao_delete(vao va);

	texture
	texture2d_create(const io::Image& img, io::IMAGE_FORMAT format);

	texture
	texture2d_create(const char* image_path, io::IMAGE_FORMAT format);

	texture
	texture2d_create(math::vec2f size, INTERNAL_TEXTURE_FORMAT internal_format, EXTERNAL_TEXTURE_FORMAT format, DATA_TYPE type, bool mipmap);

	void
	texture2d_render_offline_to(texture output, program prog, math::vec2f view_size);

	void
	texture2d_bind(texture texture, TEXTURE_UNIT texture_unit);

	void
	texture2d_unbind();

	void
	texture2d_unpack(texture texture, io::Image& image, EXTERNAL_TEXTURE_FORMAT format, DATA_TYPE type);

	void
	texture_free(texture texture);

	cubemap
	cubemap_create(math::vec2f view_size, INTERNAL_TEXTURE_FORMAT texture_format, EXTERNAL_TEXTURE_FORMAT ext_format, DATA_TYPE type, bool mipmap);

	cubemap
	cubemap_rgba_create(const io::Image imgs[6]);

	cubemap
	cubemap_hdr_create(const io::Image& img, math::vec2f view_size, bool mipmap);

	void
	cubemap_bind(cubemap texture, TEXTURE_UNIT texture_unit);

	void
	cubemap_free(cubemap cmap);

	framebuffer
	framebuffer_create();

	void
	framebuffer_bind(framebuffer fb);

	void
	framebuffer_attach(framebuffer fb, texture tex, FRAMEBUFFER_ATTACHMENT attachment);

	void
	framebuffer_unbind();

	void
	framebuffer_free(framebuffer fb);

	void
	disable_color_buffer_rw();

	void
	depth_clear();

	void
	frame_start();

	void
	color_clear(float r, float g, float b);

	void
	depth_test(DEPTH_TEST test);

	void
	draw_strip(std::size_t vertices_count);

	void
	draw_indexed(unsigned int indcies_count);

	void
	uniform1f_set(program prog, const char* uniform, float data);

	void
	uniform3f_set(program prog, const char* uniform, const math::vec3f& data);

	void
	uniform4f_set(program prog, const char* uniform, const math::vec4f& data);

	void
	uniformmat4f_set(program prog, const char* uniform, const math::Mat4f& data);

	void
	uniform1i_set(program prog, const char* uniform, int data);

	void
	view_port(int x, int y, int width, int height);

	bool
	error();
}