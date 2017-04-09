/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "bimg_p.h"

#include <libsquish/squish.h>
#include <etc1/etc1.h>
#include <etc2/ProcessRGB.hpp>
#include <nvtt/nvtt.h>
#include <pvrtc/PvrTcEncoder.h>
#include <edtaa3/edtaa3func.h>

namespace bimg
{
	bool imageEncodeFromRgba8(void* _dst, const void* _src, uint32_t _width, uint32_t _height, TextureFormat::Enum _format)
	{
		switch (_format)
		{
		case TextureFormat::BC1:
		case TextureFormat::BC2:
		case TextureFormat::BC3:
		case TextureFormat::BC4:
		case TextureFormat::BC5:
			squish::CompressImage( (const uint8_t*)_src, _width, _height, _dst
				, _format == TextureFormat::BC2 ? squish::kDxt3
				: _format == TextureFormat::BC3 ? squish::kDxt5
				: _format == TextureFormat::BC4 ? squish::kBc4
				: _format == TextureFormat::BC5 ? squish::kBc5
				:                                 squish::kDxt1
				);
			return true;

		case TextureFormat::BC6H:
			nvtt::compressBC6H( (const uint8_t*)_src, _width, _height, 4, _dst);
			return true;

		case TextureFormat::BC7:
			nvtt::compressBC7( (const uint8_t*)_src, _width, _height, 4, _dst);
			return true;

		case TextureFormat::ETC1:
			etc1_encode_image( (const uint8_t*)_src, _width, _height, 4, _width*4, (uint8_t*)_dst);
			return true;

		case TextureFormat::ETC2:
			{
				const uint32_t blockWidth  = (_width +3)/4;
				const uint32_t blockHeight = (_height+3)/4;
				const uint32_t pitch = _width*4;
				const uint8_t* src = (const uint8_t*)_src;
				uint64_t* dst = (uint64_t*)_dst;
				for (uint32_t yy = 0; yy < blockHeight; ++yy)
				{
					for (uint32_t xx = 0; xx < blockWidth; ++xx)
					{
						uint8_t block[4*4*4];
						const uint8_t* ptr = &src[(yy*pitch+xx*4)*4];

						for (uint32_t ii = 0; ii < 16; ++ii)
						{ // BGRx
							bx::memCopy(&block[ii*4], &ptr[(ii%4)*pitch + (ii&~3)], 4);
							bx::xchg(block[ii*4+0], block[ii*4+2]);
						}

						*dst++ = ProcessRGB_ETC2(block);
					}
				}
			}
			return true;

		case TextureFormat::PTC14:
			{
				using namespace Javelin;
				RgbaBitmap bmp;
				bmp.width  = _width;
				bmp.height = _height;
				bmp.data   = (uint8_t*)const_cast<void*>(_src);
				PvrTcEncoder::EncodeRgb4Bpp(_dst, bmp);
				bmp.data = NULL;
			}
			return true;

		case TextureFormat::PTC14A:
			{
				using namespace Javelin;
				RgbaBitmap bmp;
				bmp.width  = _width;
				bmp.height = _height;
				bmp.data   = (uint8_t*)const_cast<void*>(_src);
				PvrTcEncoder::EncodeRgba4Bpp(_dst, bmp);
				bmp.data = NULL;
			}
			return true;

		case TextureFormat::BGRA8:
			imageSwizzleBgra8(_dst, _width, _height, _width*4, _src);
			return true;

		case TextureFormat::RGBA8:
			bx::memCopy(_dst, _src, _width*_height*4);
			return true;

		default:
			break;
		}

		return imageConvert(_dst, _format, _src, TextureFormat::RGBA8, _width, _height);
	}

