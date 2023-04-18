#include <iostream>
#include "bmp.h"
#include <immintrin.h>
#include <chrono>
int main()
{
	RGBConvert test;
	std::string name("C:/Users/PC/source/repos/testElecard/testElecard/sources/RAKETA.bmp");
	test.readBmp(name);
	auto start1 = std::chrono::high_resolution_clock::now();
	test.ConvertRGBtoYUV420();
	auto end1 = std::chrono::high_resolution_clock::now();

	auto ms_int1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
	std::cout << "time spent: " << ms_int1.count() << "ms\n";
	test.soaReadBmp(name);
	auto start = std::chrono::high_resolution_clock::now();
	test.simdConvertRGBtoYUV420();
	auto end = std::chrono::high_resolution_clock::now();

	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "time spent: "<<ms_int.count()<<"ms";
	return 0;
}
