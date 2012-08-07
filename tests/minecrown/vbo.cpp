#include "Crown.h"
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "Types.h"

#define HMSIZE 256
#define CubeSize 2/3.0f

void DrawAxis();
void DrawGrid(float xStart, float zStart, float xStep, float zStep);
void DrawWiredCube(float x, float y, float z);
void DrawCube(float x, float y, float z);
void DrawVoxelGrid(float xStart, float yStart, float zStart, float xStep, float yStep, float zStep);

using namespace Crown;

enum Visibility
{
	V_Visible,
	V_Hidden_Opt,
	V_Hidden,
};

enum FaceOrientation
{
	FO_TOP,
	FO_BOTTOM,
	FO_LEFT,
	FO_RIGHT,
	FO_FRONT,
	FO_BACK
};

struct Block {
	Visibility visible;
};

class Chunk {

public:
	Chunk() {
		Clear();
		vb = NULL;
		ib = NULL;
	}

	void Clear() {

		mVertexCount = 0;
		mVertexList.Clear();

		for(int x = 0; x < HMSIZE; x++) {
			for(int z = 0; z < HMSIZE; z++) {
				for(int h = 0; h < HMSIZE; h++) {
					mBlocks[x][z][h].visible = V_Hidden;
				}
			}
		}
	}

	void Render() {

		vb->MakeCurrent();
		ib->MakeCurrent();
		ib->Render(Vec3(0, 0, 0), Vec3(0, 0, 0), 0.0f);
	}

	void RandomMap() {

		srand(time(NULL));
		Clear();

		for(int n = 0; n < 100; n++) {
			int xmin, xmax, zmin, zmax;
			int a, b;
			a = rand()%HMSIZE;
			b = rand()%HMSIZE;
			xmin = Math::Min(a, b);
			xmax = Math::Max(a, b);

			a = rand()%HMSIZE;
			b = rand()%HMSIZE;
			zmin = Math::Min(a, b);
			zmax = Math::Max(a, b);

			for(int x = xmin; x < xmax; x++) {
				for(int z = zmin; z < zmax; z++) {

					int h = 0;
					while (mBlocks[x][z][h].visible == V_Visible)
						h++;
					mBlocks[x][z][h].visible = V_Visible;
				}
			}
		}

		RegenerateVertexList();
	}

	void RegenerateVertexList() {

		mVertexCount = 0;
		mVertexList.Clear();

		for(int x = 0; x < HMSIZE; x++) {
			for(int z = 0; z < HMSIZE; z++) {
				for(int h = 0; h < HMSIZE; h++) {

					if (mBlocks[x][z][h].visible == V_Visible)
						GenerateFaces(x, z, h);
				}
			}
		}

		if (!vb)
			vb = GetDevice()->GetRenderer()->CreateVertexBuffer();
		if (!ib)
			ib = GetDevice()->GetRenderer()->CreateIndexBuffer();

		vb->SetVertexData(VBM_COLOR_COORDS, mVertexList.GetData(), mVertexCount);

		List<unsigned int> indexes;
		for(int i=0; i<mVertexCount; i++)
			indexes.Append(i);
		ib->SetIndexData(indexes.GetData(), mVertexCount);

		mVertexList.Clear();
	}

	void GenerateFaces(int x, int z, int h) {

		AddFace(x, z, h, FO_TOP   , x  , z  , h+1);
		AddFace(x, z, h, FO_BOTTOM, x  , z  , h-1);
		AddFace(x, z, h, FO_LEFT  , x-1, z  , h);
		AddFace(x, z, h, FO_RIGHT , x+1, z  , h);
		AddFace(x, z, h, FO_FRONT , x  , z+1, h);
		AddFace(x, z, h, FO_BACK  , x  , z-1, h);
	}

