
#pragma once

class CSquare1
{
private:
	int location;
	int count;
	GLuint vao;

	void Generate(int vertex_size, int texture_size, int* stride, float** vertices, int* size, int* count, float width, float height);

public:
	CSquare1();
	~CSquare1();

	void Create(GLuint handle, float width, float height);
	void Destroy();
	void Draw(float* matrix);
};
