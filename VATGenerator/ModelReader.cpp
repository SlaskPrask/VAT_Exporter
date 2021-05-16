#include "ModelReader.h"

ModelReader::ModelReader(FileType type, const char* path, bool& isValid)
{
	isValid = false;
	switch (type)
	{
		case FileType::FBX:
			if (!readFBX(path))
				return;
			break;
		default:
			break;
	}
	animData.calculateDeltas();
	isValid = true;
}

AnimationData& ModelReader::getAnimationData()
{
	return animData;
}

bool ModelReader::readFBX(const char* path)
{
	fbxsdk::FbxManager* sdkManager = fbxsdk::FbxManager::Create();
	
	FbxImporter* fbxImporter = FbxImporter::Create(sdkManager, "");
	FbxIOSettings* ios = FbxIOSettings::Create(sdkManager, IOSROOT);
	
	ios->SetBoolProp(IMP_FBX_MODEL, true);
	ios->SetBoolProp(IMP_FBX_ANIMATION, true);
	FbxScene* fbxScene = FbxScene::Create(sdkManager, "");


	//if import is failed, just clean up and exit
	if (!fbxImporter->Initialize(path, -1, ios) || !fbxImporter->Import(fbxScene))
	{
		std::cout << "File " << path << " bad" << std::endl;	
	
		ios->Destroy();
		fbxImporter->Destroy();
		fbxScene->Destroy();
		sdkManager->Destroy();
		return false;
	}
	ios->Destroy();
	fbxImporter->Destroy();

	FbxAnimStack* animStack = fbxScene->GetCurrentAnimationStack();
	fbxScene->SetCurrentAnimationStack(NULL);
	if (animStack == NULL)
	{
		std::cout << "No animations found." << std::endl;
		fbxScene->Destroy();
		sdkManager->Destroy();
		return false;
	}
	
	FbxNode* root = fbxScene->GetRootNode();
	FbxMesh* mesh = NULL;
	FbxNode* model = NULL;

	for (int i = 0; i < root->GetChildCount(); i++)
	{
		model = root->GetChild(i);
		mesh = model->GetMesh();
		if (mesh != NULL)
		{
			if (!fbxGetInitialVerts(mesh))
			{
				std::cout << "No mesh found." << std::endl;
				fbxScene->Destroy();
				sdkManager->Destroy();
				return false;
			}
			break;
		}		
	}

	if (mesh == NULL)
	{
		std::cout << "No mesh found." << std::endl;
		fbxScene->Destroy();
		sdkManager->Destroy();
		return false;
	}

	//Retrieve amount of frame
	fbxScene->SetCurrentAnimationStack(animStack);
	FbxTakeInfo* info = fbxScene->GetTakeInfo(animStack->GetName());
	int frames = info->mLocalTimeSpan.GetDuration().GetFrameCount(FbxTime::eFrames30);
	FbxTime frameTime;
	frameTime.SetTime(0, 0, 0, 1, 0, FbxTime::eFrames30);
	FbxTime time = info->mLocalTimeSpan.GetStart();

	FbxAMatrix dummyPos;
	FbxPose* pose = fbxScene->GetPose(0);

	//GetBones
	std::vector<FbxNode*> bones = getAllBones(mesh);

	animData.setFrames(frames);
	for (int i = 0; i < frames; i++)
	{		
		FbxAMatrix globalPos = getGlobalPosition(model, time, pose, &dummyPos);

		int vertCount = mesh->GetControlPointsCount();
		FbxVector4* vertArray = new FbxVector4[vertCount];
		memcpy(vertArray, mesh->GetControlPoints(), vertCount * sizeof(FbxVector4));

		//Gotta calculate the transfrom of the bones?
		for (unsigned int i = 0; i < bones.size(); i++)
		{
			bones[i]->EvaluateGlobalTransform(time);
			bones[i]->EvaluateLocalTransform(time);
		}


		computeLinearDeformation(globalPos, mesh, time, vertArray, pose);
		std::vector<VertexData> verts;

		for (int i = 0; i < vertCount; i++)
		{
			FbxVector4 vert = vertArray[i];
			verts.push_back(VertexData(vert[0], vert[1], vert[2]));
		}
		animData.addFrame(verts, i);
		time += frameTime;
	}
	
	fbxScene->Destroy();
	sdkManager->Destroy();

	return true;
}

