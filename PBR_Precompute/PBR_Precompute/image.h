#pragma once

namespace io
{
	struct Image
	{
		int width, height, channels;
		void* data;
	};

	enum class IMAGE_FORMAT
	{
		BMP,
		PNG,
		JPG,
		HDR
	};

	Image
		image_read(const char* path, IMAGE_FORMAT format);

	void
		image_write(const Image& img, const char* path, IMAGE_FORMAT format);

	void
		image_free(Image& img);
};