#pragma once
#include <cstdint>
#include <vector>
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
struct SoA_YUV420_uint8
{
	std::vector<uint8_t> y;
	std::vector<uint8_t> u;
	std::vector<uint8_t> v;
};
struct SoA_YUV420
{
	std::vector<float> y;
	std::vector<float> u;
	std::vector<float> v;
};
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
struct SoA_RGB
{
	std::vector<uint8_t> b;
	std::vector<uint8_t> g;
	std::vector<uint8_t> r;
};
struct SoA_YUV444 {
	std::vector<float> y;
	std::vector<float> u;
	std::vector<float> v;
};