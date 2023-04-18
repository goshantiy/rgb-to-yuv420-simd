#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#define simd
//#define outputs

#ifdef outputs
#include <iostream>
#endif //outputs

#ifdef simd
#include <immintrin.h>
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
	struct YUV444
	{
		float y;
		float u;
		float v;
	};
	struct SoA_YUV420
	{
		std::vector<float> y;
		std::vector<float> u;
		std::vector<float> v;
	};
	bool readBmp(std::string);
	YUV444 RGBtoYUV444(RGB);
	RGB YUVtoRGB(YUV444);
	void ConvertRGBtoYUV444();
	void ConvertRGBtoYUV420();
	bool writeYUV420ToFile();
	bool writeYUV444ToFile();

#ifdef simd
	bool soaReadBmp(std::string);
	void simdConvertRGBtoYUV444();
	void simdThreadsRGBtoYUV444();
	void simdConvertRGBtoYUV420();
	void soaConvertRGBtoYUV444(int pos, int size);
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

	SoA_YUV420 _yuv420_colors;
	std::vector<YUV444> _yuv444_colors;
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
	struct SoA_YUV444 {
		std::vector<uint8_t> y;
		std::vector<uint8_t> u;
		std::vector<uint8_t> v;
	};
	SoA_YUV444 _soa_yuv_colors;
#endif //simd

};