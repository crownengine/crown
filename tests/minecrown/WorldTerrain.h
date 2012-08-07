#ifndef __WORLD_TERRAIN_H__
#define __WORLD_TERRAIN_H__

#include "Crown.h"
#include "WorldMaterial.h"

using namespace Crown;

#define WORLD_SIZE 512//1024
#define WORLD_SIZE_H 128
#define CHUNK_SIZE 16
#define CHUNKS_COUNT 32 //Deve essere dispari
#define CHUNKS_COUNT_H (WORLD_SIZE_H/CHUNK_SIZE)

class WorldTerrain;

enum FaceVisibility
{
	FV_NONE		= 0,
	FV_TOP		= 1,
	FV_BOTTOM	= 2,
	FV_LEFT		= 4,
	FV_RIGHT	= 8,
	FV_FRONT	= 16,
	FV_BACK		= 32
};

enum FaceBuffer
{
	FB_TOP = 0,
	FB_BOTTOM = 1,
	FB_LEFT = 2,
	FB_RIGHT = 3,
	FB_FRONT = 4,
	FB_BACK = 5
};

struct Block
{
	bool opaque;
	uchar light;
	uchar faceVisibility;
};

struct BlockToRender
{
	Block* block;
	Crown::uint x, z, h;
};

class Chunk {

public:
	Chunk(WorldTerrain* terrain);
	~Chunk();

	void Render();

	void Dirty();

	void AssignLocation(Crown::uint x, Crown::uint z, Crown::uint h);
	void UpdateShouldRender();

	void BlockRemoved(Crown::uint x, Crown::uint z, Crown::uint h);
	void BlockRevealed(Crown::uint x, Crown::uint z, Crown::uint h);

private:
	WorldTerrain* mWorldTerrain;
	int mX, mZ, mH;
	int mRelX, mRelZ, mRelH;
	bool mIsDirty;
	bool mShouldRender;
	Crown::uint mCandidateBlocks;

	List<BlockToRender> mBlocksToRender;

	VertexBuffer* mVertexBuffer;
	IndexBuffer* mIndexBuffer;

	VertexData* mVertex;
	Crown::uint mVertexCount;
	Crown::uint* mIndices[6];
	Crown::uint mIndicesCount[6];
	Crown::uint mIndicesOffsets[6];

	void AddFace(int x, int z, int h, FaceBuffer orientation, uchar light);
	void AddFace(const FaceMaterial& faceMaterial, Vec3 traslation, float lightFactor, Crown::uint* index, Crown::uint& indicesCount);
	void RegenerateChunk();

	friend class WorldTerrain;
};


class WorldTerrain
{

public:
	
	WorldTerrain();

	~WorldTerrain();

	void RandomMap(int groundLevel, int seed, int octaves, int freq);

	void Render();

	void SetPlayerPosition(Vec3 newPosition);

	inline Vec3 GetPlayerPosition() const
	{
		return mPlayerPosition;
	}

	void LOL();

	//params
	void ToggleGrid() { mDrawGrid = !mDrawGrid; }

	void CycleViewDistance();

	//Debug infos
	int RegeneratedChunks;

private:

	Block mBlocks[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE_H];
	Chunk* mChunks[CHUNKS_COUNT][CHUNKS_COUNT][CHUNKS_COUNT_H];
	Texture* mTerrainTexture;

	int mChunkRegenerationSlots;

	Vec3 mPlayerPosition;
	Crown::uint mChunkX;
	Crown::uint mChunkZ;
	Crown::uint mChunkH;

	// Player chunks relative to mChunkX, mChunkZ, mChunkH
	Crown::uint mPlayerChunkX;
	Crown::uint mPlayerChunkZ;
	Crown::uint mPlayerChunkH;

	bool mDrawGrid;
	int mViewDistance;


	void ExtractChunksFromX(Chunk* destChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint x);
	void ReplaceChunksInX(Chunk* srcChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint x);
	void ExtractChunksFromZ(Chunk* destChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint z);
	void ReplaceChunksInZ(Chunk* srcChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint z);

	void RegenerateTerrain();

	void RecalculateVisibleBlocksSides();
	void RecalculateSunlight();
	void UpdateVisibleNeighbourBlocks(Crown::uint x, Crown::uint z, Crown::uint h);

	void Clear();

	friend class Chunk;
};


#endif //__WORLD_TERRAIN_H__
