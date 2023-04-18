#pragma once
#include "structs.h"
#include <vector>
#include <fstream>
#include <string>
class YUV420Video
{
private:
	int _frame_height;
	int _frame_width;
	std::vector<SoA_YUV420_uint8> _frame_buffer;
public:
	bool readVideo(const std::string& path,int height, int width, int frame_count);
	bool addImage(const SoA_YUV420&,int,int);
	bool writeVideo();
};