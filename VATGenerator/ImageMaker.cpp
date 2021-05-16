#include "ImageMaker.h"

ImageMaker::ImageMaker(AnimationData& animationData, const char* path, bool& success)
{
	success = false;
	frameData = animationData.getFrameData();
	frames = animationData.getFrameCount();
	verts = animationData.getVertexCount();

	double min = 30000;
	double max = -30000;

	for (int i = 0; i < frames; i++)
	{
		for (int j = 0; j < verts; j++)
		{
			VertexData data = frameData[i][j];
			if (data.x < min)
			{
				min = data.x;
			}
			if (data.y < min)
			{
				min = data.y;
			}
			if (data.z < min)
			{
				min = data.z;
			}

			if (data.x > max)
			{
				max = data.x;
			}
			if (data.y > max)
			{
				max = data.y;
			}
			if (data.z > max)
			{
				max = data.z;
			}
		}
	}
	std::cout << "Min: " << min << " | Max: " << max << std::endl;
	if (!createFile(path))
		return;

	if (!initPngData())
	{
		closeFile();
		return;
	}

	closeFile();
	success = true;
}

bool ImageMaker::createFile(const char* path)
{
	std::string str = std::string(path);
	str = str.substr(0, str.find('.'));
	str.append(".png");

	if (fopen_s(&filePointer, str.c_str(), "wb") != 0)
		return false;


	return true;
}

void ImageMaker::closeFile()
{
	fclose(filePointer);
}

bool ImageMaker::initPngData()
{
	png_voidp user_error_ptr = 0;
	png_error_ptr user_error_fn = 0, user_warning_fn = 0;

	png_structp png_ptr = png_create_write_struct
	(
		PNG_LIBPNG_VER_STRING, 
		user_error_ptr,
		user_error_fn, 
		user_warning_fn
	);
	if (!png_ptr)
		return false;

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	png_init_io(png_ptr, filePointer);
	png_set_write_status_fn(png_ptr, write_row_callback);
	png_set_IHDR(png_ptr, info_ptr, verts, frames, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_text text;
	text.compression = PNG_TEXT_COMPRESSION_NONE;
	text.key = png_charp("Title");
	text.text = png_charp("My title");
	png_set_text(png_ptr, info_ptr, &text, 1);
	
	png_write_info(png_ptr, info_ptr);
	
	png_bytep row = NULL;
	row = (png_bytep)malloc(sizeof(png_byte) * verts * 3);

	for (int y = 0; y < frames; y++)
	{
		for (int x = 0; x < verts; x++)
		{
			row[x * 3] = frameData[y][x].x; //R
			row[x * 3 + 1] = frameData[y][x].y; //G
			row[x * 3 + 2] = frameData[y][x].z; //B
		}
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);
	return true;
}

void ImageMaker::write_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
	//nothing needs to be here lol
}