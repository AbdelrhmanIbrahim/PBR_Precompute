#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Stb_Image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Stb_Image_Write.h"

namespace io
{
	Image
		image_read(const char* path, IMAGE_FORMAT format)
	{
		Image self{};

		switch (format)
		{
		case IMAGE_FORMAT::BMP:
		case IMAGE_FORMAT::PNG:
		case IMAGE_FORMAT::JPG:
			self.data = stbi_load(path, &self.width, &self.height, &self.channels, 0);
			break;
		case IMAGE_FORMAT::HDR:
			stbi_set_flip_vertically_on_load(true);
			self.data = stbi_loadf(path, &self.width, &self.height, &self.channels, 0);
			break;
		default:
			break;
		}
		return self;
	}

	void
	image_write(const Image& img, const char* path, IMAGE_FORMAT format)
	{
		switch (format)
		{
		case IMAGE_FORMAT::BMP:
			stbi_write_bmp(path, img.width, img.height, img.channels, img.data);
			break;
		case IMAGE_FORMAT::PNG:
			stbi_write_png(path, img.width, img.height, img.channels, img.data, img.width * img.channels);
			break;
		case IMAGE_FORMAT::JPG:
			stbi_write_jpg(path, img.width, img.height, 4, img.data, 100);
			break;
		case IMAGE_FORMAT::HDR:
			stbi_write_hdr(path, img.width, img.height, 4, (float*)img.data);
			break;
		default:
			assert("unsupported image format" && false);
			break;
		}
	}

	void
		image_free(Image& img)
	{
		stbi_image_free(img.data);
	}
};