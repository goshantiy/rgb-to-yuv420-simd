#include <yuv420video.h>

bool YUV420Video::readVideo(const std::string& path,  int width, int height, int frame_count)
{
	bool result = false;
	std::ifstream file(path, std::ios::binary);
	if (file.is_open())
	{
		_frame_height = height;
		_frame_width = width;
		_frame_buffer.resize(frame_count);
		int ySize = width * height;
		int uvSize = ySize / 4;

		for (int i = 0; i < frame_count; i++) {
			_frame_buffer[i].y.resize(ySize);
			_frame_buffer[i].u.resize(uvSize);
			_frame_buffer[i].v.resize(uvSize);
			file.read(reinterpret_cast<char*>(_frame_buffer[i].y.data()), ySize);
			file.read(reinterpret_cast<char*>(_frame_buffer[i].u.data()), uvSize);
			file.read(reinterpret_cast<char*>(_frame_buffer[i].v.data()), uvSize);
		}
		result = true;
	}
	return result;
}

bool YUV420Video::addImage(const SoA_YUV420& img, int width, int height)
{
	bool result = false;
	if (height < _frame_height && width < _frame_width)
	{
		for (int frame = 0; frame < _frame_buffer.size(); frame++)
		{
			for (int i = 0; i < height; i++) 
			{
				for (int j = 0; j < width; j++)
				{
					_frame_buffer[frame].y[i * _frame_width + j] = img.y[i * width + j];
				}
			}
			for (int i = 0; i < height / 2; i++)
			{
				for (int j = 0; j < width / 2; j++)
				{
					_frame_buffer[frame].u[i * _frame_width /2 + j] = img.u[i * width/2 + j];
					_frame_buffer[frame].v[i * _frame_width /2 + j] = img.v[i * width/2 + j];
				}
			}
		}
		result = true;
	}
	return result;
}
bool YUV420Video::writeVideo()
{
	std::ofstream file("output_video.yuv", std::ios::out | std::ios::binary);
	bool result = false;
	if (file.is_open())
	{
		for (auto it : _frame_buffer)
		{
			for (int i = 0; i < (_frame_height* _frame_width); i++)
				file << it.y[i];
			for (int i = 0; i < (_frame_height * _frame_width / 4); i++)
			{
				file << it.u[i];
			}
			for (int i = 0; i < (_frame_height * _frame_width / 4); i++)
			{
				file << it.v[i];
			}
		}
		result = true;
	}
	return result;
}