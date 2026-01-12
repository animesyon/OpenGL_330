
#include "framework.h"
#include "circle.h"

CCircle::CCircle()
{
    vao = 0;
}

CCircle::~CCircle()
{

}

void CCircle::Generate(int vertex_size, int* stride, float** vertices, int* size, int* count)
{
    int i, k, n, allocsize;
    double a, angle, dpd, radius;
    float x, y, z;

    n = 32;

    *count = n;
    allocsize = (*count) * vertex_size;
    *vertices = new float[allocsize];

    *stride = vertex_size * sizeof(float);
    *size = (*count) * (*stride);

    radius = 0.1;
    dpd = 360.0 / (double)n;

    k = 0;
    z = 0.0f;
    a = 0.0;

    for (i = 0; i < (*count); i++) {

        angle = a / 180.0 * M_PI;

        x = (float)(radius * cos(angle));
        y = (float)(radius * sin(angle));

        (*vertices)[k++] = x;
        (*vertices)[k++] = y;
        (*vertices)[k++] = z;

        a += dpd;
    }
}

void CCircle::Create(GLuint handle)
{
    float* vertices = NULL;
    int vertex_size, stride, size, vertex_index;
    long long vertex_offset;
    GLuint vbo;

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

    glDeleteBuffers(1, &vbo);

    if (vertices != NULL) delete[] vertices;
}

void CCircle::Destroy()
{
    glDeleteVertexArrays(1, &vao);
}

void CCircle::Draw(float* matrix)
{
    glUniformMatrix4fv(location, 1, false, matrix);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_LOOP, 0, count);
    glBindVertexArray(0);
}
