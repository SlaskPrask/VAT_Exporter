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

	if (initPngData())
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
	png_set_IHDR(png_ptr, info_ptr, verts, frames, 16, 2, 0, 0, 0);
	//png_set_rows(png_ptr, info_ptr,)
	//png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
}

void ImageMaker::write_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
	//nothing needs to be here lol
}