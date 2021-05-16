#pragma once
#include "VertexData.h"
#include <iostream>
#include <fbxsdk.h>
#include <vector>
#include <stack>
#include <unordered_set>

enum class FileType : int
{
	FBX = 0,
};

class ModelReader
{
private:
	
#pragma region FBX Import
	bool readFBX(const char* path);
	bool fbxGetInitialVerts(FbxMesh* mesh);
	FbxAMatrix getPoseMatrix(FbxPose* pose, int nodeIndex);
	FbxAMatrix getGlobalPosition(FbxNode* node, const FbxTime& time, FbxPose* pose, FbxAMatrix* parentGlobalPos = NULL);
	FbxAMatrix getGeometry(FbxNode* node);
	void computeClusterDeformation(FbxAMatrix& globalPosition, FbxMesh* mesh, FbxCluster* cluster, FbxAMatrix& vertexTransformMatrix, FbxTime time, FbxPose* pose);
	void computeLinearDeformation(FbxAMatrix& globalPosition, FbxMesh* mesh, FbxTime& time, FbxVector4* vertexArray, FbxPose* pose);
	void matrixScale(FbxAMatrix& matrix, double value);
	void matrixAdd(FbxAMatrix& dstMatrix, FbxAMatrix& srcMatrix);
	std::vector<FbxNode*> getAllBones(FbxMesh* mesh);

#pragma endregion

	AnimationData animData;
public:
	ModelReader(FileType type, const char* path, bool& success);
	~ModelReader();
	AnimationData& getAnimationData();
};

