#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

#include "GDX11/Core/GDX11Assert.h"

namespace GDX11::Utils
{
	ImageData LoadImageFile(const std::string& filename, bool flipImageY, int reqComponents)
	{
		ImageData data = {};
		stbi_set_flip_vertically_on_load(flipImageY);
		data.pixels = stbi_load(filename.c_str(), &data.width, &data.height, &data.nrComponents, reqComponents);

		GDX11_CORE_ASSERT(data.pixels, "Failed to load image file: {0}", filename);

		return data;
	}

	void FreeImageData(ImageData* data)
	{
		stbi_image_free(data->pixels);
		data->pixels = nullptr;
	}

	void WriteImagePNG(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data, int strideInBytes)
	{
		stbi_flip_vertically_on_write(flipImageY);
		int res = stbi_write_png(filename.c_str(), width, height, nrComponents, data, strideInBytes);

		GDX11_CORE_ASSERT(res != 0, "Failed to write image");
	}

	void WriteImageBMP(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data)
	{
		stbi_flip_vertically_on_write(flipImageY);
		int res = stbi_write_bmp(filename.c_str(), width, height, nrComponents, data);

		GDX11_CORE_ASSERT(res != 0, "Failed to write image");
	}

	void WriteImageTGA(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data)
	{
		stbi_flip_vertically_on_write(flipImageY);
		int res = stbi_write_tga(filename.c_str(), width, height, nrComponents, data);

		GDX11_CORE_ASSERT(res != 0, "Failed to write image");
	}

	void WriteImageJPG(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data, int quality)
	{
		stbi_flip_vertically_on_write(flipImageY);
		int res = stbi_write_jpg(filename.c_str(), width, height, nrComponents, data, quality);

		GDX11_CORE_ASSERT(res != 0, "Failed to write image");
	}

	void WriteImageHDR(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const float* data)
	{
		stbi_flip_vertically_on_write(flipImageY);
		int res = stbi_write_hdr(filename.c_str(), width, height, nrComponents, data);

		GDX11_CORE_ASSERT(res != 0, "Failed to write image");
	}
}