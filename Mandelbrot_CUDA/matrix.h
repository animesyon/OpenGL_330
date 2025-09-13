
#pragma once

const int NA = 16;
const int NB = 4;

union mat_type {
    float a[NA];
    float b[NB][NB];
};

class CMatrix
{
private:
    mat_type m;

public:
    CMatrix();
    CMatrix(float a11, float a12, float a13, float a14,
        float a21, float a22, float a23, float a24,
        float a31, float a32, float a33, float a34,
        float a41, float a42, float a43, float a44);
    CMatrix(float* a);
    ~CMatrix();

    void LookAt(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz);

    void Orthographic(float left, float right, float bottom, float top, float znear, float zfar);
    void Perspective(float left, float right, float bottom, float top, float znear, float zfar);
    void Perspective(float fovy, float aspect, float znear, float zfar);

    void Identity();
    void Scale(float x, float y, float z);
    void Translate(float x, float y, float z);
    void Rotate(float a, float rx, float ry, float rz);

    void operator =(CMatrix m);

    CMatrix operator*(CMatrix m);

    void operator *=(CMatrix m);

    operator float* ();
};
