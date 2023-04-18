#include "bmp.h"

bool RGBConvert::readBmp(const std::string& path)
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
YUV444 RGBConvert::RGBtoYUV444(RGB rgb)
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
			_yuv444_colors.push_back(buf);
#ifdef outputs
			std::cout << (int)buf.y << " " << (int)buf.u << " " << (int)buf.v << "\n";
#endif
		}
}
void RGBConvert::ConvertRGBtoYUV420()
{
	int size = _rgb_colors.size();
	if (size % 4)
		size -= size % 4;
	_yuv420_colors.y.resize(size);
	_yuv420_colors.u.resize(size / 4);
	_yuv420_colors.v.resize(size / 4);
	int height = _info_header.biHeight - _info_header.biHeight%4;
	int width = _info_header.biWidth - _info_header.biWidth%4;
	_yuv420_height = height;
	_yuv420_width = width;
	for (int y = 0; y < height; ++y)
		for (int x = 0;x < width; ++x) {
			_yuv420_colors.y[y * width + x] = 0.2126 * _rgb_colors[y * width + x].r +
				0.7152 * _rgb_colors[y * width + x].g + 0.0722 * _rgb_colors[y * width + x].b;
		}

	for (int y = 0; y < height; y += 2) {
		for (int x = 0; x < width; x += 2) {
			int index = y * width + x;
			const RGB& rgb0 = _rgb_colors[index];
			const RGB& rgb1 = _rgb_colors[index + 1];
			const RGB& rgb2 = _rgb_colors[index + width];
			const RGB& rgb3 = _rgb_colors[index + width + 1];
			_yuv420_colors.u[(y * width) / 4 + x / 2] =
				((-0.1146 * rgb0.r - 0.3854 * rgb0.g + 0.5 * rgb0.b) +
					(-0.1146 * rgb1.r - 0.3854 * rgb1.g + 0.5 * rgb1.b) +
					(-0.1146 * rgb2.r - 0.3854 * rgb2.g + 0.5 * rgb2.b) +
					(-0.1146 * rgb3.r - 0.3854 * rgb3.g + 0.5 * rgb3.b)) / 4 + 128;
			_yuv420_colors.v[(y * width) / 4 + x / 2] =
				((0.5 * rgb0.r - 0.4542 * rgb0.g - 0.0458 * rgb0.b) +
					(0.5 * rgb1.r - 0.4542 * rgb1.g - 0.0458 * rgb1.b) +
					(0.5 * rgb2.r - 0.4542 * rgb2.g - 0.0458 * rgb2.b) +
					(0.5 * rgb3.r - 0.4542 * rgb3.g - 0.0458 * rgb3.b)) / 4 + 128;
		}
	}
}
bool RGBConvert::writeYUV444ToFile()
{
	std::ofstream file("output.yuv", std::ios::out | std::ios::binary);
	bool result = false;

	if (file.is_open()) {
		for (auto it : _soa_yuv_colors.y) {
			file << (uint8_t)std::round(it);
		}
		for (auto it : _soa_yuv_colors.u) {
			file << (uint8_t)std::round(it);
		}
		for (auto it : _soa_yuv_colors.v) {
			file << (uint8_t)std::round(it);
		}
		file.close();
		result = true;
	}
	return result;
}
void RGBConvert::flipAndRotateYUV420()
{
	std::vector<float> rotatedY(_yuv420_colors.y.size());
	std::vector<float> rotatedU(_yuv420_colors.u.size());
	std::vector<float> rotatedV(_yuv420_colors.v.size());
	//flip and rotate
	for (int x = 0; x < _yuv420_width; x++) {
		for (int y = 0; y < _yuv420_height; y++) {
			uint8_t pixel = _yuv420_colors.y[y * _yuv420_width + x];
			int newX = _yuv420_height - y - 1;
			int newY = x;
			rotatedY[newX * _yuv420_width + newY] = pixel;
		}
	}
	//flip and rotate
	for (int x = 0; x < _yuv420_width/2; x++) {
		for (int y = 0; y < _yuv420_height / 2; y++) {
			uint8_t uPixel = _yuv420_colors.u[y * _yuv420_width / 2 + x];
			uint8_t vPixel = _yuv420_colors.v[y * _yuv420_width / 2 + x];
			int newX = (_yuv420_height / 2) - y - 1;
			int newY = x;
			rotatedU[newX * _yuv420_width / 2 + newY] = uPixel;
			rotatedV[newX * _yuv420_width / 2 + newY] = vPixel;
		}
	}
	_yuv420_colors.y = rotatedY;
	_yuv420_colors.u = rotatedU;
	_yuv420_colors.v = rotatedV;
}
bool RGBConvert::writeYUV420ToFile() {
	std::ofstream file("output.yuv", std::ios::out | std::ios::binary);
	bool result = false;
	flipAndRotateYUV420();

	if (file.is_open()) {
		for (auto it : _yuv420_colors.y) {
			file << (uint8_t)std::round(it);
		}
		for (auto it : _yuv420_colors.u) {
			file << (uint8_t)std::round(it);
		}
		for (auto it : _yuv420_colors.v) {
			file << (uint8_t)std::round(it);
		}
		file.close();
		result = true;
	}
	return result;
}
SoA_YUV420 RGBConvert::getImage()
{
	return _yuv420_colors;
}
#ifdef simd
void RGBConvert::soaConvertRGBtoYUV444(int pos, int size)
{
	for (int i = pos; i < size; ++i)
	{
		_soa_yuv_colors.y[i] = 0.2126 * _soa_rgb_colors.r[i] + 0.7152 * _soa_rgb_colors.g[i]
			+ 0.0722 * _soa_rgb_colors.b[i];
		_soa_yuv_colors.u[i] = (-0.1146 * _soa_rgb_colors.r[i] - 0.3854 * _soa_rgb_colors.g[i]
			+ 0.5 * _soa_rgb_colors.b[i]) + 128;
		_soa_yuv_colors.v[i] = (0.5 * _soa_rgb_colors.r[i] - 0.4542 * _soa_rgb_colors.g[i]
			- 0.0458 * _soa_rgb_colors.b[i]) + 128;
	}
}
bool RGBConvert::soaReadBmp(const std::string& path)
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
		soaConvertRGBtoYUV444(0,size);
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