bool ModelReader::fbxGetInitialVerts(FbxMesh* mesh)
{
	std::vector<VertexData> verts;

	for (int i = 0; i < mesh->GetControlPointsCount(); i++)
	{
		FbxVector4 vec = mesh->GetControlPointAt(i);
		verts.push_back(VertexData(vec[0], vec[1], vec[2]));
	}

	animData.setInitialVertices(verts);
	return verts.size() > 0;
}

FbxAMatrix ModelReader::getPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
	FbxAMatrix lPoseMatrix;
	FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

	memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

	return lPoseMatrix;
}

FbxAMatrix ModelReader::getGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPos)
{
	FbxAMatrix globalPosition;
	bool posFound = false;

	if (pPose)
	{
		int nodeIndex = pPose->Find(pNode);
		if (nodeIndex > -1)
		{
			if (pPose->IsBindPose() || !pPose->IsLocalMatrix(nodeIndex))
			{
				globalPosition = getPoseMatrix(pPose, nodeIndex);
			}
			else
			{
				FbxAMatrix lParentGlobalPosition;

				if (pParentGlobalPos)
				{
					lParentGlobalPosition = *pParentGlobalPos;
				}
				else
				{
					if (pNode->GetParent())
					{
						lParentGlobalPosition = getGlobalPosition(pNode->GetParent(), pTime, pPose);
					}
				}

				FbxAMatrix lLocalPosition = getPoseMatrix(pPose, nodeIndex);
				globalPosition = lParentGlobalPosition * lLocalPosition;
			}
			posFound = true;
		}

		if (!posFound)
		{
			// There is no pose entry for that node, get the current global position instead.

			// Ideally this would use parent global position and local position to compute the global position.
			// Unfortunately the equation 
			//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
			// does not hold when inheritance type is other than "Parent" (RSrs).
			// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
			globalPosition = pNode->EvaluateGlobalTransform(pTime);
		}

		return globalPosition;
	}
}

FbxAMatrix ModelReader::getGeometry(FbxNode* node)
{
	FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);
	return FbxAMatrix(lT, lR, lS);
}

void ModelReader::computeClusterDeformation(FbxAMatrix& globalPosition, FbxMesh* mesh, FbxCluster* cluster, FbxAMatrix& vertexTransformMatrix, FbxTime time, FbxPose* pose)
{
	/*FbxAMatrix referenceGlobalInitPos;
	cluster->GetTransformMatrix(referenceGlobalInitPos);
	FbxAMatrix referenceGlobalCurrentPos = globalPosition;
	FbxAMatrix referenceGeometry = getGeometry(mesh->GetNode());
	referenceGlobalInitPos *= referenceGeometry;

	FbxAMatrix clusterGlobalInitPos;
	cluster->GetTransformLinkMatrix(clusterGlobalInitPos);
	FbxAMatrix clusterGlobalCurrentPos = getGlobalPosition(cluster->GetLink(), time, pose);

	FbxAMatrix clusterRelativeInitPos = clusterGlobalInitPos.Inverse() * referenceGlobalInitPos;
	FbxAMatrix clusterRelativeCurrentPosInverse = referenceGlobalCurrentPos.Inverse() * clusterGlobalCurrentPos;
	vertexTransformMatrix = clusterRelativeCurrentPosInverse * clusterRelativeInitPos;
	*/
	vertexTransformMatrix = cluster->GetLink()->EvaluateLocalTransform(time);
}

