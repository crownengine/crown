#include "WorldTerrain.h"
#include <time.h>
#include <cstdlib>
#include "Crown.h"
#include "perlin.h"
#include <iostream>

using namespace Crown;

int perlinSeed;
extern MovableCamera* cam;

void DrawWiredCube(float x, float y, float z, float size);

WorldTerrain::WorldTerrain()
{
	Clear();

	mChunkX = 0;
	mChunkZ = 0;
	mChunkH = 0;

	mDrawGrid = true;
	
	
	
	mPlayerChunkX = (CHUNKS_COUNT+1)/2.0f;
	mPlayerChunkZ = (CHUNKS_COUNT+1)/2.0f;
	mPlayerChunkH = CHUNKS_COUNT_H;
	mPlayerPosition = Vec3((mPlayerChunkX + 0.5) * CHUNK_SIZE, mPlayerChunkH * CHUNK_SIZE, (mPlayerChunkZ + 0.5) * CHUNK_SIZE);

	for(int x = 0; x < CHUNKS_COUNT; x++)
	{
		for(int z = 0; z < CHUNKS_COUNT; z++)
		{
			for(int h = 0; h < CHUNKS_COUNT_H; h++)
			{
					mChunks[x][z][h] = new Chunk(this);
			}
		}
	}

	BMPImageLoader bmp;
	Image* image = bmp.LoadFile("res/terrain.bmp");
	mTerrainTexture = new GLTexture();
	//mTerrainTexture->SetWrap(TW_CLAMP_TO_EDGE);
	mTerrainTexture->SetGenerateMipMaps(true);
	mTerrainTexture->SetFilter(TF_ANISOTROPIC);
	mTerrainTexture->CreateFromImage(image);

	delete image;

	GLfloat fogColor[4] = {0.457f, 0.754f, 1.0f, 1.0f};//{0.5, 0.5, 0.5, 1.0}; 

	glEnable (GL_FOG);
	glFogi (GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 10.0f);
	glFogfv (GL_FOG_COLOR, fogColor);
	glHint (GL_FOG_HINT, GL_NICEST);

	glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);

	mViewDistance = 128;
	CycleViewDistance();
}

WorldTerrain::~WorldTerrain()
{
	for(int x = 0; x < CHUNKS_COUNT; x++)
	{
		for(int z = 0; z < CHUNKS_COUNT; z++)
		{
			for(int h = 0; h < CHUNKS_COUNT_H; h++)
			{

					delete mChunks[x][z][h];
			}
		}
	}

	delete mTerrainTexture;
}

void WorldTerrain::Clear()
{
	for(int x = 0; x < WORLD_SIZE; x++)
	{
		for(int z = 0; z < WORLD_SIZE; z++)
		{
			for(int h = 0; h < WORLD_SIZE_H; h++)
			{
				mBlocks[x][z][h].opaque = false;
				mBlocks[x][z][h].faceVisibility = FV_NONE;
				mBlocks[x][z][h].light = 4;
			}
		}
	}
}

void WorldTerrain::RandomMap(int groundLevel, int seed, int octaves, int freq)
{
	Clear();

	Perlin per(octaves, freq, WORLD_SIZE_H/2, seed);

	for(int x = 0; x < WORLD_SIZE; x++)
	{
		for(int z = 0; z < WORLD_SIZE; z++)
		{
			int height = per.Get(x * 1.0f / WORLD_SIZE, z * 1.0f / WORLD_SIZE) - groundLevel;
			if (height < 0)
				height /= 6;
			height = (WORLD_SIZE_H / 2) + height;
			for(int h = 0; h < height; h++)
				mBlocks[x][z][h].opaque = true;
		}
	}

	int px = (int) (cam->GetPosition().x / CubeSize);
	int pz = (int) (cam->GetPosition().z / CubeSize);
	int ph = WORLD_SIZE_H-1;

	while(ph > 1 && !mBlocks[px][pz][ph-1].opaque)
		ph--;

	ph++;

	mPlayerChunkX = px / CHUNK_SIZE;
	mPlayerChunkZ = pz / CHUNK_SIZE;
	mPlayerChunkH = ph / CHUNK_SIZE;

	cam->SetPosition(Vec3(px*CubeSize, ph*CubeSize, pz*CubeSize));


	RecalculateVisibleBlocksSides();
	RecalculateSunlight();

	RegenerateTerrain();
}