void RGBConvert::simdConvertRGBtoYUV420()
{
	int size = _soa_rgb_colors.r.size();
	if (size % 8)
		size -= size % 8;
	_yuv420_colors.y.resize(size);
	_yuv420_colors.u.resize(size / 4);
	_yuv420_colors.v.resize(size / 4);
	int height = _info_header.biHeight - _info_header.biHeight % 8;
	int width = _info_header.biWidth - _info_header.biWidth % 8;
	_yuv420_width = width;
	_yuv420_height = height;
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

	for (size_t i = 0; i < size && size >= 8; i += 8)
	{
		const __m256 r = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
			_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.r[i]))));
		const __m256 g = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
			_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.g[i]))));
		const __m256 b = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
			_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.b[i]))));

		//0.2126 * rgb.r + 0.7152 * rgb.g + 0.0722 * rgb.b;

		const __m256 y = _mm256_fmadd_ps(factor_y, r,
			_mm256_fmadd_ps(factor_y2, g, _mm256_mul_ps(factor_y3, b)));

		_mm256_storeu_ps(&_yuv420_colors.y[i], y);
	}

	for (int y = 0; y < height; y += 2) {
		for (int x = 0; x < width; x += 8) {
			int index = y * width + x;
			const __m256 r1 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
				_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.r[index]))));
			const __m256 g1 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
				_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.g[index]))));
			const __m256 b1 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
				_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.b[index]))));

			const __m256 r2 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
				_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.r[index + width]))));
			const __m256 g2 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
				_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.g[index + width]))));
			const __m256 b2 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(
				_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&_soa_rgb_colors.b[index + width]))));


			//u = (-0.1146 * rgb.r - 0.3854 * rgb.g + 0.5 * rgb.b) + 128;
			//v = (0.5 * rgb.r - 0.4542 * rgb.g - 0.0458 * rgb.b) + 128

			/*
			we take an 8x2 rectangle, add the upper row with the lower one,
				add the neighboring elementsand divide by 4, i.e.we find the average value for 4 2x2 squares,
				write them to m128and put them in memory for U and V
				*/
			const __m256 u1 = _mm256_add_ps(
				_mm256_add_ps(_mm256_mul_ps(factor_u, r1), _mm256_mul_ps(factor_u2, g1)),
				_mm256_add_ps(_mm256_mul_ps(factor_u3, b1), offset_128));
			const __m256 u2 = _mm256_add_ps(
				_mm256_add_ps(_mm256_mul_ps(factor_u, r2), _mm256_mul_ps(factor_u2, g2)),
				_mm256_add_ps(_mm256_mul_ps(factor_u3, b2), offset_128));

			__m256 div1 = _mm256_div_ps(_mm256_add_ps(u1, u2), _mm256_set1_ps(4.f));
			__m256 result1 = _mm256_hadd_ps(div1, div1);

			const __m256 v1 = _mm256_add_ps(
				_mm256_add_ps(_mm256_mul_ps(factor_v, r1), _mm256_mul_ps(factor_v2, g1)),
				_mm256_add_ps(_mm256_mul_ps(factor_v3, b1), offset_128));
			const __m256 v2 = _mm256_add_ps(
				_mm256_add_ps(_mm256_mul_ps(factor_v, r2), _mm256_mul_ps(factor_v2, g2)),
				_mm256_add_ps(_mm256_mul_ps(factor_v3, b2), offset_128));

			__m256 div2 = _mm256_div_ps(_mm256_add_ps(v1, v2), _mm256_set1_ps(4.f));
			__m256 result2 = _mm256_hadd_ps(div2, div2);



			__m256 mask = _mm256_set_epi32(0, -1, 0, -1, 0, -1, 0, -1);

			//extract from m256 even index values and store in m128
			__m256 even_result1 = _mm256_and_ps(result1, mask);
			__m128 low1 = _mm256_extractf128_ps(even_result1, 0);
			__m128 high1 = _mm256_extractf128_ps(even_result1, 1);
			__m128 permuted_low1 = _mm_permute_ps(low1, _MM_SHUFFLE(2, 0, 3, 1));
			__m128 permuted_high1 = _mm_permute_ps(high1, _MM_SHUFFLE(3, 1, 2, 0));
			__m128 sum1 = _mm_add_ps(permuted_low1, permuted_high1);

			__m256 even_result2 = _mm256_and_ps(result2, mask);
			__m128 low2 = _mm256_extractf128_ps(even_result2, 0);
			__m128 high2 = _mm256_extractf128_ps(even_result2, 1);
			__m128 permuted_low2 = _mm_permute_ps(low2, _MM_SHUFFLE(2, 0, 3, 1));
			__m128 permuted_high2 = _mm_permute_ps(high2, _MM_SHUFFLE(3, 1, 2, 0));
			__m128 sum2 = _mm_add_ps(permuted_low2, permuted_high2);
			int index_uv = (y * width) / 4 + x / 2;


			_mm_storeu_ps(&_yuv420_colors.u[index_uv], sum1);
			_mm_storeu_ps(&_yuv420_colors.v[index_uv], sum2);
		}
	}

}
#endif //simd
