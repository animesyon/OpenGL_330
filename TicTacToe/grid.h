
#pragma once

class CGrid
{
private:
	int location1, location2;
	int count;
	GLuint vao;

	void Generate(int vertex_size, int* stride, float** vertices, int* size, int* count);

public:
	CGrid();
	~CGrid();

	void Create(GLuint handle);
	void Destroy();
	void Draw(float* matrix);
};