void ModelReader::computeLinearDeformation(FbxAMatrix& globalPosition, FbxMesh* mesh, FbxTime& time, FbxVector4* vertexArray, FbxPose* pose)
{
	FbxCluster::ELinkMode clusterMode = ((FbxSkin*)mesh->GetDeformer(0))->GetCluster(0)->GetLinkMode();
	int vertexCount = mesh->GetControlPointsCount();
	
	FbxAMatrix* clusterDeformation = new FbxAMatrix[vertexCount];
	memset(clusterDeformation, 0, vertexCount * sizeof(FbxAMatrix));

	double* clusterWeight = new double[vertexCount];
	memset(clusterWeight, 0, vertexCount * sizeof(double));

	int clusterCount = ((FbxSkin*)mesh->GetDeformer(0))->GetClusterCount();

	for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
	{
		FbxCluster* cluster = ((FbxSkin*)mesh->GetDeformer(0))->GetCluster(clusterIndex);
		if (!cluster->GetLink())
			continue;

		FbxAMatrix vertexTransformMatrix;
		computeClusterDeformation(globalPosition, mesh, cluster, vertexTransformMatrix, time, pose);

		int vertexIndexCount = cluster->GetControlPointIndicesCount();
		for (int i = 0; i < vertexIndexCount; i++)
		{
			int index = cluster->GetControlPointIndices()[i];

			if (index >= vertexCount)
				continue;

			double weight = cluster->GetControlPointWeights()[i];
			if (weight == 0.0)
				continue;

			FbxAMatrix influence = vertexTransformMatrix;

			//matrix scale
			matrixScale(influence, weight);
			matrixAdd(clusterDeformation[index], influence);
			clusterWeight[index] += weight;
		}
	}

	//compute actual verts, assume normalized
	for (int i = 0; i < vertexCount; i++)
	{
		FbxVector4 srcVertex = vertexArray[i];
		FbxVector4& dstVertex = vertexArray[i];
		double weight = clusterWeight[i];

		if (weight != 0.0)
		{
			dstVertex = clusterDeformation[i].MultT(srcVertex);
			dstVertex /= weight;
		}
	}

	delete[] clusterDeformation;
	delete[] clusterWeight;
}

void ModelReader::matrixScale(FbxAMatrix& matrix, double value)
{
	for (int m = 0; m < 4; m++)
	{
		for (int n = 0; n < 4; n++)
		{
			matrix[m][n] *= value;
		}
	}
}

void ModelReader::matrixAdd(FbxAMatrix& dstMatrix, FbxAMatrix& srcMatrix)
{
	for (int m = 0; m < 4; m++)
	{
		for (int n = 0; n < 4; n++)
		{
			dstMatrix[m][n] += srcMatrix[m][n];
		}
	}
}

std::vector<FbxNode*> ModelReader::getAllBones(FbxMesh* mesh)
{
	std::vector<FbxNode*> bones;

	FbxNode* root = ((FbxSkin*)mesh->GetDeformer(0))->GetCluster(0)->GetLink();

	//Find bone parent
	while (strcmp(root->GetParent()->GetTypeFlags()[0], "Skeleton") == 0)
	{
		root = root->GetParent();
	}

	//Get all bones
	std::stack<FbxNode*> stack;
	std::unordered_set<FbxNode*> visited;
	stack.push(root);

	while (!stack.empty())
	{
		FbxNode* node = stack.top();
		stack.pop();

		if (visited.find(node) != visited.end())
			continue;

		visited.insert(node);
		stack.push(node);

		int children = node->GetChildCount();
		for (int i = 0; i < children; i++)
		{
			FbxNode* child = node->GetChild(i);
			if (strcmp(child->GetTypeFlags()[0], "Skeleton") == 0)
			{
				bones.push_back(child);
			}
			else
			{
				visited.insert(child);
			}
		}
	}

	return bones;
}

ModelReader::~ModelReader()
{

}
