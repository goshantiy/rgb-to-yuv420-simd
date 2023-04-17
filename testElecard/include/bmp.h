#pragma once
#include <fstream>
#include <string>
#include <vector>
#define simd
#define outputs

#ifdef outputs
#include <iostream>
#endif //outputs

#ifdef simd
#include <immintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#endif //simd

class RGBConvert
{
public:
	struct RGB
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};
	struct YUV
	{
		float y;
		float u;
		float v;
	};

	bool readBmp(std::string);
	YUV RGBtoYUV444(RGB);
	RGB YUVtoRGB(YUV);
	void ConvertRGBtoYUV444();
#ifdef simd
	bool simdReadBmp(std::string);
	void simdConvertRGBtoYUV444();
#endif //simd
private:
	struct	BITMAPFILEHEADER {
		uint16_t bfType;
		uint32_t bfSize;
		uint16_t bfReserved1;
		uint16_t bfReserved2;
		uint32_t bfOffBits;
	};
	struct BITMAPINFOHEADER {
		uint32_t biSize;
		uint32_t biWidth;
		uint32_t biHeight;
		uint16_t biPlanes;
		uint16_t biBitCount;
		uint32_t biCompression;
		uint32_t biSizeImage;
		uint32_t biXPelsPerMeter;
		uint32_t biYPelsPerMeter;
		uint32_t biClrUsed;
		uint32_t biClrImportant;
	};
	std::vector<YUV> _yuv_colors;
	std::vector<RGB> _rgb_colors;
	BITMAPFILEHEADER _file_header;
	BITMAPINFOHEADER _info_header;

#ifdef simd
	struct SoA_RGB
	{
		std::vector<uint8_t> b;
		std::vector<uint8_t> g;
		std::vector<uint8_t> r;
	};
	SoA_RGB _soa_rgb_colors;
	struct SoA_YUV {
		std::vector<float> y;
		std::vector<float> u;
		std::vector<float> v;
	};
	SoA_YUV _soa_yuv_colors;
#endif //simd

};