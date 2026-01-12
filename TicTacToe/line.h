
#pragma once

class CLine
{
private:
	int location1, location2;
	int count;
	GLuint vao, vbo;

	void Generate(int vertex_size, int* stride, float** vertices, int* size, int* count, float x1, float y1, float x2, float y2);

public:
	CLine();
	~CLine();

	void Create(GLuint handle);
	void Destroy();
	void Draw(float* matrix);

	void Update(float x1, float y1, float x2, float y2);
	void Update(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

	void Hide();
};
