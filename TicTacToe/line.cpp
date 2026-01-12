
#include "framework.h"
#include "line.h"

CLine::CLine()
{
    vao = 0;
}

CLine::~CLine()
{

}

//               0                   1
//            y2 +-------------------+
//               |                   |
//               |                   |
//               |                   |
//               |                   |
//            y1 +-------------------+
//               x1                  x2
//               3                   2

void CLine::Generate(int vertex_size, int* stride, float** vertices, int* size, int* count, float x1, float y1, float x2, float y2)
{
    int k, allocsize;

    *count = 4;
    allocsize = (*count) * vertex_size;
    *vertices = new float[allocsize];

    *stride = vertex_size * sizeof(float);
    *size = (*count) * (*stride);

    k = 0;

    (*vertices)[k++] = x1;
    (*vertices)[k++] = y2;
    (*vertices)[k++] = 0.0f;

    (*vertices)[k++] = x2;
    (*vertices)[k++] = y2;
    (*vertices)[k++] = 0.0f;

    (*vertices)[k++] = x2;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = 0.0f;

    (*vertices)[k++] = x1;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = 0.0f;

}

void CLine::Create(GLuint handle)
{
    float* vertices = NULL;
    int vertex_size, stride, size, vertex_index;
    long long vertex_offset;

    vertex_size = 3;
    vertex_offset = 0LL;

    vertex_index = glGetAttribLocation(handle, "v_vertex");
    location1 = glGetUniformLocation(handle, "m_matrix");

    Generate(vertex_size, &stride, &vertices, &size, &count, 0.86f, 0.5f, 0.87f, 0.51f);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(vertex_index, vertex_size, GL_FLOAT, GL_FALSE, stride, (const void*)vertex_offset);
    glEnableVertexAttribArray(vertex_index);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (vertices != NULL) delete[] vertices;
}

void CLine::Destroy()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void CLine::Draw(float* matrix)
{
    glUniformMatrix4fv(location1, 1, false, matrix);

    glBindVertexArray(vao);
    glDrawArrays(GL_QUADS, 0, count);
    glBindVertexArray(0);
}

void CLine::Update(float x1, float y1, float x2, float y2)
{
    float* vertices;
    int k;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    vertices = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    k = 0;

    vertices[k++] = x1;
    vertices[k++] = y2;
    vertices[k++] = 0.0f;

    vertices[k++] = x2;
    vertices[k++] = y2;
    vertices[k++] = 0.0f;

    vertices[k++] = x2;
    vertices[k++] = y1;
    vertices[k++] = 0.0f;

    vertices[k++] = x1;
    vertices[k++] = y1;
    vertices[k++] = 0.0f;

    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void CLine::Update(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    float* vertices;
    int k;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    vertices = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    k = 0;

    vertices[k++] = x1;
    vertices[k++] = y1;
    vertices[k++] = 0.0f;

    vertices[k++] = x2;
    vertices[k++] = y2;
    vertices[k++] = 0.0f;

    vertices[k++] = x3;
    vertices[k++] = y3;
    vertices[k++] = 0.0f;

    vertices[k++] = x4;
    vertices[k++] = y4;
    vertices[k++] = 0.0f;

    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void CLine::Hide()
{
    Update(0.86f, 0.5f, 0.87f, 0.51f);
}