void WorldTerrain::UpdateVisibleNeighbourBlocks(Crown::uint xC, Crown::uint zC, Crown::uint hC)
{
	for(int x = xC-1; x < (int)xC+2; x++)
	{
		if (x < 0 || x >= WORLD_SIZE)
			continue;
		for(int z = zC-1; z < (int)zC+2; z++)
		{
			if (z < 0 || z >= WORLD_SIZE)
				continue;
			for(int h = hC-1; h < (int)hC+2; h++)
			{
				if (h<0 || h>=WORLD_SIZE_H)
					continue;
				Block& b = mBlocks[x][z][h];
				b.faceVisibility = FV_NONE;
				if (b.opaque)
				{
					if (x == 0 || !mBlocks[x-1][z][h].opaque)
						b.faceVisibility |= FV_LEFT;
				
					if (z == 0 || !mBlocks[x][z-1][h].opaque)
						b.faceVisibility |= FV_BACK;

					if (h == 0 || !mBlocks[x][z][h-1].opaque)
						b.faceVisibility |= FV_BOTTOM;

					if (x == WORLD_SIZE-1 || !mBlocks[x+1][z][h].opaque)
						b.faceVisibility |= FV_RIGHT;

					if (z == WORLD_SIZE-1 || !mBlocks[x][z+1][h].opaque)
						b.faceVisibility |= FV_FRONT;

					if (h == WORLD_SIZE_H-1 || !mBlocks[x][z][h+1].opaque)
						b.faceVisibility |= FV_TOP;
				}

				Crown::uint cx = x / CHUNK_SIZE;
				Crown::uint cz = z / CHUNK_SIZE;
				Crown::uint ch = h / CHUNK_SIZE;

				if (cx < 0 || cz < 0 || ch < 0 || cx >= CHUNKS_COUNT || cz >= CHUNKS_COUNT || ch >= CHUNKS_COUNT_H)
					continue;

				if (b.faceVisibility == FV_NONE)
					mChunks[cx][cz][ch]->BlockRemoved(x, z, h);
				else
					mChunks[cx][cz][ch]->BlockRevealed(x, z, h);
			}
		}
	}
}

void WorldTerrain::RecalculateVisibleBlocksSides()
{
	for(int x = 0; x < WORLD_SIZE; x++)
	{
		for(int z = 0; z < WORLD_SIZE; z++)
		{
			for(int h = 0; h < WORLD_SIZE_H; h++)
			{
				Block& b = mBlocks[x][z][h];
				Crown::uint fv = FV_NONE;
				if (b.opaque)
				{
					if (x == 0 || !mBlocks[x-1][z][h].opaque)
						fv |= FV_LEFT;
					if (x == WORLD_SIZE-1 || !mBlocks[x+1][z][h].opaque)
						fv |= FV_RIGHT;
				
					if (z == 0 || !mBlocks[x][z-1][h].opaque)
						fv |= FV_BACK;
					if (z == WORLD_SIZE-1 || !mBlocks[x][z+1][h].opaque)
						fv |= FV_FRONT;

					if (h == 0 || !mBlocks[x][z][h-1].opaque)
						fv |= FV_BOTTOM;
					if (h == WORLD_SIZE_H-1 || !mBlocks[x][z][h+1].opaque)
						fv |= FV_TOP;
				}
				b.faceVisibility = fv;
			}
		}
	}
}

