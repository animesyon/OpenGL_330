
// GL_VERSION_1_5 GL_VERSION_2_0 GL_VERSION_3_0

#include "framework.h"
#include "square2.h"

CSquare2::CSquare2()
{
    vao = 0;
    vbo = 0;
}

CSquare2::~CSquare2()
{

}

void CSquare2::Generate(int vertex_size, int* stride, float** vertices, int* size, int* count)
{
    float x1, y1, x2, y2, z;
    int k, allocsize;

    *count = 4;
    allocsize = (*count) * vertex_size;
    *vertices = new float[allocsize];

    *stride = vertex_size * sizeof(float);
    *size = (*count) * (*stride);

    x1 = 0.0f;
    y1 = 0.0f;

    x2 = x1 + 1.0f;
    y2 = y1 + 1.0f;

    z = 0.0f;

    //               y2 +---------------------+
    //                  |                     |
    //                  |                     |
    //                  |                     |
    //               y1 +---------------------+
    //                  x1                    x2

    k = 0;

    // 0
    (*vertices)[k++] = x1;
    (*vertices)[k++] = y2;
    (*vertices)[k++] = z;

    // 1
    (*vertices)[k++] = x2;
    (*vertices)[k++] = y2;
    (*vertices)[k++] = z;

    // 2
    (*vertices)[k++] = x2;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = z;

    // 3
    (*vertices)[k++] = x1;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = z;
}

void CSquare2::Create(GLuint handle)
{
    float* vertices = NULL;
    int vertex_size, stride, size, vertex_index;
    long long vertex_offset;

    vertex_size = 3;

    vertex_offset = 0LL;

    vertex_index = glGetAttribLocation(handle, "v_vertex");
    location = glGetUniformLocation(handle, "m_matrix");

    Generate(vertex_size, &stride, &vertices, &size, &count);

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

void CSquare2::Destroy()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void CSquare2::Draw(float* matrix)
{
    glUniformMatrix4fv(location, 1, false, matrix);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_LOOP, 0, count);
    glBindVertexArray(0);
}

void CSquare2::Update(float x1, float y1, float x2, float y2)
{
    float* vertices;
    float z = 0.0f;
    int k;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    vertices = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

    k = 0;

    // 0
    vertices[k++] = x1;
    vertices[k++] = y2;
    vertices[k++] = z;

    // 1
    vertices[k++] = x2;
    vertices[k++] = y2;
    vertices[k++] = z;

    // 2
    vertices[k++] = x2;
    vertices[k++] = y1;
    vertices[k++] = z;

    // 3
    vertices[k++] = x1;
    vertices[k++] = y1;
    vertices[k++] = z;

    glUnmapBuffer(GL_ARRAY_BUFFER);
}
