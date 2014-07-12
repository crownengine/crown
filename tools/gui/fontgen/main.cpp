#include <cmath>
#include <ctime>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <freetype.h>
#include <stdint.h>
#include <stb_image.c>
#include <string>

#include "BinPacker.hpp"

using namespace std;

static int font_size = 4;
static char font_name[100];

//-----------------------------------------------------------------------------
struct sdf_glyph
{
	int ID;
	int width, height;
	int x, y;
	float xoff, yoff;
	float xadv;
};

//-----------------------------------------------------------------------------
bool render_signed_distance_font(FT_Library &ft_lib, const char* font_file, int texture_size, bool export_c_header);
unsigned char get_SDF_radial(unsigned char *fontmap, int w, int h, int x, int y, int max_radius);
bool gen_pack_list(FT_Face &ft_face, int pixel_size, int pack_tex_size, const std::vector<int> &render_list, std::vector<sdf_glyph> &packed_glyphs);
int save_png_SDFont(const char* orig_filename, int img_width, int img_height, const std::vector<unsigned char> &img_data, const std::vector<sdf_glyph> &packed_glyphs);

//	number of rendered pixels per SDF pixel (larger value means higher quality, up to a point)
const int scaler = 16;

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	printf( "Crown Font Generator\n\n" );
	if(argc != 4)
	{
		printf( "Usage: ./fontgen <src> <name> <texture_dim>" );
		return -1;
	}

	const char* src = argv[1];
	const char* name = argv[2];
	strncpy(font_name, name, strlen(name));
	int texture_size = atoi(argv[3]);

	bool export_c_header = false;

	// Checks texture size
	if (texture_size < 64) texture_size = 64;
	if (texture_size > 4096) texture_size = 4096;

	FT_Library ft_lib;
	int ft_err = FT_Init_FreeType( &ft_lib );
	if( ft_err )
	{
		printf( "Failed to initialize the FreeType library!\n" );
		return -1;
	}

	render_signed_distance_font(ft_lib, src, texture_size, export_c_header);

	ft_err = FT_Done_FreeType( ft_lib );

    return 0;
}