void WorldTerrain::RecalculateSunlight()
{
	for(int x = 0; x < WORLD_SIZE; x++)
	{
		for(int z = 0; z < WORLD_SIZE; z++)
		{
			mBlocks[x][z][WORLD_SIZE_H-1].light = 15;
		}
	}

	for(int x = 0; x < WORLD_SIZE; x++)
	{
		for(int z = 0; z < WORLD_SIZE; z++)
		{
			for(int h = WORLD_SIZE_H-2; h > 0 && !mBlocks[x][z][h].opaque; h--)
			{
				mBlocks[x][z][h].light = 15;
			}
		}
	}
}

void WorldTerrain::RegenerateTerrain()
{

	for(int x = 0; x < CHUNKS_COUNT; x++)
	{
		for(int z = 0; z < CHUNKS_COUNT; z++)
		{
			for(int h = 0; h < CHUNKS_COUNT_H; h++)
			{
					mChunks[x][z][h]->AssignLocation(x*CHUNK_SIZE	, z*CHUNK_SIZE, h*CHUNK_SIZE);
			}
		}
	}
}

void WorldTerrain::Render()
{
	glEnable(GL_TEXTURE_2D);
	Renderer* r = GetDevice()->GetRenderer();
	r->SetTexture(1, mTerrainTexture);
	//mTerrainTexture->MakeCurrent();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	mChunkRegenerationSlots = 30;

	for(int x = 0; x < CHUNKS_COUNT; x++)
	{
		for(int z = 0; z < CHUNKS_COUNT; z++)
		{
			for(int h = 0; h < CHUNKS_COUNT_H; h++)
			{
					mChunks[x][z][h]->Render();
			}
		}
	}

	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_TEXTURE_2D);
}