	void AddFace(int x, int z, int h, FaceOrientation orientation, int x1, int z1, int h1) {

		if (!(x1 < 0 || x1 > HMSIZE-1 || z1 < 0 || z1 > HMSIZE-1 || h1 < 0 || h1 > HMSIZE-1))
			if (mBlocks[x1][z1][h1].visible == V_Visible)
				return;

		float xx = x * CubeSize;
		float zz = z * CubeSize;
		float hh = h * CubeSize;

		switch (orientation) {

			case FO_TOP:
				AppendVertex(xx           , hh + CubeSize, zz);
				AppendColor(.3f, .0f, .0f);
				AppendVertex(xx + CubeSize, hh + CubeSize, zz);
				AppendColor(.25f, .0f, .0f);
				AppendVertex(xx + CubeSize, hh + CubeSize, zz - CubeSize);
				AppendColor(.3f, .0f, .0f);
				AppendVertex(xx           , hh + CubeSize, zz - CubeSize);
				AppendColor(.25f, .0f, .0f);
				break;

			case FO_BOTTOM:
				AppendVertex(xx           , hh, zz);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx           , hh, zz - CubeSize);
				AppendColor(.0f, .0f, .25f);
				AppendVertex(xx + CubeSize, hh, zz - CubeSize);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx + CubeSize, hh, zz);
				AppendColor(.0f, .0f, .25f);
				break;

			case FO_LEFT:
				AppendVertex(xx, hh           , zz);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx, hh + CubeSize, zz);
				AppendColor(.0f, .0f, .25f);
				AppendVertex(xx, hh + CubeSize, zz - CubeSize);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx, hh           , zz - CubeSize);
				AppendColor(.0f, .0f, .25f);
				break;

			case FO_RIGHT:
				AppendVertex(xx + CubeSize, hh           , zz);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx + CubeSize, hh           , zz - CubeSize);
				AppendColor(.0f, .0f, .25f);
				AppendVertex(xx + CubeSize, hh + CubeSize, zz - CubeSize);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx + CubeSize, hh + CubeSize, zz);
				AppendColor(.0f, .0f, .25f);
				break;

			case FO_FRONT:
				AppendVertex(xx           , hh           , zz);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx + CubeSize, hh           , zz);
				AppendColor(.0f, .0f, .25f);
				AppendVertex(xx + CubeSize, hh + CubeSize, zz);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx           , hh + CubeSize, zz);
				AppendColor(.0f, .0f, .25f);
				break;

			case FO_BACK:
				AppendVertex(xx           , hh           , zz - CubeSize);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx           , hh + CubeSize, zz - CubeSize);
				AppendColor(.0f, .0f, .25f);
				AppendVertex(xx + CubeSize, hh + CubeSize, zz - CubeSize);
				AppendColor(.0f, .0f, .3f);
				AppendVertex(xx + CubeSize, hh           , zz - CubeSize);
				AppendColor(.0f, .0f, .25f);
				break;

		}

		mVertexCount += 4;
	}

	void AppendVertex(float x, float y, float z) {

		mVertexList.Append(x);
		mVertexList.Append(y);
		mVertexList.Append(z);
	}

	void AppendColor(float r, float g, float b) {

		mVertexList.Append(r);
		mVertexList.Append(g);
		mVertexList.Append(b);
	}

	void Optimize() {
		
		for(int x = 1; x < HMSIZE-1; x++) {
			for(int y = 1; y < HMSIZE-1; y++) {
				for(int z = 1; z < HMSIZE-1; z++) {
					if (mBlocks[x][y][z].visible == V_Visible) {
						if (mBlocks[x-1][y][z].visible != V_Visible && mBlocks[x-1][y][z].visible != V_Hidden_Opt)
							continue;
						if (mBlocks[x+1][y][z].visible != V_Visible && mBlocks[x+1][y][z].visible != V_Hidden_Opt)
							continue;
						if (mBlocks[x][y-1][z].visible != V_Visible && mBlocks[x][y-1][z].visible != V_Hidden_Opt)
							continue;
						if (mBlocks[x][y+1][z].visible != V_Visible && mBlocks[x][y+1][z].visible != V_Hidden_Opt)
							continue;
						if (mBlocks[x][y][z-1].visible != V_Visible && mBlocks[x][y][z-1].visible != V_Hidden_Opt)
							continue;
						if (mBlocks[x][y][z+1].visible != V_Visible && mBlocks[x][y][z+1].visible != V_Hidden_Opt)
							continue;
						mBlocks[x][y][z].visible = V_Hidden_Opt;
					}
				}
			}
		}

		for(int x = 1; x < HMSIZE-1; x++) {
			for(int y = 1; y < HMSIZE-1; y++) {
				for(int z = 1; z < HMSIZE-1; z++) {
					if (mBlocks[x][y][z].visible == V_Hidden_Opt) {
						mBlocks[x][y][z].visible = V_Hidden;
					}
				}
			}
		}
	}

