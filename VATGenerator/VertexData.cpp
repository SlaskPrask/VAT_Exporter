#include "VertexData.h"

VertexData::VertexData(double x, double y, double z) : x(x), y(y), z(z)
{

}

VertexData& VertexData::operator-=(const VertexData& rhs)
{
	(*this).x -= rhs.x;
	(*this).y -= rhs.x;
	(*this).z -= rhs.x;
	return *this;
}

AnimationData::AnimationData()
{
	frameCount = 0;
	frameData = NULL;
}

AnimationData::~AnimationData()
{
	if (frameData != NULL)
		delete[] frameData;
}

void AnimationData::addFrame(std::vector<VertexData> vertData, int frame)
{
	frameData[frame] = vertData;
}

void AnimationData::setInitialVertices(std::vector<VertexData> initialVerts)
{
	initialVertices = initialVerts;
}

void AnimationData::setFrames(int frames)
{
	frameCount = frames;
	frameData = new std::vector<VertexData>[frameCount];
}

int AnimationData::getFrameCount()
{
	return frameCount;
}

std::vector<VertexData>* AnimationData::getFrameData()
{
	return frameData;
}

int AnimationData::getVertexCount()
{
	return initialVertices.size();
}

void AnimationData::calculateDeltas()
{
	for (int i = 0; i < frameCount; i++)
	{
		for (unsigned int j = 0; j < initialVertices.size(); j++)
		{
			frameData[i][j] -= initialVertices[j];
		}
	}
}