void WorldTerrain::SetPlayerPosition(Vec3 newPosition)
{
	newPosition /= CubeSize;
	Crown::uint chunkX = (Crown::uint) (newPosition.x / (CHUNK_SIZE)) - mChunkX;
	Crown::uint chunkZ = (Crown::uint) (newPosition.z / (CHUNK_SIZE)) - mChunkZ;
	Crown::uint chunkH = (Crown::uint) (newPosition.y / (CHUNK_SIZE)) - mChunkH;

	mPlayerPosition = newPosition;

	if (chunkX > (CHUNKS_COUNT+1)/2  && (mChunkX + CHUNKS_COUNT) * CHUNK_SIZE < WORLD_SIZE)
	{
		//Fai uno spostamento verso x+
		Chunk* tempChunks[CHUNKS_COUNT][CHUNKS_COUNT_H];

		mChunkX += 1;

		ExtractChunksFromX(tempChunks, 0);

		//Slide all chunks behind in the -x direction
		for(int x = 0; x < CHUNKS_COUNT-1; x++)
		{
			for(int z = 0; z < CHUNKS_COUNT; z++)
			{
				for(int h = 0; h < CHUNKS_COUNT_H; h++)
				{
						Chunk* c = mChunks[x+1][z][h];
						mChunks[x][z][h] = c;
						c->mRelX--;
						c->UpdateShouldRender();
				}
			}
		}

		ReplaceChunksInX(tempChunks, CHUNKS_COUNT-1);
	}
	else if (chunkX < (CHUNKS_COUNT+1)/2  && mChunkX > 0)
	{
		//Fai uno spostamento verso x-
		Chunk* tempChunks[CHUNKS_COUNT][CHUNKS_COUNT_H];

		mChunkX -= 1;

		ExtractChunksFromX(tempChunks, CHUNKS_COUNT-1);

		for(int x = CHUNKS_COUNT-1; x > 0; x--)
		{
			for(int z = 0; z < CHUNKS_COUNT; z++)
			{
				for(int h = 0; h < CHUNKS_COUNT_H; h++)
				{
						Chunk* c = mChunks[x-1][z][h];
						mChunks[x][z][h] = c;
						c->mRelX++;
						c->UpdateShouldRender();
				}
			}
		}

		ReplaceChunksInX(tempChunks, 0);
	}
	else if (chunkZ > (CHUNKS_COUNT+1)/2  && (mChunkZ + CHUNKS_COUNT) * CHUNK_SIZE < WORLD_SIZE)
	{
		//Fai uno spostamento verso z+
		Chunk* tempChunks[CHUNKS_COUNT][CHUNKS_COUNT_H];

		mChunkZ += 1;

		ExtractChunksFromZ(tempChunks, 0);

		for(int z = 0; z < CHUNKS_COUNT-1; z++)
		{
			for(int x = 0; x < CHUNKS_COUNT; x++)
			{
				for(int h = 0; h < CHUNKS_COUNT_H; h++)
				{
						Chunk* c = mChunks[x][z+1][h];
						mChunks[x][z][h] = c;
						c->mRelZ--;
						c->UpdateShouldRender();
				}
			}
		}

		ReplaceChunksInZ(tempChunks, CHUNKS_COUNT-1);	
	}
	else if (chunkZ < (CHUNKS_COUNT+1)/2  && mChunkZ > 0)
	{
		//Fai uno spostamento verso z-
		Chunk* tempChunks[CHUNKS_COUNT][CHUNKS_COUNT_H];

		mChunkZ -= 1;

		ExtractChunksFromZ(tempChunks, CHUNKS_COUNT-1);

		for(int z = CHUNKS_COUNT-1; z > 0; z--)
		{
			for(int x = 0; x < CHUNKS_COUNT; x++)
			{
				for(int h = 0; h < CHUNKS_COUNT_H; h++)
				{
						Chunk* c = mChunks[x][z-1][h];
						mChunks[x][z][h] = c;
						c->mRelZ++;
						c->UpdateShouldRender();
				}
			}
		}

		ReplaceChunksInZ(tempChunks, 0);
	}
	else if (chunkX != mPlayerChunkX || chunkZ != mPlayerChunkZ || chunkH != mPlayerChunkH)
	{
		if (chunkX >= CHUNKS_COUNT || chunkZ >= CHUNKS_COUNT || chunkH >= CHUNKS_COUNT_H)
			return;
		Crown::uint xOff = chunkX - mPlayerChunkX;
		Crown::uint zOff = chunkZ - mPlayerChunkZ;
		Crown::uint hOff = chunkH - mPlayerChunkH;
		mPlayerChunkX = chunkX;
		mPlayerChunkZ = chunkZ;
		mPlayerChunkH = chunkH;
		//Just update the ShouldDraw of chunks

		for(int x = 0; x < CHUNKS_COUNT; x++)
		{
			for(int z = 0; z < CHUNKS_COUNT; z++)
			{
				for(int h = 0; h < CHUNKS_COUNT_H; h++)
				{
						mChunks[x][z][h]->UpdateShouldRender();
						mChunks[x][z][h]->mRelX -= xOff;
						mChunks[x][z][h]->mRelZ -= zOff;
						mChunks[x][z][h]->mRelH -= hOff;
				}
			}
		}
	}
}

void WorldTerrain::LOL()
{
	Crown::uint x = rand() % CHUNK_SIZE;
	Crown::uint z = rand() % CHUNK_SIZE;

	Crown::uint h = 0;
	while (h < WORLD_SIZE_H-1 && mBlocks[x][z][h+1].opaque)
		h++;

	mBlocks[x][z][h].opaque = false;
	
	UpdateVisibleNeighbourBlocks(x, z, h);
}

void WorldTerrain::CycleViewDistance()
{
	if (mViewDistance == 256)
		mViewDistance = 32;
	else
		mViewDistance <<= 1;

	std::cout << "View distance changed: " << mViewDistance << std::endl;
	
	cam->SetFarClipDistance(mViewDistance);
	glFogf(GL_FOG_START, mViewDistance*CubeSize*0.35);
	glFogf(GL_FOG_END, mViewDistance*CubeSize);
}