//-----------------------------------------------------------------------------
bool render_signed_distance_font(FT_Library &ft_lib, const char* font_file, int texture_size, bool export_c_header)
{
	FT_Face ft_face;
	int ft_err = FT_New_Face(ft_lib, font_file, 0, &ft_face);
	if(ft_err)
	{
		printf( "Failed to read the font file '%s'\n", font_file );
		return false;
	}
	else
	{
		printf("Font:%s\n", font_file);

		int max_unicode_char= 65535;
		std::vector< int > render_list;
		for (int char_idx = 0; char_idx <= max_unicode_char; ++char_idx)
		{
			render_list.push_back(char_idx);
		}
		//	find the perfect size

		std::vector< sdf_glyph > all_glyphs;
		//	initial guess for the size of the Signed Distance Field font
		//	(intentionally low, the first trial will be at font_size*2, so 8x8)

		bool keep_going = true;
		while (keep_going)
		{
			font_size <<= 1;
			keep_going = gen_pack_list(ft_face, font_size, texture_size, render_list, all_glyphs);
		}

		int font_size_step = font_size >> 2;
		while (font_size_step)
		{
			if (keep_going)
			{
				font_size += font_size_step;
			}
			else
			{
				font_size -= font_size_step;
			}
			font_size_step >>= 1;
			keep_going = gen_pack_list(ft_face, font_size, texture_size, render_list, all_glyphs);
		}

		//	just in case
		while((!keep_going) && (font_size > 1))
		{
			--font_size;
			keep_going = gen_pack_list(ft_face, font_size, texture_size, render_list, all_glyphs);
		}

		printf("Font size = %i pixels\n\n", font_size);

		if (!keep_going)
		{
			printf( "The data will not fit in a texture %i^2\n", texture_size);
			return -1;
		}

		//	set up the RAM for the final rendering/compositing
		//	(use all four channels, so PNG compression is simple)
		std::vector<unsigned char> pdata(4 * texture_size * texture_size, 0);

		//	render all the glyphs individually
		printf("Rendering into image... ");

		int packed_glyph_index = 0;

		int tin = clock();

		for (unsigned int char_index = 0; char_index < render_list.size(); ++char_index)
		{
			int glyph_index = FT_Get_Char_Index(ft_face, render_list[char_index]);

			if (glyph_index)
			{
				ft_err = FT_Load_Glyph(ft_face, glyph_index, 0);
				if (!ft_err)
				{
					ft_err = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_MONO);
					if (!ft_err)
					{
						//	we have the glyph, already rendered, get the data about it
						int w = ft_face->glyph->bitmap.width;
						int h = ft_face->glyph->bitmap.rows;
						int p = ft_face->glyph->bitmap.pitch;

						//	oversize the holding buffer so I can smooth it!
						int sw = w + scaler * 4;
						int sh = h + scaler * 4;
						unsigned char *smooth_buf = new unsigned char[sw*sh];
						for (int i = 0; i < sw*sh; ++i)
						{
							smooth_buf[i] = 0;
						}

						//	copy the glyph into the buffer to be smoothed
						unsigned char *buf = ft_face->glyph->bitmap.buffer;
						for (int j = 0; j < h; ++j)
						{
							for (int i = 0; i < w; ++i)
							{
								smooth_buf[scaler*2+i+(j+scaler*2)*sw] = 255 * ((buf[j*p+(i>>3)] >> (7 - (i & 7))) & 1);
							}
						}
						//	do the SDF
						int sdfw = all_glyphs[packed_glyph_index].width;
						int sdfx = all_glyphs[packed_glyph_index].x;
						int sdfh = all_glyphs[packed_glyph_index].height;
						int sdfy = all_glyphs[packed_glyph_index].y;
						for (int j = 0; j < sdfh; ++j)
						{
							for (int i = 0; i < sdfw; ++i)
							{
								int pd_idx = (i+sdfx+(j+sdfy)*texture_size) * 4;
								pdata[pd_idx] = get_SDF_radial(smooth_buf, sw, sh, i*scaler + (scaler >>1), j*scaler + (scaler >>1), 2*scaler);
								pdata[pd_idx+1] = pdata[pd_idx];
								pdata[pd_idx+2] = pdata[pd_idx];
								pdata[pd_idx+3] = pdata[pd_idx];
							}
						}
						++packed_glyph_index;

						delete [] smooth_buf;
					}
					//printf( "%i ", render_list[char_index] );
				}
			}
		}

		tin = clock() - tin;
		printf("Done. %.3f seconds.\n", ((float)tin) / CLOCKS_PER_SEC);

		printf("Exporting... ");
		tin = save_png_SDFont(font_file, texture_size, texture_size, pdata, all_glyphs);
		printf("Done. %.3f seconds.\n", ((float)tin) / CLOCKS_PER_SEC);

		//	clean up my data
		all_glyphs.clear();
		pdata.clear();

		ft_err = FT_Done_Face( ft_face );
	}

	return true;
}

