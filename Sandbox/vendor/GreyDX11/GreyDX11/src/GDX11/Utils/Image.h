#pragma once
#include <string>

namespace GDX11::Utils
{
	struct ImageData
	{
		uint8_t* pixels;
		int width;
		int height;
		int nrComponents;
	};

	ImageData LoadImageFile(const std::string& filename, bool flipImageY, int reqComponents);
	void FreeImageData(ImageData* data);

	void WriteImagePNG(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data, int strideInBytes);
	void WriteImageBMP(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data);
	void WriteImageTGA(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data);
	void WriteImageJPG(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const void* data, int quality);
	void WriteImageHDR(const std::string& filename, bool flipImageY, int width, int height, int nrComponents, const float* data);
}