void WorldTerrain::ExtractChunksFromX(Chunk* destChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint x)
{
	for(int z = 0; z < CHUNKS_COUNT; z++)
	{
		for(int h = 0; h < CHUNKS_COUNT_H; h++)
		{
				destChunks[z][h] = mChunks[x][z][h];
		}
	}
}

void WorldTerrain::ReplaceChunksInX(Chunk* srcChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint x)
{
	for(int z = 0; z < CHUNKS_COUNT; z++)
	{
		for(int h = 0; h < CHUNKS_COUNT_H; h++)
		{
				Chunk* c;
				c = srcChunks[z][h];
				mChunks[x][z][h] = c;
				c->AssignLocation((mChunkX + x) * CHUNK_SIZE, c->mZ, c->mH);
		}
	}
}

void WorldTerrain::ExtractChunksFromZ(Chunk* destChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint z)
{
	for(int x = 0; x < CHUNKS_COUNT; x++)
	{
		for(int h = 0; h < CHUNKS_COUNT_H; h++)
		{
				destChunks[x][h] = mChunks[x][z][h];
		}
	}
}

void WorldTerrain::ReplaceChunksInZ(Chunk* srcChunks[CHUNKS_COUNT][CHUNKS_COUNT_H], Crown::uint z)
{
	for(int x = 0; x < CHUNKS_COUNT; x++)
	{
		for(int h = 0; h < CHUNKS_COUNT_H; h++)
		{
				Chunk* c;
				c = srcChunks[x][h];
				mChunks[x][z][h] = c;
				c->AssignLocation(c->mX, (mChunkZ + z) * CHUNK_SIZE, c->mH);
		}
	}
}

//------------CHUNK-------------
Chunk::Chunk(WorldTerrain* terrain):
	mWorldTerrain(terrain), mX(0), mZ(0), mH(0), mRelX(0), mRelZ(0), mRelH(0), mIsDirty(false), mShouldRender(false),
	mCandidateBlocks(0),
	mVertexBuffer(NULL), mIndexBuffer(NULL),
	mVertex(NULL), mVertexCount(0)
{
	for(int i = 0; i < 6; i++)
	{
		mIndices[i] = NULL;
		mIndicesCount[i] = 0;
	}

	mVertexBuffer = GetDevice()->GetRenderer()->CreateVertexBuffer();
	mIndexBuffer = GetDevice()->GetRenderer()->CreateIndexBuffer();
}

Chunk::~Chunk()
{
	delete[] mVertex;
	for(int i = 0; i < 6; i++)
		if (mIndices[i] != NULL)
		{
			delete[] mIndices[i];
			mIndices[i] = NULL;
		}
}

