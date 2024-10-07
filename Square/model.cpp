
#include "framework.h"
#include "model.h"

CModel::CModel()
{
    vao = 0;
}

CModel::~CModel()
{

}

void CModel::Generate(int vertex_coordinate, int* stride, float** vertices, int* size1, unsigned int** indices, int* size2, int* count)
{
    int k, allocsize;
    float x1, y1, x2, y2, z, l, w;

    *count = 4;
    allocsize = (*count) * vertex_coordinate;
    *vertices = new float[allocsize];

    *stride = vertex_coordinate * sizeof(float);
    *size1 = (*count) * (*stride);

    //         0               1
    //      y2 +---------------+
    //         |               |
    //         |s              |
    //         |        s      |
    //      y1 +---------------+
    //         x1              x2
    //         2               3
    //

    l = 40.0f;
    w = 15.0f;

    x1 = -l / 2.0f;
    x2 = x1 + l;

    y1 = -w / 2.0f;
    y2 = y1 + w;

    z = 0.0f;

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
    (*vertices)[k++] = x1;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = z;

    // 3
    (*vertices)[k++] = x2;
    (*vertices)[k++] = y1;
    (*vertices)[k++] = z;

    //                0     1
    //                +-----+
    //                |\    |
    //                | \   |
    //                |A \ B|
    //                |   \ |
    //                |    \|
    //                +-----+
    //                2     3

    * count = 6;
    *indices = new unsigned int[*count];

    *size2 = (*count) * sizeof(unsigned int);

    k = 0;

    // A
    (*indices)[k++] = 0;
    (*indices)[k++] = 3;
    (*indices)[k++] = 2;

    // B
    (*indices)[k++] = 0;
    (*indices)[k++] = 1;
    (*indices)[k++] = 3;
}

void CModel::Create(GLuint handle)
{
    float* vertices = NULL;
    unsigned int* indices = NULL;
    int vertex_coordinate, stride, size1, size2, vertex_attribute;
    long long vertex_offset;
    GLuint vbo, ebo;

    vertex_coordinate = 3;     // x, y, z

    vertex_offset = 0LL;

    vertex_attribute = glGetAttribLocation(handle, "vertex");
    location = glGetUniformLocation(handle, "matrix");

    Generate(vertex_coordinate, &stride, &vertices, &size1, &indices, &size2, &count);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size1, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(vertex_attribute, vertex_coordinate, GL_FLOAT, GL_FALSE, stride, (const void*)vertex_offset);
    glEnableVertexAttribArray(vertex_attribute);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size2, indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    if (vertices != NULL) delete[] vertices;
    if (indices != NULL) delete[] indices;
}

void CModel::Destroy()
{
    glDeleteVertexArrays(1, &vao);
}

void CModel::Draw(float* matrix)
{
    long long index_offset = 0LL;

    glUniformMatrix4fv(location, 1, false, matrix);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (const GLvoid*)index_offset);
    glBindVertexArray(0);
}
