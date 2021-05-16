#pragma once
#include <vector>
#include "VertexData.h"
#include <string>
#include <png.h>

class ImageMaker
{
private:
	int frames;
	int verts;
	std::vector<VertexData>* frameData;

	bool createFile(const char* path);
	void closeFile();
	bool initPngData();
	FILE* filePointer;

	static void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass);
public:

	ImageMaker(AnimationData& animationData, const char* path, bool& success);
};