private:

	Block mBlocks[HMSIZE][HMSIZE][HMSIZE];
	List<float> mVertexList;
	int mVertexCount;
	VertexBuffer* vb;
	IndexBuffer* ib;
};

Chunk chunk;

class InputReceiver : public EventReceiver {

public:

	InputReceiver() {
		mDevice = GetDevice();
		mDevice->GetEventDispatcher()->RegisterEventReceiver(this);
	}

	void HandleEvent(const Event& event) {

		if (event.event_type == ET_KEYBOARD) {
			if (event.keyboard.type == KET_RELEASED) {
				if (event.keyboard.key == KK_ESCAPE) {

					if (mDevice) {
						mDevice->StopRunning();
					}
				}

				if (event.keyboard.key == KK_SPACE) {

					chunk.RandomMap();
				}
			}
		}
	}

private:

	Device* mDevice;
};

int main(int argc, char** argv) {

	int wndW, wndH;
	wndW = 1280;
	wndH = 800;
	bool full = false;

	if (argc == 3) {
		wndW = atoi(argv[1]);
		wndH = atoi(argv[2]);
	}

	Device* device = GetDevice();


	if (!device->Init(wndW, wndH, 32, false)) {

		return 0;
	}

	InputReceiver ir;

	Renderer* renderer = device->GetRenderer();
	SceneManager* smgr = device->GetSceneManager();
	ResourceManager* resman = device->GetResourceManager();

	device->GetMainWindow()->SetTitle("Crown Engine v0.1");
	device->GetMainWindow()->GetCursorControl()->SetVisible(true);
	device->GetMainWindow()->SetFullscreen(full);

	Scene* scene = new Scene();

	MovableCamera* cam;
	cam = scene->AddMovableCamera(
		0,
		Vec3(0.0f, 3.0f, HMSIZE*CubeSize),
		Angles(0, 0, 0),
		Vec3(1, 1, 1),
		true,
		90.0f,
		1.59f,
		true,
		0.5,
		1);
	cam->SetActive(true);
	cam->SetSpeed(0.1);
	cam->SetFarClipDistance(150.0f);

	std::cout << "Entity count: " << scene->GetEntityCount() << std::endl;
	std::cout << "Light count: " << scene->GetLightCount() << std::endl;
	std::cout << "Sizeof RenderWindow: " << sizeof(RenderWindow) << std::endl;
/*
	Material material;
	material.mAmbient = Color(0.1, 0.1, 0.1, 1);
	material.mDiffuse = Color(1, 1, 1, 1);
	material.mSpecular = Color(0, 0, 0, 1);
	material.mShininess = 128;
	material.mSeparateSpecularColor = true;
	material.mTexturing = false;
	material.mLighting = true;

	renderer->SetMaterial(material);
*/
	Mat4 identity;
	identity.LoadIdentity();

	renderer->SetClearColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

	Mat4 ortho;
	ortho.BuildProjectionOrthoRH(wndW, wndH, 1, -1);

	Mat4 perspective;
	perspective.BuildProjectionPerspectiveRH(90.0f, 1.59f, 0.1f, 100.0f);

	Mat4 text;
	text.LoadIdentity();
	text.SetTranslation(Vec3(400, 350, 0));


	//Crown::Font font;
	//Image* testImg = font.LoadFont("tests/font/arialbd.ttf");

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	chunk.RandomMap();


////struct MyVertex
//// {
////   float x, y, z;        //Vertex
////	 float r, g, b;
////   //float nx, ny, nz;     //Normal
////   //float s0, t0;         //Texcoord0
//// };
////
//// MyVertex pvertex[4];
//// //VERTEX 0
//// pvertex[0].x = 0.0;
//// pvertex[0].y = 0.0;
//// pvertex[0].z = 0.0;
//// pvertex[0].r = 1.0f;
//// pvertex[0].g = 0.0f;
//// pvertex[0].b = 0.0f;
////
//// //VERTEX 1
//// pvertex[1].x = 1.0;
//// pvertex[1].y = 0.0;
//// pvertex[1].z = 0.0;
//// pvertex[1].r = 1.0f;
//// pvertex[1].g = 1.0f;
//// pvertex[1].b = 0.0f;
////
//// //VERTEX 2
//// pvertex[2].x = 1.0;
//// pvertex[2].y = 1.0;
//// pvertex[2].z = 0.0;
//// pvertex[2].r = 0.0f;
//// pvertex[2].g = 1.0f;
//// pvertex[2].b = 0.0f;
////
//// pvertex[3].x = 0.0;
//// pvertex[3].y = 1.0;
//// pvertex[3].z = 0.0;
//// pvertex[3].r = 0.0f;
//// pvertex[3].g = 1.0f;
//// pvertex[3].b = 0.0f;
////
////
//// uint VertexVBOID;
//// uint IndexVBOID;
////
//// glGenBuffers(1, &VertexVBOID);
//// glBindBuffer(GL_ARRAY_BUFFER, VertexVBOID);
//// glBufferData(GL_ARRAY_BUFFER, sizeof(MyVertex)*4, pvertex, GL_DYNAMIC_DRAW);
////
//// ushort pindices[4];
//// pindices[0] = 0;
//// pindices[1] = 1;
//// pindices[2] = 2;
//// pindices[3] = 3;
////
//// glGenBuffers(1, &IndexVBOID);
//// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexVBOID);
//// glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ushort)*4, pindices, GL_DYNAMIC_DRAW);
////
//// //Define this somewhere in your header file
//// #define BUFFER_OFFSET(i) ((char *)NULL + (i))



	while (device->IsRunning()) {

		WindowEventHandler::GetInstance()->ManageEvents();
		GetDevice()->GetMainWindow()->GetRenderContext()->MakeCurrent();

		// ----------- Begin Scene -----------
		renderer->BeginScene();

		renderer->SetMatrix(MT_PROJECTION, ortho);
		//renderer->SetMatrix(MT_MODEL, text);

		glDisable(GL_LIGHTING);
		glColor3f(1, 1, 1);

		//glDrawPixels(testImg->GetWidth(), testImg->GetHeight(), GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, testImg->GetBuffer());

		cam->Render();

		//renderer->SetMatrix(MT_PROJECTION, perspective);

		renderer->SetMatrix(MT_MODEL, identity);

		DrawAxis();
		//DrawGrid(-0.0f, 0.0f, 2/3.0f, 2/3.0f);
		//DrawVoxelGrid(0.0f, 0.0f, 0.0f, 2/3.0f, 2/3.0f, 2/3.0f);

		chunk.Render();
		////glBindBuffer(GL_ARRAY_BUFFER, VertexVBOID);
		////glEnableClientState(GL_VERTEX_ARRAY);
		////glVertexPointer(3, GL_FLOAT, sizeof(MyVertex), BUFFER_OFFSET(0));   //The starting point of the VBO, for the vertices
		////
		////glEnableClientState(GL_COLOR_ARRAY);
		////glColorPointer(3, GL_FLOAT, sizeof(MyVertex), BUFFER_OFFSET(12));

		////glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexVBOID);
		//////To render, we can either use glDrawElements or glDrawRangeElements
		//////The is the number of indices. 3 indices needed to make a single triangle
		////glDrawElements(GL_QUADS, 4, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));   //The starting point of the IBO

		renderer->EndScene();
		// ----------- End Scene -------------

		GetDevice()->GetMainWindow()->Update();
		GetDevice()->GetMainWindow()->SetTitle(Str(renderer->GetFPS()).c_str());
	}

	//glDeleteBuffers(1, &VertexVBOID);
	//glDeleteBuffers(1, &IndexVBOID);


	//delete testImg;

	device->Shutdown();

	//delete testImg;

	return 0;

}