//-----------------------------------------------------------------------------
int save_png_SDFont(const char* orig_filename, int img_width, int img_height, const std::vector<unsigned char> &img_data, const std::vector< sdf_glyph > &packed_glyphs)
{
	//	save my image
	string dest(orig_filename);
	unsigned offs = dest.find(".ttf");
	dest.replace(offs, 4, "");
	dest += ".tga";

	int tin = clock();
	FILE* file = fopen(dest.c_str(), "wb");
	if ( NULL != file )
	{
		uint8_t type = 2;
		uint8_t bpp = 32;
		uint8_t xorig = 0;
		uint8_t yorig = 0;

		putc(0, file);
		putc(0, file);
		putc(type, file);
		putc(0, file); 
		putc(0, file);
		putc(0, file); 
		putc(0, file);
		putc(0, file);
		putc(0, file); 
		putc(xorig, file);
		putc(0, file); 
		putc(yorig, file);
		putc(img_width&0xff, file);
		putc( (img_width>>8)&0xff, file);
		putc(img_height&0xff, file);
		putc( (img_height>>8)&0xff, file);
		putc(bpp, file);
		putc(32, file);

		uint32_t width = img_width * bpp / 8;
		uint8_t* data = (uint8_t*)img_data.data();
		for (uint32_t yy = 0; yy < img_height; ++yy)
		{
			fwrite(data, width, 1, file);
			data += width;
		}

		fclose(file);
	}
	tin = clock() - tin;

	//	now save the acompanying info
	offs = dest.find(".tga");
	dest.replace(offs, 4, "");
	dest += ".font";
	FILE *fp = fopen(dest.c_str(), "w");

	// EDITED by CROWN DEVELOPERS -- json output
	if( fp )
	{
		fprintf(fp, "{\n");
		fprintf(fp, "\"material\": \"%s\",\n", font_name);
		fprintf(fp, "\"count\":%i,\n", packed_glyphs.size());
		fprintf(fp, "\"size\":%i,\n", img_width);
		fprintf(fp, "\"font_size\": %i, \n", font_size);
		fprintf(fp, "\"glyphs\" : [\n");
		for(unsigned int i = 0; i < packed_glyphs.size()-1; ++i)
		{
			fprintf( fp, "\t{\"id\":%i, \"x\":%i, \"y\":%i, \"width\":%i, \"height\":%i,",
				packed_glyphs[i].ID,
				packed_glyphs[i].x,
				img_height - packed_glyphs[i].y,
				packed_glyphs[i].width,
				packed_glyphs[i].height
				);
			fprintf( fp, "\"x_offset\":%.2f, \"y_offset\":%.2f, \"x_advance\":%.2f},\n",
				packed_glyphs[i].xoff,
				packed_glyphs[i].yoff,
				packed_glyphs[i].xadv
				);
		}

		fprintf( fp, "\t{\"id\":%i, \"x\":%i, \"y\":%i, \"width\":%i, \"height\":%i,",
			packed_glyphs[packed_glyphs.size()-1].ID,
			packed_glyphs[packed_glyphs.size()-1].x,
			img_height - packed_glyphs[packed_glyphs.size()-1].y,
			packed_glyphs[packed_glyphs.size()-1].width,
			packed_glyphs[packed_glyphs.size()-1].height
			);
		fprintf( fp, "\"x_offset\":%.2f, \"y_offset\":%.2f, \"x_advance\":%.2f}\n",
			packed_glyphs[packed_glyphs.size()-1].xoff,
			packed_glyphs[packed_glyphs.size()-1].yoff,
			packed_glyphs[packed_glyphs.size()-1].xadv
			);
		fprintf(fp, "\t]\n");
		fprintf(fp, "}");
		fclose( fp );
	}

	return tin;
}

