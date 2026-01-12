
#pragma once

class CCross
{
private:
	int location1, location2;
	int count;
	GLuint vao;

	void Generate(int vertex_size, int* stride, float** vertices, int* size, int* count);

public:
	CCross();
	~CCross();

	void Create(GLuint handle);
	void Destroy();
	void Draw(float* matrix);
};
