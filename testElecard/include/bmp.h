#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <thread>

#include "structs.h"
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
	
	bool readBmp(const std::string&);
	YUV444 RGBtoYUV444(RGB);
	RGB YUVtoRGB(YUV444);
	void ConvertRGBtoYUV444();
	void ConvertRGBtoYUV420();
	void flipAndRotateYUV420();
	bool writeYUV444ToFile();
	bool writeYUV420ToFile();
	SoA_YUV420 getImage();
#ifdef simd
	bool soaReadBmp(const std::string&);
	void simdConvertRGBtoYUV444();
	void simdThreadsRGBtoYUV444();
	void simdConvertRGBtoYUV420();
	void soaConvertRGBtoYUV444();
	void soaConvertRGBtoYUV444(int pos, int size);
#endif //simd

private:
	SoA_YUV420 _yuv420_colors;
	std::vector<YUV444> _yuv444_colors;
	std::vector<RGB> _rgb_colors;
	BITMAPFILEHEADER _file_header;
	BITMAPINFOHEADER _info_header;
	int _yuv420_height;
	int _yuv420_width;
#ifdef simd
	SoA_RGB _soa_rgb_colors;
	SoA_YUV444 _soa_yuv_colors;
#endif //simd

};