//-----------------------------------------------------------------------------
bool gen_pack_list(
		FT_Face &ft_face,
		int pixel_size,
		int pack_tex_size,
		const std::vector< int > &render_list,
		std::vector< sdf_glyph > &packed_glyphs )
{
	int ft_err;
	packed_glyphs.clear();
	ft_err = FT_Set_Pixel_Sizes( ft_face, pixel_size*scaler, 0 );
	std::vector< int > rectangle_info;
	std::vector< std::vector<int> > packed_glyph_info;
	for( unsigned int char_index = 0; char_index < render_list.size(); ++char_index )
	{
		int glyph_index = FT_Get_Char_Index( ft_face, render_list[char_index] );
		if( glyph_index )
		{
			ft_err = FT_Load_Glyph( ft_face, glyph_index, 0 );
			if( !ft_err )
			{

				ft_err = FT_Render_Glyph( ft_face->glyph, FT_RENDER_MODE_MONO );
				if( !ft_err )
				{
					sdf_glyph add_me;
					//	we have the glyph, already rendered, get the data about it
					int w = ft_face->glyph->bitmap.width;
					int h = ft_face->glyph->bitmap.rows;
					//	oversize the holding buffer so I can smooth it!
					int sw = w + scaler * 4;
					int sh = h + scaler * 4;
					//	do the SDF
					int sdfw = sw / scaler;
					int sdfh = sh / scaler;
					rectangle_info.push_back( sdfw );
					rectangle_info.push_back( sdfh );
					//	add in the data I already know
					add_me.ID = render_list[char_index];
					add_me.width = sdfw;
					add_me.height = sdfh;
					//	these need to be filled in later (after packing)
					add_me.x = -1;
					add_me.y = -1;
					//	these need scaling...
					add_me.xoff = ft_face->glyph->bitmap_left;
					add_me.yoff = ft_face->glyph->bitmap_top;
					add_me.xadv = ft_face->glyph->advance.x / 64.0;
					//	so scale them (the 1.5's have to do with the padding
					//	border and the sampling locations for the SDF)
					add_me.xoff = add_me.xoff / scaler - 1.5;
					add_me.yoff = add_me.yoff / scaler + 1.5;
					add_me.xadv = add_me.xadv / scaler;
					//	add it to my list
					packed_glyphs.push_back( add_me );
				}
			}
		}
	}
	const bool dont_allow_rotation = false;
	BinPacker bp;
	bp.Pack( rectangle_info, packed_glyph_info, pack_tex_size, dont_allow_rotation );
	//	populate the actual coordinates
	if( packed_glyph_info.size() == 1 )
	{
		//	it all fit into one!
		unsigned int lim = packed_glyph_info[0].size();
		for( unsigned int i = 0; i < lim; i += 4 )
		{
			//	index, x, y, rotated
			unsigned int idx = packed_glyph_info[0][i+0];
			packed_glyphs[idx].x = packed_glyph_info[0][i+1];
			packed_glyphs[idx].y = packed_glyph_info[0][i+2];
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
unsigned char get_SDF_radial(
		unsigned char *fontmap,
		int w, int h,
		int x, int y,
		int max_radius )
{
	//	hideous brute force method
	float d2 = max_radius*max_radius+1.0;
	unsigned char v = fontmap[x+y*w];
	for( int radius = 1; (radius <= max_radius) && (radius*radius < d2); ++radius )
	{
		int line, lo, hi;
		//	north
		line = y - radius;
		if( (line >= 0) && (line < h) )
		{
			lo = x - radius;
			hi = x + radius;
			if( lo < 0 ) { lo = 0; }
			if( hi >= w ) { hi = w-1; }
			int idx = line * w + lo;
			for( int i = lo; i <= hi; ++i )
			{
				//	check this pixel
				if( fontmap[idx] != v )
				{
					float nx = i - x;
					float ny = line - y;
					float nd2 = nx*nx+ny*ny;
					if( nd2 < d2 )
					{
						d2 = nd2;
					}
				}
				//	move on
				++idx;
			}
		}
		//	south
		line = y + radius;
		if( (line >= 0) && (line < h) )
		{
			lo = x - radius;
			hi = x + radius;
			if( lo < 0 ) { lo = 0; }
			if( hi >= w ) { hi = w-1; }
			int idx = line * w + lo;
			for( int i = lo; i <= hi; ++i )
			{
				//	check this pixel
				if( fontmap[idx] != v )
				{
					float nx = i - x;
					float ny = line - y;
					float nd2 = nx*nx+ny*ny;
					if( nd2 < d2 )
					{
						d2 = nd2;
					}
				}
				//	move on
				++idx;
			}
		}
		//	west
		line = x - radius;
		if( (line >= 0) && (line < w) )
		{
			lo = y - radius + 1;
			hi = y + radius - 1;
			if( lo < 0 ) { lo = 0; }
			if( hi >= h ) { hi = h-1; }
			int idx = lo * w + line;
			for( int i = lo; i <= hi; ++i )
			{
				//	check this pixel
				if( fontmap[idx] != v )
				{
					float nx = line - x;
					float ny = i - y;
					float nd2 = nx*nx+ny*ny;
					if( nd2 < d2 )
					{
						d2 = nd2;
					}
				}
				//	move on
				idx += w;
			}
		}
		//	east
		line = x + radius;
		if( (line >= 0) && (line < w) )
		{
			lo = y - radius + 1;
			hi = y + radius - 1;
			if( lo < 0 ) { lo = 0; }
			if( hi >= h ) { hi = h-1; }
			int idx = lo * w + line;
			for( int i = lo; i <= hi; ++i )
			{
				//	check this pixel
				if( fontmap[idx] != v )
				{
					float nx = line - x;
					float ny = i - y;
					float nd2 = nx*nx+ny*ny;
					if( nd2 < d2 )
					{
						d2 = nd2;
					}
				}
				//	move on
				idx += w;
			}
		}
	}
	d2 = sqrtf( d2 );
	if( v==0 )
	{
		d2 = -d2;
	}
	d2 *= 127.5 / max_radius;
	d2 += 127.5;
	if( d2 < 0.0 ) d2 = 0.0;
	if( d2 > 255.0 ) d2 = 255.0;
	return (unsigned char)(d2 + 0.5);
}