void Chunk::Render()
{
	Box box;
	box.min = Vec3(mX*CubeSize, mH*CubeSize, mZ*CubeSize);
	box.max = Vec3((mX+CHUNK_SIZE)*CubeSize, (mH+CHUNK_SIZE)*CubeSize, (mZ+CHUNK_SIZE)*CubeSize);

	//Vec3 minToMax = box.max - box.min;
	//Compensate imprecise frustum culling
	//box.min -= minToMax/(18*1.6);
	//box.max += minToMax/(18*1.6);

	if (mShouldRender)// && cam->GetFrustum().IntersectsBox(box))
	{
		if (mWorldTerrain->mDrawGrid)
		{
			glColor3f(0.6f, 0.6f, 1.0f);
			DrawWiredCube(mX*CubeSize, mH*CubeSize, mZ*CubeSize, 16*CubeSize);
			glColor3f(1.0f, 1.0f, 1.0f);
		}

		if (mCandidateBlocks > 0)
		{

			if (mIsDirty && mWorldTerrain->mChunkRegenerationSlots > 0)
			{
				mWorldTerrain->mChunkRegenerationSlots--;
				mIsDirty = false;
				mWorldTerrain->RegeneratedChunks++;
				RegenerateChunk();
			}

			if (mVertexBuffer == NULL || mIndexBuffer == NULL)
				return;

			mVertexBuffer->MakeCurrent();
			mIndexBuffer->MakeCurrent();
		
			if (mRelX >= 0)
				glDrawElements(GL_TRIANGLES, mIndicesCount[FB_LEFT], GL_UNSIGNED_INT, (void*) (mIndicesOffsets[FB_LEFT]*sizeof(Crown::uint)));
			if (mRelX <= 0)
				glDrawElements(GL_TRIANGLES, mIndicesCount[FB_RIGHT], GL_UNSIGNED_INT, (void*) (mIndicesOffsets[FB_RIGHT]*sizeof(Crown::uint)));

			if (mRelZ >= 0)
				glDrawElements(GL_TRIANGLES, mIndicesCount[FB_BACK], GL_UNSIGNED_INT, (void*) (mIndicesOffsets[FB_BACK]*sizeof(Crown::uint)));
			if (mRelZ <= 0)
				glDrawElements(GL_TRIANGLES, mIndicesCount[FB_FRONT], GL_UNSIGNED_INT, (void*) (mIndicesOffsets[FB_FRONT]*sizeof(Crown::uint)));

			if (mRelH >= 0)
				glDrawElements(GL_TRIANGLES, mIndicesCount[FB_BOTTOM], GL_UNSIGNED_INT, (void*) (mIndicesOffsets[FB_BOTTOM]*sizeof(Crown::uint)));
			if (mRelH <= 0)
				glDrawElements(GL_TRIANGLES, mIndicesCount[FB_TOP], GL_UNSIGNED_INT, (void*) (mIndicesOffsets[FB_TOP]*sizeof(Crown::uint)));
		}
	}

	if (mVertex)
	{
		delete[] mVertex;
		mVertex = NULL;
		for(int i = 0; i < 6; i++)
		{
			if (mIndices[i] != NULL)
			{
				delete[] mIndices[i];
				mIndices[i] = NULL;
			}
		}
	}
}

void Chunk::UpdateShouldRender()
{
	float d = (float) (pow(mRelX*CHUNK_SIZE + CHUNK_SIZE / (mRelX < 0 ? -2.0 : 2.0), 2) +
										 pow(mRelZ*CHUNK_SIZE + CHUNK_SIZE / (mRelZ < 0 ? -2.0 : 2.0), 2));

	if (d > pow((float)mWorldTerrain->mViewDistance + 2*CHUNK_SIZE, 2))
		mShouldRender = false;
	else
		mShouldRender = true;
}

void Chunk::Dirty()
{
	mIsDirty = true;
}

void Chunk::AssignLocation(Crown::uint x, Crown::uint z, Crown::uint h)
{
	mX = x;
	mZ = z;
	mH = h;

	int xTo = mX + CHUNK_SIZE;
	int zTo = mZ + CHUNK_SIZE;
	int hTo = mH + CHUNK_SIZE;

	mCandidateBlocks = 0;

	mBlocksToRender.Clear();

	for(int x = mX; x < xTo; x++)
	{
		for(int z = mZ; z < zTo; z++)
		{
			for(int h = mH; h < hTo; h++)
			{
				Block& b = mWorldTerrain->mBlocks[x][z][h];
				if (!b.opaque || b.faceVisibility == 0)
					continue;

				BlockToRender bt = {&b, x, z, h};
				mBlocksToRender.Append(bt);
				mCandidateBlocks++;
			}
		}
	}

	mRelX = mX / CHUNK_SIZE - (mWorldTerrain->mChunkX + mWorldTerrain->mPlayerChunkX);
	mRelZ = mZ / CHUNK_SIZE - (mWorldTerrain->mChunkZ + mWorldTerrain->mPlayerChunkZ);
	mRelH = mH / CHUNK_SIZE - (mWorldTerrain->mChunkH + mWorldTerrain->mPlayerChunkH);

	mIsDirty = true;
	UpdateShouldRender();
}

