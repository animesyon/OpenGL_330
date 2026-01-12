
#pragma once

class CCircle
{
private:
	int location;
	int count;
	GLuint vao;

	void Generate(int vertex_size, int* stride, float** vertices, int* size, int* count);

public:
	CCircle();
	~CCircle();

	void Create(GLuint handle);
	void Destroy();
	void Draw(float* matrix);
};
