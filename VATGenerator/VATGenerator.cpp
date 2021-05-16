#include <iostream>
#include <string>
#include <conio.h>
#include "ModelReader.h"
#include "ImageMaker.h"

const char* supportedFiletypes[] = { ".fbx" };
const int supportedFiletypesCount = 1;

const char* stripFileExtension(char* path)
{
	for (int i = strlen(path); i >= 0; i--)
	{
		if (path[i] == '.')
		{
			return path + i;
		}
	}

	return "";
}

void printSupportedFormats()
{
	std::cout << "Supported formats are: \n";
	for (int i = 0; i < supportedFiletypesCount; i++)
	{
		std::cout << supportedFiletypes[i] << std::endl;
	}
}

bool checkValidFiletype(const char* fileExt, FileType& type)
{
	for (int i = 0; i < supportedFiletypesCount; i++)
	{
		if (strcmp(fileExt, supportedFiletypes[i]) == 0)
		{
			type = (FileType)i;
			return true;
		}
	}


	return false;
}

void exitMessage()
{
	std::cout << "Press any key to exit.\n";
	_getch();
}

void createData(FileType type, const char* path)
{
	bool success;
	ModelReader model = ModelReader(type, path, success);

	if (!success)
	{
		std::cout << "Error reading model data" << std::endl;
		return;
	}

	ImageMaker imgMaker = ImageMaker(model.getAnimationData(), path, success);

	if (!success)
	{
		std::cout << "Error writing file" << std::endl;
		return;
	}

	if (success)
	{
		std::cout << "Completed successfully" << std::endl;
	}

}

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	createData(FileType::FBX, "P:/VertexAnimationTextureGenerator/VAT_Exporter/Debug/pontus_crowd_30fps.fbx");
#else

	if (argc < 2)
	{
		std::cout << "Please drop a valid model with animation onto VATGenerator.exe\n";
		printSupportedFormats();
	}
	else
	{
		//Get File Extension
		const char* fileExt = stripFileExtension(argv[1]);
		FileType type;
		//Check if supported format
		if (checkValidFiletype(fileExt, type))
		{
			createData(type, argv[1]);
		}
		else
		{
			std::cout << "Invalid file type:" << fileExt << std::endl;
			printSupportedFormats();
		}
	}

#endif
	exitMessage();
}