void DrawAxis() {

	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();
}

void DrawGrid(float xStart, float zStart, float xStep, float zStep) {

	glColor3f(.5f, .5f, .5f);
	const unsigned int steps = 128;
	for (unsigned int i = 0; i <= steps; i++) {
		glBegin(GL_LINES);
		glVertex3f(xStart + xStep * (float)i, -1.0f, zStart);
		glVertex3f(xStart + xStep * (float)i, -1.0f, zStart-steps*zStep);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(xStart, -1.0f, zStart - zStep * (float)i);
		glVertex3f(xStart+steps*xStep, -1.0f, zStart - zStep * (float)i);
		glEnd();
	}
}

void DrawWiredCube(float x, float y, float z) {

	glColor3f(.5f, .5f, .5f);

	glBegin(GL_LINES);

	glVertex3f(x, y, z);
	glVertex3f(x, y, z - CubeSize);

	glVertex3f(x + CubeSize, y, z);
	glVertex3f(x + CubeSize, y, z - CubeSize);

	glVertex3f(x, y + CubeSize, z);
	glVertex3f(x, y + CubeSize, z - CubeSize);

	glVertex3f(x + CubeSize, y + CubeSize, z);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);

	glVertex3f(x					 , y, z);
	glVertex3f(x + CubeSize, y, z);

	glVertex3f(x					 , y, z - CubeSize);
	glVertex3f(x + CubeSize, y, z - CubeSize);

	glVertex3f(x					 , y + CubeSize , z);
	glVertex3f(x + CubeSize, y + CubeSize , z);

	glVertex3f(x					 , y + CubeSize , z - CubeSize);
	glVertex3f(x + CubeSize, y + CubeSize , z - CubeSize);

	glVertex3f(x, y						, z);
	glVertex3f(x, y + CubeSize, z);

	glVertex3f(x + CubeSize, y					 , z);
	glVertex3f(x + CubeSize, y + CubeSize, z);

	glVertex3f(x, y						, z - CubeSize);
	glVertex3f(x, y + CubeSize, z - CubeSize);

	glVertex3f(x + CubeSize, y					 , z - CubeSize);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);

	glEnd();
}

