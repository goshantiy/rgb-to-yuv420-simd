#include <iostream>
#include "bmp.h"
#include <immintrin.h>

int main()
{
	RGBConvert test;
	std::string name("C:/Users/PC/source/repos/testElecard/testElecard/sources/123.bmp");
	test.readBmp(name);
	test.ConvertRGBtoYUV444();
	test.simdReadBmp(name);
	test.simdConvertRGBtoYUV444();
	return 0;
}
