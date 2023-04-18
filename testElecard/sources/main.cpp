#include <iostream>
#include "bmp.h"
#include "yuv420video.h"
int main()
{
	RGBConvert image;
	std::string name("C:/Users/PC/source/repos/testElecard/testElecard/files/256.bmp");
	if(image.soaReadBmp(name))
	image.simdConvertRGBtoYUV420();
	image.writeYUV420ToFile();

	SoA_YUV420 img = image.getImage();
	YUV420Video video;
	std::string videopath("C:/Users/PC/source/repos/testElecard/testElecard/files/bus_cif.yuv");
	video.readVideo(videopath,  352, 288, 150);
	video.addImage(img, 256, 256);
	video.writeVideo();
	return 0;
}