	bool imageEncodeFromRgba32f(bx::AllocatorI* _allocator, void* _dst, const void* _src, uint32_t _width, uint32_t _height, TextureFormat::Enum _format)
	{
		const uint8_t* src = (const uint8_t*)_src;

		switch (_format)
		{
		case TextureFormat::RGBA8:
			{
				uint8_t* dst = (uint8_t*)_dst;
				for (uint32_t yy = 0; yy < _height; ++yy)
				{
					for (uint32_t xx = 0; xx < _width; ++xx)
					{
						const uint32_t offset = yy*_width + xx;
						const float* input = (const float*)&src[offset * 16];
						uint8_t* output    = &dst[offset * 4];
						output[0] = uint8_t(input[0]*255.0f + 0.5f);
						output[1] = uint8_t(input[1]*255.0f + 0.5f);
						output[2] = uint8_t(input[2]*255.0f + 0.5f);
						output[3] = uint8_t(input[3]*255.0f + 0.5f);
					}
				}
			}
			return true;

		case TextureFormat::BC5:
			{
				uint8_t* temp = (uint8_t*)BX_ALLOC(_allocator, _width*_height*4);
				for (uint32_t yy = 0; yy < _height; ++yy)
				{
					for (uint32_t xx = 0; xx < _width; ++xx)
					{
						const uint32_t offset = yy*_width + xx;
						const float* input = (const float*)&src[offset * 16];
						uint8_t* output    = &temp[offset * 4];
						output[0] = uint8_t(input[0]*255.0f + 0.5f);
						output[1] = uint8_t(input[1]*255.0f + 0.5f);
						output[2] = uint8_t(input[2]*255.0f + 0.5f);
						output[3] = uint8_t(input[3]*255.0f + 0.5f);
					}
				}

				imageEncodeFromRgba8(_dst, temp, _width, _height, _format);
				BX_FREE(_allocator, temp);
			}
			return true;

		default:
			break;
		}

		return imageConvert(_dst, _format, _src, TextureFormat::RGBA32F, _width, _height);
	}

	void imageRgba32f11to01(void* _dst, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t yy = 0; yy < _height; ++yy)
		{
			for (uint32_t xx = 0; xx < _width; ++xx)
			{
				const uint32_t offset = yy*_pitch + xx * 16;
				const float* input = (const float*)&src[offset];
				float* output = (float*)&dst[offset];
				output[0] = input[0]*0.5f + 0.5f;
				output[1] = input[1]*0.5f + 0.5f;
				output[2] = input[2]*0.5f + 0.5f;
				output[3] = input[3]*0.5f + 0.5f;
			}
		}
	}

	static void edtaa3(bx::AllocatorI* _allocator, double* _dst, uint32_t _width, uint32_t _height, double* _src)
	{
		const uint32_t numPixels = _width*_height;

		short* xdist = (short *)BX_ALLOC(_allocator, numPixels*sizeof(short) );
		short* ydist = (short *)BX_ALLOC(_allocator, numPixels*sizeof(short) );
		double* gx   = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );
		double* gy   = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );

		::computegradient(_src, _width, _height, gx, gy);
		::edtaa3(_src, gx, gy, _width, _height, xdist, ydist, _dst);

		for (uint32_t ii = 0; ii < numPixels; ++ii)
		{
			if (_dst[ii] < 0.0)
			{
				_dst[ii] = 0.0;
			}
		}

		BX_FREE(_allocator, xdist);
		BX_FREE(_allocator, ydist);
		BX_FREE(_allocator, gx);
		BX_FREE(_allocator, gy);
	}

	inline double min(double _a, double _b)
	{
		return _a > _b ? _b : _a;
	}

	inline double max(double _a, double _b)
	{
		return _a > _b ? _a : _b;
	}

	inline double clamp(double _val, double _min, double _max)
	{
		return max(min(_val, _max), _min);
	}

	void imageMakeDist(bx::AllocatorI* _allocator, void* _dst, uint32_t _width, uint32_t _height, uint32_t _pitch, float _edge, const void* _src)
	{
		const uint32_t numPixels = _width*_height;

		double* imgIn   = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );
		double* outside = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );
		double* inside  = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );

		for (uint32_t yy = 0; yy < _height; ++yy)
		{
			const uint8_t* src = (const uint8_t*)_src + yy*_pitch;
			double* dst = &imgIn[yy*_width];
			for (uint32_t xx = 0; xx < _width; ++xx)
			{
				dst[xx] = double(src[xx])/255.0;
			}
		}

		edtaa3(_allocator, outside, _width, _height, imgIn);

		for (uint32_t ii = 0; ii < numPixels; ++ii)
		{
			imgIn[ii] = 1.0 - imgIn[ii];
		}

		edtaa3(_allocator, inside, _width, _height, imgIn);

		BX_FREE(_allocator, imgIn);

		uint8_t* dst = (uint8_t*)_dst;

		double edgeOffset = _edge*0.5;
		double invEdge = 1.0/_edge;

		for (uint32_t ii = 0; ii < numPixels; ++ii)
		{
			double dist = clamp( ( (outside[ii] - inside[ii])+edgeOffset) * invEdge, 0.0, 1.0);
			dst[ii] = 255-uint8_t(dist * 255.0);
		}

		BX_FREE(_allocator, inside);
		BX_FREE(_allocator, outside);
	}

} // namespace bimg
