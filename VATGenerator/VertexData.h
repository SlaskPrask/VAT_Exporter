#pragma once
#include <vector>
#include <queue>
#include <iostream>
class VertexData
{
private:

public:

	double x;
	double y;
	double z;
	VertexData(double x, double y, double z);

	VertexData& operator-=(const VertexData& rhs);
};

class AnimationData
{
private:
	std::vector<VertexData> initialVertices;
	std::vector<VertexData>* frameData;
	int frameCount;
public:
	
	AnimationData();
	~AnimationData();
	void addFrame(std::vector<VertexData> vertData, int frame);
	void setInitialVertices(std::vector<VertexData> initialVerts);
	void setFrames(int frames);
	int getFrameCount();
	std::vector<VertexData>* getFrameData();
	int getVertexCount();
	void calculateDeltas();
};
