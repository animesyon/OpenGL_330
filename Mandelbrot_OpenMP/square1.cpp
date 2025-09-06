
#include "framework.h"
#include "square1.h"

CSquare1::CSquare1()
{
    vao = 0;
}

CSquare1::~CSquare1()
{

}

void CSquare1::Generate(int vertex_size, int texture_size, int* stride, float** vertices, int* size, int* count, float width, float height)
{
    float x1, y1, x2, y2, z;
    int k, allocsize;
    float s1, t1, s2, t2;

    *count = 4;
    allocsize = (*count) * (vertex_size + texture_size);
    *vertices = new float[allocsize];

    *stride = (vertex_size + texture_size) * sizeof(float);
    *size = (*count) * (*stride);

    x1 = 0.0f;
    y1 = 0.0f;

    x2 = width;
    y2 = height;

    z = 0.0f;

    //            t2 y2 +---------------------+
    //                  |                     |
    //                  |                     |
    //                  |                     |
    //            t1 y1 +---------------------+
    //                  x1                    x2
    //                  s1                    s2

    s1 = 0.0f;
    s2 = 1.0f;

    t1 = 0.0f;
    t2 = 1.0f;

    k = 0;

    // 0
    (*vertices)[k++] = x1;
    (*vertices)[k++] = y2;
    (*vertices)[k++] = z;
    (*vertices)[k++] = s1;
    (*vertices)[k++] = t2;

    // 1
    (*vertices)[k++] = x2;
    (*vertices)[k++] = y2;
    (*vertices)[k++] = z;
    (*vertices)[k++] = s2;
    (*vertices)[k++] = t2;

    // 2
    (*vertices)[k++] = x2;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = z;
    (*vertices)[k++] = s2;
    (*vertices)[k++] = t1;

    // 3
    (*vertices)[k++] = x1;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = z;
    (*vertices)[k++] = s1;
    (*vertices)[k++] = t1;
}

void CSquare1::Create(GLuint handle, float width, float height)
{
    float* vertices = NULL;
    int vertex_size, texture_size, stride, size, vertex_index, texture_index;
    long long vertex_offset, texture_offset;
    GLuint vbo;

    vertex_size = 3;
    texture_size = 2;

    vertex_offset = 0LL;
    texture_offset = vertex_size * sizeof(float);

    vertex_index = glGetAttribLocation(handle, "v_vertex");
    texture_index = glGetAttribLocation(handle, "v_texture");
    location = glGetUniformLocation(handle, "m_matrix");

    Generate(vertex_size, texture_size, &stride, &vertices, &size, &count, width, height);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(vertex_index, vertex_size, GL_FLOAT, GL_FALSE, stride, (const void*)vertex_offset);
    glEnableVertexAttribArray(vertex_index);

    glVertexAttribPointer(texture_index, texture_size, GL_FLOAT, GL_FALSE, stride, (const void*)texture_offset);
    glEnableVertexAttribArray(texture_index);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);

    if (vertices != NULL) delete[] vertices;
}

void CSquare1::Destroy()
{
    glDeleteVertexArrays(1, &vao);
}

void CSquare1::Draw(float* matrix)
{
    glUniformMatrix4fv(location, 1, false, matrix);

    glBindVertexArray(vao);
    glDrawArrays(GL_QUADS, 0, count);
    glBindVertexArray(0);
}
