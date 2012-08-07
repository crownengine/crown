#include "Wall.h"
#include "Globals.h"
#include "Joystick.h"
#include <GL/glew.h>	// FIXME

Wall::Wall(Scene* scene, Camera* camera, real x, real y, real w, real h):
	PhysicNode(scene, camera, Vec3(x, y, 0), Angles(0, 0, 0), true, CG_WALL)
{
	mStoneTexture = GetTextureManager()->Load("res/chainsawbuffet/stone_wall_texture.bmp");

	SetUseIntegerCoordinatesForRendering(true);

	Crown::Rect* r = new Crown::Rect();
	r->SetFromCenterAndDimensions(Vec2::ZERO, w+5, h+5);
	//SetShape(r);

	SetInverseMass(0.0);

	bool meshCreated;
	mMesh = GetMeshManager()->Create("wall_mesh", meshCreated);

	if (mMesh != NULL && meshCreated)
	{
		MeshChunk* chunk = new MeshChunk();
		real halfSizeX, halfSizeY;
		real sizeZ;
		halfSizeX = w/2;
		halfSizeY = h/2;
		sizeZ = 80;
		real texWidth = 512;
		real texHeight = 512;
		//Ceiling Face
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX, -halfSizeY, sizeZ), Vec3::ZAXIS, Vec2(                  0.0f,                    0.0f)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX, -halfSizeY, sizeZ), Vec3::ZAXIS, Vec2(halfSizeX*2 / texWidth,                    0.0f)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX,  halfSizeY, sizeZ), Vec3::ZAXIS, Vec2(halfSizeX*2 / texWidth, halfSizeY*2 / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX,  halfSizeY, sizeZ), Vec3::ZAXIS, Vec2(                  0.0f, halfSizeY*2 / texHeight)));
		
		//Right Face
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX,  halfSizeY,   0.0), Vec3::XAXIS, Vec2(halfSizeY*2 / texWidth,              0.0f)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX,  halfSizeY, sizeZ), Vec3::XAXIS, Vec2(halfSizeY*2 / texWidth, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX, -halfSizeY, sizeZ), Vec3::XAXIS, Vec2(                  0.0f, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX, -halfSizeY,   0.0), Vec3::XAXIS, Vec2(                  0.0f,              0.0f)));

		//Bottom Face
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX, -halfSizeY,   0.0),-Vec3::YAXIS, Vec2(halfSizeX*2 / texWidth,              0.0f)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX, -halfSizeY, sizeZ),-Vec3::YAXIS, Vec2(halfSizeX*2 / texWidth, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX, -halfSizeY, sizeZ),-Vec3::YAXIS, Vec2(                  0.0f, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX, -halfSizeY,   0.0),-Vec3::YAXIS, Vec2(                  0.0f,              0.0f)));

		//Left Face
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX, -halfSizeY,   0.0),-Vec3::XAXIS, Vec2(                  0.0f,              0.0f)));
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX, -halfSizeY, sizeZ),-Vec3::XAXIS, Vec2(                  0.0f, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX,  halfSizeY, sizeZ),-Vec3::XAXIS, Vec2(halfSizeY*2 / texWidth, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX,  halfSizeY,   0.0),-Vec3::XAXIS, Vec2(halfSizeY*2 / texWidth,              0.0f)));

		//Top Face
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX,  halfSizeY,   0.0), Vec3::YAXIS, Vec2(                  0.0f,              0.0f)));
		chunk->mVertexList.Append(VertexData(Vec3(-halfSizeX,  halfSizeY, sizeZ), Vec3::YAXIS, Vec2(                  0.0f, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX,  halfSizeY, sizeZ), Vec3::YAXIS, Vec2(halfSizeX*2 / texWidth, sizeZ / texHeight)));
		chunk->mVertexList.Append(VertexData(Vec3( halfSizeX,  halfSizeY,   0.0), Vec3::YAXIS, Vec2(halfSizeX*2 / texWidth,              0.0f)));

		chunk->mFaceList.Append(FaceData( 0,  1,  2));
		chunk->mFaceList.Append(FaceData( 0,  2,  3));
		chunk->mFaceList.Append(FaceData( 4,  5,  6));
		chunk->mFaceList.Append(FaceData( 4,  6,  7));
		chunk->mFaceList.Append(FaceData( 8,  9, 10));
		chunk->mFaceList.Append(FaceData( 8, 10, 11));
		chunk->mFaceList.Append(FaceData(12, 13, 14));
		chunk->mFaceList.Append(FaceData(12, 14, 15));
		chunk->mFaceList.Append(FaceData(16, 17, 18));
		chunk->mFaceList.Append(FaceData(16, 18, 19));

		mMesh->AddMeshChunk(chunk);
		mMesh->RecompileMesh();
	}
}

Wall::~Wall()
{
}

void Wall::Update(real dt)
{
	PhysicNode::Update(dt);
}

void Wall::Render()
{
	PhysicNode::Render();
	Renderer* r = GetDevice()->GetRenderer();

	r->SetTexture(0, mStoneTexture);
	r->RenderVertexIndexBuffer(mMesh->GetVertexBuffer(), mMesh->GetIndexBuffer());
}
