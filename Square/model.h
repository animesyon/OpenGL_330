
#pragma once

class CModel
{
private:
	int location;
	int count;
	GLuint vao;

	void Generate(int vertex_coordinate, int* stride, float** vertices, int* size1, unsigned int** indices, int* size2, int* count);

public:
	CModel();
	~CModel();

	void Create(GLuint handle);
	void Destroy();
	void Draw(float* matrix);
};