void DrawCube(float x, float y, float z) {

	glBegin(GL_QUADS);

	
	//top
	glColor3f(.3f, .0f, .0f);
	glVertex3f(x           , y + CubeSize, z);
	glColor3f(.25f, .0f, .0f);
	glVertex3f(x + CubeSize, y + CubeSize, z);
	glColor3f(.3f, .0f, .0f);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);
	glColor3f(.25f, .0f, .0f);
	glVertex3f(x           , y + CubeSize, z - CubeSize);

	//bottom
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x           , y, z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x           , y, z - CubeSize);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y, z);

	//left
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x, y           , z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x, y + CubeSize, z);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x, y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x, y           , z - CubeSize);

	//right
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y           , z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y           , z - CubeSize);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y + CubeSize, z);

	//front
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x           , y           , z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y           , z);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y + CubeSize, z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x           , y + CubeSize, z);

	//back
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x           , y           , z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x           , y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y           , z - CubeSize);

	glEnd();
}

void DrawVoxelGrid(float xStart, float yStart, float zStart, float xStep, float yStep, float zStep) {

	glColor3f(.5f, .5f, .5f);

	const unsigned int steps = 16;
	glBegin(GL_LINES);
	for (unsigned int i = 0; i <= steps; i++) {
		
		for(unsigned int j = 0; j <= steps; j++) {
			glVertex3f(xStart + xStep * (float)i, j * yStep, zStart);
			glVertex3f(xStart + xStep * (float)i, j * yStep, zStart-steps*zStep);

			glVertex3f(xStart + xStep * (float)j, yStart, zStart - zStep * (float)i);
			glVertex3f(xStart + xStep * (float)j, yStart+steps*yStep, zStart - zStep * (float)i);

			glVertex3f(xStart, j * yStep, zStart - zStep * (float)i);
			glVertex3f(xStart+steps*xStep, j * yStep, zStart - zStep * (float)i);
		}
		
	}
	glEnd();
}

