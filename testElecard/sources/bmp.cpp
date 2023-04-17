#include "bmp.h"

bool RGBConvert::readBmp(std::string path)
{
	bool result = false;
	std::ifstream file(path, std::ios::binary);
	if (file.is_open())
	{
		file.read(reinterpret_cast<char*>(&_file_header), 14);//or #pragma pack
		file.read(reinterpret_cast<char*>(&_info_header), 40);
		const int padding = (4 - (_info_header.biWidth * 3) % 4) % 4;//calculate padding for align to 4 bytes
		for (int i = 0; i < _info_header.biHeight; ++i)//read colors 
		{
			RGB buf;
			for (int j = 0; j < _info_header.biWidth; ++j)
			{
				file.read(reinterpret_cast<char*>(&buf), sizeof(RGB));
				#ifdef outputs
				std::cout <<(int)buf.b<<" "<<(int)buf.g << " " << (int)buf.r<<"\n";
				#endif
				_rgb_colors.push_back(buf);
			}
			file.seekg(padding, std::ios_base::cur);//skip padding
		}
		result = true;
	}
	return result;
}
RGBConvert::YUV444 RGBConvert::RGBtoYUV444(RGB rgb)
{
	YUV444 result;
	result.y = 0.2126 * rgb.r + 0.7152 * rgb.g + 0.0722 * rgb.b;
	result.u = (-0.1146 * rgb.r - 0.3854 * rgb.g + 0.5 * rgb.b) + 128;
	result.v = (0.5 * rgb.r - 0.4542 * rgb.g - 0.0458 * rgb.b) + 128;
	return result;
}
void RGBConvert::ConvertRGBtoYUV444()
{
	if(_rgb_colors.size())
		for (auto it : _rgb_colors)
		{
			YUV444 buf = RGBtoYUV444(it);
			_yuv_colors.push_back(buf);
#ifdef outputs
			std::cout << (int)buf.y << " " << (int)buf.u << " " << (int)buf.v << "\n";
#endif
		}
}
#ifdef simd
void RGBConvert::soaConvertRGBtoYUV444()
{
	size_t size = _soa_rgb_colors.r.size();
	for (int i = 0; i < size; i++)
	{
		_soa_yuv_colors.y[i] = 0.2126 * _soa_rgb_colors.r[i] + 0.7152 * _soa_rgb_colors.g[i]
			+ 0.0722 * _soa_rgb_colors.b[i];
		_soa_yuv_colors.u[i] = (-0.1146 * _soa_rgb_colors.r[i] - 0.3854 * _soa_rgb_colors.g[i]
			+ 0.5 * _soa_rgb_colors.b[i]) + 128;
		_soa_yuv_colors.v[i] = (0.5 * _soa_rgb_colors.r[i] - 0.4542 * _soa_rgb_colors.g[i]
			- 0.0458 * _soa_rgb_colors.b[i]) + 128;
	}
}
void RGBConvert::soaConvertRGBtoYUV444(int pos, int size)
{
	for (int i = pos; i < size; i++)
	{
		_soa_yuv_colors.y[i] = 0.2126 * _soa_rgb_colors.r[i] + 0.7152 * _soa_rgb_colors.g[i]
			+ 0.0722 * _soa_rgb_colors.b[i];
		_soa_yuv_colors.u[i] = (-0.1146 * _soa_rgb_colors.r[i] - 0.3854 * _soa_rgb_colors.g[i]
			+ 0.5 * _soa_rgb_colors.b[i]) + 128;
		_soa_yuv_colors.v[i] = (0.5 * _soa_rgb_colors.r[i] - 0.4542 * _soa_rgb_colors.g[i]
			- 0.0458 * _soa_rgb_colors.b[i]) + 128;
	}
}
bool RGBConvert::soaReadBmp(std::string path)
{
	bool result = false;
	std::ifstream file(path, std::ios::binary);
	if (file.is_open())
	{
		file.read(reinterpret_cast<char*>(&_file_header), 14);//or #pragma pack
		file.read(reinterpret_cast<char*>(&_info_header), 40);
		const int padding = (4 - (_info_header.biWidth * 3) % 4) % 4;//calculate padding for align to 4 bytes
		for (int i = 0; i < _info_header.biHeight; ++i)//read colors 
		{
			uint8_t r,g,b;
			for (int j = 0; j < _info_header.biWidth; ++j)
			{
				file.read(reinterpret_cast<char*>(&b), 1);
				file.read(reinterpret_cast<char*>(&g), 1);
				file.read(reinterpret_cast<char*>(&r), 1);
				_soa_rgb_colors.b.push_back(b);
				_soa_rgb_colors.g.push_back(g);
				_soa_rgb_colors.r.push_back(r);
			}
			file.seekg(padding, std::ios_base::cur);//skip padding
		}
		result = true;
	}
	return result;
}
void RGBConvert::simdThreadsRGBtoYUV444()
{

	size_t size = _soa_rgb_colors.r.size();
	_soa_yuv_colors.y.resize(size);
	_soa_yuv_colors.u.resize(size);
	_soa_yuv_colors.v.resize(size);
	if (size < 8)//no reason for simd and threads
	{
		soaConvertRGBtoYUV444();
		return;
	}
	size_t num_threads = std::thread::hardware_concurrency()/2;
	size_t thread_chunk_num = size / 8;// nums of chunks of 8 elements
	if (thread_chunk_num < num_threads)// no reason for creating threads if chunks num is too small
	{
		simdConvertRGBtoYUV444();
		return;
	}
	size_t chunks_for_thread = thread_chunk_num / num_threads;
	//YUV constants
	const __m256 factor_y = _mm256_set1_ps(0.2126f);
	const __m256 factor_u = _mm256_set1_ps(-0.1146f);
	const __m256 factor_v = _mm256_set1_ps(0.5f);

	const __m256 factor_y2 = _mm256_set1_ps(0.7152f);
	const __m256 factor_u2 = _mm256_set1_ps(-0.3854f);
	const __m256 factor_v2 = _mm256_set1_ps(-0.4542f);

	const __m256 factor_y3 = _mm256_set1_ps(0.0722f);
	const __m256 factor_u3 = _mm256_set1_ps(0.5f);
	const __m256 factor_v3 = _mm256_set1_ps(-0.0458f);
	const __m256 offset_128 = _mm256_set1_ps(128.f);
	std::vector<std::thread> threads(num_threads);
	for (int i = 0; i < num_threads; i++)
	{
		threads[i] = std::thread([&] {
			for (int j = i * thread_chunk_num; j < (i* chunks_for_thread); j += 8) {
				const __m256 r = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
					_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.r[i]))));
				const __m256 g = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
					_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.g[i]))));
				const __m256 b = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
					_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.b[i]))));
				//0.2126 * rgb.r + 0.7152 * rgb.g + 0.0722 * rgb.b;
				const __m256 y = _mm256_fmadd_ps(factor_y, r,
					_mm256_fmadd_ps(factor_y2, g, _mm256_mul_ps(factor_y3, b)));
				//(-0.1146 * rgb.r - 0.3854 * rgb.g + 0.5 * rgb.b) + 128;
				const __m256 u = _mm256_add_ps(
					_mm256_add_ps(_mm256_mul_ps(factor_u, r), _mm256_mul_ps(factor_u2, g)),
					_mm256_add_ps(_mm256_mul_ps(factor_u3, b), offset_128));
				//(0.5 * rgb.r - 0.4542 * rgb.g - 0.0458 * rgb.b) + 128
				const __m256 v = _mm256_add_ps(
					_mm256_add_ps(_mm256_mul_ps(factor_v, r), _mm256_mul_ps(factor_v2, g)),
					_mm256_add_ps(_mm256_mul_ps(factor_v3, b), offset_128));
				_mm256_storeu_ps(&_soa_yuv_colors.y[i], y);
				_mm256_storeu_ps(&_soa_yuv_colors.u[i], u);
				_mm256_storeu_ps(&_soa_yuv_colors.v[i], v);
			}//for
			}//lambda
		);//thread	
	}
	int off_elements = size % 8;
	soaConvertRGBtoYUV444(size - off_elements, size);
	for (int i = 0; i < num_threads; i++)
	{
		threads[i].join();
	}
}
void RGBConvert::simdConvertRGBtoYUV444()
{
	//YUV constants
	const __m256 factor_y = _mm256_set1_ps(0.2126f);
	const __m256 factor_u = _mm256_set1_ps(-0.1146f);
	const __m256 factor_v = _mm256_set1_ps(0.5f);

	const __m256 factor_y2 = _mm256_set1_ps(0.7152f);
	const __m256 factor_u2 = _mm256_set1_ps(-0.3854f);
	const __m256 factor_v2 = _mm256_set1_ps(-0.4542f);

	const __m256 factor_y3 = _mm256_set1_ps(0.0722f);
	const __m256 factor_u3 = _mm256_set1_ps(0.5f);
	const __m256 factor_v3 = _mm256_set1_ps(-0.0458f);

	const __m256 offset_128 = _mm256_set1_ps(128.f);

	size_t size = _soa_rgb_colors.r.size();
	_soa_yuv_colors.y.resize(size);
	_soa_yuv_colors.u.resize(size);
	_soa_yuv_colors.v.resize(size);
	int off_elements = size % 8;
	size -= off_elements;
	for (size_t i = 0; i < size&& size >= 8; i += 8) {
		const __m256 r = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
			_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.r[i]))));
		const __m256 g = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
			_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.g[i]))));
		const __m256 b = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
			_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.b[i]))));
		//0.2126 * rgb.r + 0.7152 * rgb.g + 0.0722 * rgb.b;
		const __m256 y = _mm256_fmadd_ps(factor_y, r,
			_mm256_fmadd_ps(factor_y2, g, _mm256_mul_ps(factor_y3, b)));
		//(-0.1146 * rgb.r - 0.3854 * rgb.g + 0.5 * rgb.b) + 128;
		const __m256 u = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(factor_u, r), _mm256_mul_ps(factor_u2, g)),
			_mm256_add_ps(_mm256_mul_ps(factor_u3, b), offset_128));
		//(0.5 * rgb.r - 0.4542 * rgb.g - 0.0458 * rgb.b) + 128
		const __m256 v = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(factor_v, r), _mm256_mul_ps(factor_v2, g)),
			_mm256_add_ps(_mm256_mul_ps(factor_v3, b),offset_128));

		_mm256_storeu_ps(&_soa_yuv_colors.y[i], y);
		_mm256_storeu_ps(&_soa_yuv_colors.u[i], u);
		_mm256_storeu_ps(&_soa_yuv_colors.v[i], v);
	}
	soaConvertRGBtoYUV444(size - off_elements, size);
}
#endif //simd