void Chunk::BlockRemoved(Crown::uint x, Crown::uint z, Crown::uint h)
{
	Block* block = &mWorldTerrain->mBlocks[x][z][h];
	for(int i = 0; i<mBlocksToRender.GetSize(); i++)
	{
		if (mBlocksToRender[i].block == block)
		{
			mBlocksToRender[i] = mBlocksToRender[mBlocksToRender.GetSize()-1];
			mBlocksToRender.Remove(mBlocksToRender.GetSize()-1);
			mCandidateBlocks--;
			mIsDirty = true;
			return;
		}
	}
}

void Chunk::BlockRevealed(Crown::uint x, Crown::uint z, Crown::uint h)
{
	mIsDirty = true;
	Block* block = &mWorldTerrain->mBlocks[x][z][h];
	for(int i = 0; i<mBlocksToRender.GetSize(); i++)
	{
		if (mBlocksToRender[i].block == block)
		{
			return;
		}
	}
	BlockToRender bt = {block, x, z, h};
	mBlocksToRender.Append(bt);
	mCandidateBlocks++;
}

void Chunk::RegenerateChunk()
{
	mVertexCount = 0;
	for(int i = 0; i < 6; i++)
		mIndicesCount[i] = 0;

	if (mCandidateBlocks == 0)
	{
		mCandidateBlocks = 0;
		mVertexBuffer->SetVertexData((VertexBufferMode) (VBM_NORMAL_COORDS | VBM_TEXTURE_COORDS | VBM_COLOR_COORDS), NULL, 0);
		mIndexBuffer->SetIndexData(NULL, 0);
		return;
	}

	if (mVertex != NULL)
		delete[] mVertex;
	for(int i = 0; i < 6; i++)
		if (mIndices[i] != NULL)
			delete[] mIndices[i];

	mVertex = new VertexData[mCandidateBlocks * 24];
	for(int i = 0; i < 6; i++)
		mIndices[i] = new Crown::uint[mCandidateBlocks * 36];

	for(int i = 0; i<mBlocksToRender.GetSize(); i++)
	{
		BlockToRender& bt = mBlocksToRender[i];
		Block* b = bt.block;

		uchar light;

		if (b->faceVisibility & FV_TOP)
		{
			if (bt.h < WORLD_SIZE_H-1)
				light = mWorldTerrain->mBlocks[bt.x][bt.z][bt.h+1].light;
			AddFace(bt.x, bt.z, bt.h, FB_TOP, light);
		}

		if (b->faceVisibility & FV_BOTTOM)
		{
			if (bt.h > 0)
				light = mWorldTerrain->mBlocks[bt.x][bt.z][bt.h-1].light;
			AddFace(bt.x, bt.z, bt.h, FB_BOTTOM, light);
		}

		if (b->faceVisibility & FV_LEFT)
		{
			if (bt.x > 0)
				light = mWorldTerrain->mBlocks[bt.x-1][bt.z][bt.h].light;
			AddFace(bt.x, bt.z, bt.h, FB_LEFT, light);
		}

		if (b->faceVisibility & FV_RIGHT)
		{
			if (bt.x < WORLD_SIZE_H-1)
				light = mWorldTerrain->mBlocks[bt.x+1][bt.z][bt.h].light;
			AddFace(bt.x, bt.z, bt.h, FB_RIGHT, light);
		}

		if (b->faceVisibility & FV_FRONT)
		{
			if (bt.z < WORLD_SIZE_H-1)
				light = mWorldTerrain->mBlocks[bt.x][bt.z+1][bt.h].light;
			AddFace(bt.x, bt.z, bt.h, FB_FRONT, light);
		}

		if (b->faceVisibility & FV_BACK)
		{
			if (bt.z > 0)
				light = mWorldTerrain->mBlocks[bt.x][bt.z-1][bt.h].light;
			AddFace(bt.x, bt.z, bt.h, FB_BACK, light);
		}
	}

	mVertexBuffer->SetVertexData((VertexBufferMode) (VBM_NORMAL_COORDS | VBM_TEXTURE_COORDS | VBM_COLOR_COORDS), (float*)mVertex, mVertexCount);
	int totIndicesCount = 0;
	for(int i = 0; i < 6; i++)
		totIndicesCount += mIndicesCount[i];

	mIndexBuffer->SetIndexData(NULL, totIndicesCount);
	totIndicesCount = 0;
	for(int i = 0; i < 6; i++)
	{
		mIndicesOffsets[i] = totIndicesCount;
		if (mIndicesCount[i] > 0)
		{
			mIndexBuffer->SetIndexSubData(mIndices[i], totIndicesCount, mIndicesCount[i]);
			totIndicesCount += mIndicesCount[i];
		}
	}
}

