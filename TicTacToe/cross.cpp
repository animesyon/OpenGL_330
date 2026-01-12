
#include "framework.h"
#include "cross.h"

CCross::CCross()
{
    vao = 0;
}

CCross::~CCross()
{

}

void CCross::Generate(int vertex_size, int* stride, float** vertices, int* size, int* count)
{
    int k, allocsize;

    *count = 4;
    allocsize = (*count) * vertex_size;
    *vertices = new float[allocsize];

    *stride = vertex_size * sizeof(float);
    *size = (*count) * (*stride);

    k = 0;

    // L1
    (*vertices)[k++] = -0.1f;
    (*vertices)[k++] = -0.1f;
    (*vertices)[k++] =  0.0f;

    (*vertices)[k++] =  0.1f;
    (*vertices)[k++] =  0.1f;
    (*vertices)[k++] =  0.0f;

    // L2
    (*vertices)[k++] =  0.1f;
    (*vertices)[k++] = -0.1f;
    (*vertices)[k++] =  0.0f;

    (*vertices)[k++] = -0.1f;
    (*vertices)[k++] =  0.1f;
    (*vertices)[k++] =  0.0f;

}

void CCross::Create(GLuint handle)
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

void CCross::Destroy()
{
    glDeleteVertexArrays(1, &vao);
}

void CCross::Draw(float* matrix)
{
    glUniformMatrix4fv(location1, 1, false, matrix);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, count);
    glBindVertexArray(0);
}
