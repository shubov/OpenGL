//Шубов Михаил Павлович БПИ164
//Реализовано Солнце, Лодка с мачтой и парусом, Звезда.
//На каждый объект приходится по одному VAO, VBO и IBO(Index Buffer Object)
//Окрашен фон окна
//При изменении размера окна пропорции не меняются

#include "common_header.h"
#include "win_OpenGLApp.h"
#include "shaders.h"

CShader shVertex, shFragment;
CShaderProgram spMain;

const float PI = 3.141592653589f;

const int vertices_per_angle = 3;
const int sun_vertices_len = 360 * vertices_per_angle + 1 + 1;

//Moves object by offset
void offsetElement(float* v, int len, float x, float y, float z);
//Scale figure by 3 dimensions
void scaleElement (float* v, int len, float x, float y, float z);
//Generate sun
void createSun(float*& vert, unsigned int*& ind);

unsigned int uiVBO[3];
unsigned int uiVAO[3];
unsigned int IBO[3];

const int star_vertices_len = 60;
const int star_indices_len = 36;
const int boat_vertices_len = 66;
const int boat_indices_len = 15;

void SetupFigure(unsigned int &vao, unsigned int &vbo, unsigned int &ibo, float pos[], int pos_len, unsigned int ind[], int ind_len) {
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, pos_len * sizeof(float), pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)12);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind_len * sizeof(unsigned int), ind, GL_STATIC_DRAW);
}

void DrawElement(unsigned int& vao, unsigned int& ibo, unsigned int ind_len) {
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, ind_len, GL_UNSIGNED_INT, nullptr);
}

// Initializes OpenGL features that will be used.
// lpParam - Pointer to anything you want.
void InitScene(LPVOID lpParam)
{
	glClearColor(0.1f, 0.2f, 0.2f, 1.0f);

	// Setup a star
	float star_vertices[star_vertices_len] = {
		0.9902113033f, 0.8618033989f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.8449027977f, 0.8618033989f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.6097886967f, 0.8618033989f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.7273457472f, 0.7763932023f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.9175570505f, 0.6381966011f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.8726542528f, 0.7763932023f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.8f,          1.0f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.7550972023f, 0.8618033989f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.6824429495f, 0.6381966011f, 0.0f, 0.98f, 0.83f, 0.25f,
		0.8f, 0.7236067977f, 0.0f, 0.98f, 0.83f, 0.25f,
	};

	unsigned int star_indices[star_indices_len] = {
		6,3,5,
		2,3,7,
		3,8,9,
		3,9,5,
		9,4,5,
		5,0,1
	};

	// Setup a boat
	float boat_vertices[boat_vertices_len] = {
		0.0f, -0.8f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.2f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.8f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.6f, -0.8f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.275f, -0.5f, 0.0f, 0.75f, 0.6f, 0.41f,
		-0.275f, 0.3f, 0.0f, 0.75f, 0.6f, 0.41f,
		-0.325f, -0.5f, 0.0f, 0.75f, 0.6f, 0.41f,
		-0.325f, 0.3f, 0.0f, 0.75f, 0.6f, 0.41f,
		-0.275f, 0.3f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.275f, -0.3f, 0.0f, 0.0f, 0.0f, 1.0f,
	};
	
	unsigned int boat_indices[boat_indices_len] = {
		0,1,3,
		3,1,2,
		4,6,7,
		7,5,4,
		8,9,10
	};

	float* sun_vertices;
	unsigned int* sun_indices;
	createSun(sun_vertices, sun_indices);
	scaleElement(sun_vertices, sun_vertices_len * 6, 0.15f, 0.3f, 0.3f);
	offsetElement(sun_vertices, sun_vertices_len * 6, -0.65f, 0.35f, 0.0f);

	glGenVertexArrays(3, uiVAO); // Generate two VAOs, one for triangle and one for quad
	glGenBuffers(3, uiVBO); // And four VBOs
	glGenBuffers(3, IBO);

	SetupFigure(uiVAO[0], uiVBO[0], IBO[0], star_vertices, star_vertices_len, star_indices, star_indices_len);
	SetupFigure(uiVAO[1], uiVBO[1], IBO[1], boat_vertices, boat_vertices_len, boat_indices, boat_indices_len);
	SetupFigure(uiVAO[2], uiVBO[2], IBO[2], sun_vertices, sun_vertices_len * 6, sun_indices, sun_vertices_len * 3);

	// Load shaders and create shader program

	shVertex.LoadShader("data\\shaders\\shader.vert", GL_VERTEX_SHADER);
	shFragment.LoadShader("data\\shaders\\shader.frag", GL_FRAGMENT_SHADER);

	spMain.CreateProgram();
	spMain.AddShaderToProgram(&shVertex);
	spMain.AddShaderToProgram(&shFragment);

	spMain.LinkProgram();
	spMain.UseProgram();
}

// Renders whole scene.
// lpParam - Pointer to anything you want.
void RenderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	// We just clear color
	glClear(GL_COLOR_BUFFER_BIT);
	
	DrawElement(uiVAO[0], IBO[0], star_indices_len);
	DrawElement(uiVAO[1], IBO[1], boat_indices_len);
	DrawElement(uiVAO[2], IBO[2], sun_vertices_len * 3);

	oglControl->SwapBuffersM();
}

// Releases OpenGL scene.
// lpParam - Pointer to anything you want.
void ReleaseScene(LPVOID lpParam)
{
	spMain.DeleteProgram();
	shVertex.DeleteShader();
	shFragment.DeleteShader();
}

void scaleElement(float* v, int len, float x, float y, float z) {
	if (len % 3 != 0)
		return;
	for (size_t i = 0; i < len; i+=3) {
		v[i++] *= x;
		v[i++] *= y;
		v[i++] *= z;
	}
}

void offsetElement(float* v, int len, float x, float y, float z) {
	if (len % 3 != 0)
		return;
	for (size_t i = 0; i < len; i+=3) {
		v[i++] += x;
		v[i++] += y;
		v[i++] += z;
	}
}

void createSun(float*& vertices, unsigned int*& indices) {
	vertices = new float[sun_vertices_len * 6];
	indices = new unsigned int[sun_vertices_len * 3];

	float radius = 1.01f;
	float delta = 0.009;

	//centre of the sun (pos)
	vertices[0] = 0.0f; vertices[1] = 0.0f; vertices[2] = 0.0f; 
	//colour
	vertices[3] = 1.0f; vertices[4] = 0.98f; vertices[5] = 0.0f;

	for (int i = 6; i < sun_vertices_len * 6; ) {
		//rays
		if (i / 2 % 40 <= 5)
			radius = 1.5f;
		else
			radius = 1.0f;

		radius += delta;
		float angle = i / 2 / 3 - 1;
		
		//position of new vertex
		vertices[i++] = 0 + sin((PI / 180.0) * angle) * radius;
		vertices[i++] = 0 + cos((PI / 180.0) * angle) * radius;
		vertices[i++] = 0.0f;
		//colour
		vertices[i++] = 1.0f;
		vertices[i++] = 0.98f;
		vertices[i++] = 0.0f;
	}
	//filling the index buffer
	for (int i = 0, j = 1; 
			i < sun_vertices_len * 3 - 3, 
			j < sun_vertices_len - 1;
		) {
		indices[i++] = 0; 
		indices[i++] = j++;
		indices[i++] = j;
	}
}