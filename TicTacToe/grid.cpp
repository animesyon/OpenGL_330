
#include "framework.h"
#include "grid.h"

CGrid::CGrid()
{
    vao = 0;
}

CGrid::~CGrid()
{

}

void CGrid::Generate(int vertex_size, int* stride, float** vertices, int* size, int* count)
{
    int k, allocsize;

    *count = 8;
    allocsize = (*count) * vertex_size;
    *vertices = new float[allocsize];

    *stride = vertex_size * sizeof(float);
    *size = (*count) * (*stride);

    k = 0;

    // V1
    (*vertices)[k++] = -0.15f;
    (*vertices)[k++] =  0.45f;
    (*vertices)[k++] =  0.0f;

    (*vertices)[k++] = -0.15f;
    (*vertices)[k++] = -0.45f;
    (*vertices)[k++] =  0.0f;

    // V2
    (*vertices)[k++] = 0.15f;
    (*vertices)[k++] = 0.45f;
    (*vertices)[k++] = 0.0f;

    (*vertices)[k++] =  0.15f;
    (*vertices)[k++] = -0.45f;
    (*vertices)[k++] =  0.0f;

    // H1
    (*vertices)[k++] = -0.45f;
    (*vertices)[k++] = -0.15f;
    (*vertices)[k++] =  0.0f;

    (*vertices)[k++] =  0.45f;
    (*vertices)[k++] = -0.15f;
    (*vertices)[k++] =  0.0f;

    // H2
    (*vertices)[k++] = -0.45f;
    (*vertices)[k++] =  0.15f;
    (*vertices)[k++] =  0.0f;

    (*vertices)[k++] = 0.45f;
    (*vertices)[k++] = 0.15f;
    (*vertices)[k++] = 0.0f;

}

void CGrid::Create(GLuint handle)
{
    float* vertices = NULL;
    int vertex_size, stride, size, vertex_index;
    long long vertex_offset;
    GLuint vbo;

    vertex_size = 3;
    vertex_offset = 0LL;

    vertex_index = glGetAttribLocation(handle, "v_vertex");
    location1 = glGetUniformLocation(handle, "m_matrix");

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

    glDeleteBuffers(1, &vbo);

    if (vertices != NULL) delete[] vertices;
}

void CGrid::Destroy()
{
    glDeleteVertexArrays(1, &vao);
}

void CGrid::Draw(float* matrix)
{
    glUniformMatrix4fv(location1, 1, false, matrix);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, count);
    glBindVertexArray(0);
}