void Chunk::AddFace(int x, int z, int h, FaceBuffer face, uchar light)
{
	float xx = x * CubeSize;
	float zz = z * CubeSize;
	float hh = h * CubeSize;

	FaceMaterial faceMaterial = GetMaterial(0, face);

	float lightFactor = light / 15.0f;
	AddFace(faceMaterial, Vec3(xx, hh, zz), lightFactor, mIndices[face], mIndicesCount[face]);
}

void Chunk::AddFace(const FaceMaterial& faceMaterial, Vec3 traslation, float lightFactor, Crown::uint* index, Crown::uint& indicesCount)
{
	Crown::uint firstIdx = mVertexCount;
	
	VertexData vd;
	
	vd = faceMaterial.vd0;
	vd.position += traslation;
	//vd.color *= lightFactor;
	memcpy(&mVertex[mVertexCount], &vd, sizeof(VertexData));
	mVertexCount++;
	
	vd = faceMaterial.vd1;
	vd.position += traslation;
	//vd.color *= lightFactor;
	memcpy(&mVertex[mVertexCount], &vd, sizeof(VertexData));
	mVertexCount++;
	
	vd = faceMaterial.vd2;
	vd.position += traslation;
	//vd.color *= lightFactor;
	memcpy(&mVertex[mVertexCount], &vd, sizeof(VertexData));
	mVertexCount++;
	
	vd = faceMaterial.vd3;
	vd.position += traslation;
	//vd.color *= lightFactor;
	memcpy(&mVertex[mVertexCount], &vd, sizeof(VertexData));
	mVertexCount++;
	
	index[indicesCount++] = firstIdx;
	index[indicesCount++] = firstIdx+1;
	index[indicesCount++] = firstIdx+2;

	index[indicesCount++] = firstIdx;
	index[indicesCount++] = firstIdx+2;
	index[indicesCount++] = firstIdx+3;
}


void DrawWiredCube(float x, float y, float z, float size)
{
	glBegin(GL_LINES);

	glVertex3f(x, y, z);
	glVertex3f(x, y, z + size);

	glVertex3f(x + size, y, z);
	glVertex3f(x + size, y, z + size);

	glVertex3f(x, y + size, z);
	glVertex3f(x, y + size, z + size);

	glVertex3f(x + size, y + size, z);
	glVertex3f(x + size, y + size, z + size);

	glVertex3f(x					 , y, z);
	glVertex3f(x + size, y, z);

	glVertex3f(x					 , y, z + size);
	glVertex3f(x + size, y, z + size);

	glVertex3f(x					 , y + size , z);
	glVertex3f(x + size, y + size , z);

	glVertex3f(x					 , y + size , z + size);
	glVertex3f(x + size, y + size , z + size);

	glVertex3f(x, y						, z);
	glVertex3f(x, y + size, z);

	glVertex3f(x + size, y					 , z);
	glVertex3f(x + size, y + size, z);

	glVertex3f(x, y						, z + size);
	glVertex3f(x, y + size, z + size);

	glVertex3f(x + size, y					 , z + size);
	glVertex3f(x + size, y + size, z + size);

	glEnd();
}

