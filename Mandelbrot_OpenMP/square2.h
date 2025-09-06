
#pragma once

class CSquare2
{
private:
	int location;
	int count;
	GLuint vao, vbo;

	void Generate(int vertex_size, int* stride, float** vertices, int* size, int* count);

public:
	CSquare2();
	~CSquare2();

	void Create(GLuint handle);
	void Destroy();
	void Draw(float* matrix);

	void Update(float x1, float y1, float x2, float y2);
};
