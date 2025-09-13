
#include "framework.h"
#include "matrix.h"

// constructor
CMatrix::CMatrix()
{
	m.a[0] = 1.0f;    m.a[4] = 0.0f;    m.a[8] = 0.0f;    m.a[12] = 0.0f;
	m.a[1] = 0.0f;    m.a[5] = 1.0f;    m.a[9] = 0.0f;    m.a[13] = 0.0f;
	m.a[2] = 0.0f;    m.a[6] = 0.0f;    m.a[10] = 1.0f;    m.a[14] = 0.0f;
	m.a[3] = 0.0f;    m.a[7] = 0.0f;    m.a[11] = 0.0f;    m.a[15] = 1.0f;
}

// constructor
CMatrix::CMatrix(float a11, float a12, float a13, float a14,
	float a21, float a22, float a23, float a24,
	float a31, float a32, float a33, float a34,
	float a41, float a42, float a43, float a44)
{
	m.a[0] = a11; m.a[4] = a12; m.a[8] = a13; m.a[12] = a14;
	m.a[1] = a21; m.a[5] = a22; m.a[9] = a23; m.a[13] = a24;
	m.a[2] = a31; m.a[6] = a32; m.a[10] = a33; m.a[14] = a34;
	m.a[3] = a41; m.a[7] = a42; m.a[11] = a43; m.a[15] = a44;
}

// constructor
CMatrix::CMatrix(float* a)
{
	int i;

	for (i = 0; i < NA; i++)
		m.a[i] = a[i];
}

// destructor
CMatrix::~CMatrix()
{

}

// gluLookAt
// https://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml
// GluLookAt code
// https://www.opengl.org/wiki/GluLookAt_code
//  How to compute the matrix used by gluLookAt
// https://www.opengl.org/discussion_boards/showthread.php/123685-How-to-compute-the-matrix-used-by-gluLookAt
//
void CMatrix::LookAt(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz)
{
	float f, fx, fy, fz;
	float s, sx, sy, sz;
	float    ux, uy, uz;

	fx = centerx - eyex;
	fy = centery - eyey;
	fz = centerz - eyez;

	f = (float)sqrt(fx * fx + fy * fy + fz * fz);
	fx /= f;
	fy /= f;
	fz /= f;

	// cross product
	// s = f x up
	//
	sx = (fy * upz) - (fz * upy);
	sy = (fz * upx) - (fx * upz);
	sz = (fx * upy) - (fy * upx);

	s = (float)sqrt(sx * sx + sy * sy + sz * sz);
	sx /= s;
	sy /= s;
	sz /= s;

	// cross product
	// u = s x f
	//
	ux = (sy * fz) - (sz * fy);
	uy = (sz * fx) - (sx * fz);
	uz = (sx * fy) - (sy * fx);

	// multiply matrix
	// 
	//      |   sx   sy   sz  0  |     |   0.0   0.0   0.0   -eyex   |
	//      |   ux   uy   uz  0  |     |   0.0   0.0   0.0   -eyey   |
	//      |  -fx  -fy  -fz  0  |     |   0.0   0.0   0.0   -yeyz   |
	//      |   0    0    0   1  |     |   0.0   0.0   0.0    1      |

	CMatrix A(sx, sy, sz, 0.0f,
		ux, uy, uz, 0.0f,
		-fx, -fy, -fz, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	CMatrix B, C;

	B.Translate(-eyex, -eyey, -eyez);

	C = A * B;

	int i;

	for (i = 0; i < NA; ++i)
		this->m.a[i] = C.m.a[i];
}

// Orthographic Projection Matrix
// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml
// 
//          2                                                    rigth + left
//    ------------           0                0               -  -------------
//    right - left                                                right - left
//
//                            2                                  top + bottom
//                        ------------         0              -  -------------
//         0              top - bottom                           top - bottom
//
//                                               -2               far + near
//         0                    0           -------------     -  -------------
//                                            far - near          far - near
//
//         0                   0                   0                  1
//
//
void CMatrix::Orthographic(float left, float right, float bottom, float top, float znear, float zfar)
{
	m.a[0] = 2.0f / (right - left);     m.a[4] = 0.0f;                    m.a[8] = 0.0f;                    m.a[12] = -(right + left) / (right - left);
	m.a[1] = 0.0f;			          m.a[5] = 2.0f / (top - bottom);     m.a[9] = 0.0f;                    m.a[13] = -(top + bottom) / (top - bottom);
	m.a[2] = 0.0f;                    m.a[6] = 0.0f;                    m.a[10] = -2.0f / (zfar - znear);    m.a[14] = -(zfar + znear) / (zfar - znear);
	m.a[3] = 0.0f;                    m.a[7] = 0.0f;                    m.a[11] = 0.0f;                    m.a[15] = 1.0f;
}

//
//      |             2n                            right + left                        |
//      |       ------------           0            -------------          0            |
//      |       right - left                        right - left                        |
//      |                                                                               |
//      |                             2n            top + bottom                        |
//      |             0          ------------       -------------          0            |
//      |                        top - bottom       top - bottom                        |
//      |                                                                               |
//      |                                          -(far + near)         -2fn           |
//      |             0                0           -------------       -----------      |
//      |                                            far - near        far - near       |
//      |                                                                               |
//      |                                                                               |
//      |             0                0               -1                 0             |
//      |                                                                               |
//
void CMatrix::Perspective(float left, float right, float bottom, float top, float znear, float zfar)
{
	m.a[0] = (2.0f * znear) / (right - left);    m.a[4] = 0.0f;                              m.a[8] = (right + left) / (right - left);     m.a[12] = 0.0f;
	m.a[1] = 0.0f;                             m.a[5] = (2.0f * znear) / (top - bottom);     m.a[9] = (top + bottom) / (top - bottom);     m.a[13] = 0.0f;
	m.a[2] = 0.0f;                             m.a[6] = 0.0f;                              m.a[10] = -(zfar + znear) / (zfar - znear);     m.a[14] = (2.0f * zfar * znear) / (zfar - znear);
	m.a[3] = 0.0f;                             m.a[7] = 0.0f;                              m.a[11] = -1.0f;                              m.a[15] = 0.0f;
}

// Perspective Projection matrix
// https://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
//
//      f = cotangent(fovy / 2.0f);
//
//      |         f                                                   |
//      |       -------       0          0                 0          |
//      |       aspect                                                |
//      |                                                             |
//      |          0          f          0                  0         |
//      |                                                             |
//      |                           zFar + zNear    2 * zFar * zNear  |
//      |          0          0     ------------    ----------------  |
//      |                           zNear - zFar      zNear - zFar    |
//      |                                                             |
//      |          0          0           -1                0         |
//      |                                                             |
//
void CMatrix::Perspective(float fovy, float aspect, float znear, float zfar)
{
	float f = (float)atan(fovy / 2.0f);

	m.a[0] = f / aspect;     m.a[4] = 0.0f;     m.a[8] = 0.0f;                             m.a[12] = 0.0f;
	m.a[1] = 0.0f;           m.a[5] = f;        m.a[9] = 0.0f;                             m.a[13] = 0.0f;
	m.a[2] = 0.0f;           m.a[6] = 0.0f;     m.a[10] = (zfar + znear) / (znear - zfar);    m.a[14] = (2.0f * zfar * znear) / (znear - zfar);
	m.a[3] = 0.0f;           m.a[7] = 0.0f;     m.a[11] = -1.0f;                             m.a[15] = 0.0f;
}

// identity matrix
// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLoadIdentity.xml
// 
//      |  1  0  0  0  |
//      |  0  1  0  0  |
//      |  0  0  1  0  |
//      |  0  0  0  1  |
//
void CMatrix::Identity()
{
	m.a[0] = 1.0f;    m.a[4] = 0.0f;    m.a[8] = 0.0f;    m.a[12] = 0.0f;
	m.a[1] = 0.0f;    m.a[5] = 1.0f;    m.a[9] = 0.0f;    m.a[13] = 0.0f;
	m.a[2] = 0.0f;    m.a[6] = 0.0f;    m.a[10] = 1.0f;    m.a[14] = 0.0f;
	m.a[3] = 0.0f;    m.a[7] = 0.0f;    m.a[11] = 0.0f;    m.a[15] = 1.0f;
}

// scale matrix
// https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glScale.xml
// 
//      |  x  0  0  0  |
//      |  0  y  0  0  |
//      |  0  0  z  0  |
//      |  0  0  0  1  |
//
void CMatrix::Scale(float x, float y, float z)
{
	m.a[0] = x;		  m.a[4] = 0.0f;    m.a[8] = 0.0f;    m.a[12] = 0.0f;
	m.a[1] = 0.0f;    m.a[5] = y;       m.a[9] = 0.0f;    m.a[13] = 0.0f;
	m.a[2] = 0.0f;    m.a[6] = 0.0f;    m.a[10] = z;      m.a[14] = 0.0f;
	m.a[3] = 0.0f;    m.a[7] = 0.0f;    m.a[11] = 0.0f;   m.a[15] = 1.0f;
}

// Translation Matrix
// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml
// 
//      |  1  0  0  x  |
//      |  0  1  0  y  |
//      |  0  0  1  z  |
//      |  0  0  0  1  |
//
void CMatrix::Translate(float x, float y, float z)
{
	m.a[0] = 1.0f;     m.a[4] = 0.0f;     m.a[8] = 0.0f;     m.a[12] = x;
	m.a[1] = 0.0f;     m.a[5] = 1.0f;     m.a[9] = 0.0f;     m.a[13] = y;
	m.a[2] = 0.0f;     m.a[6] = 0.0f;     m.a[10] = 1.0f;     m.a[14] = z;
	m.a[3] = 0.0f;     m.a[7] = 0.0f;     m.a[11] = 0.0f;     m.a[15] = 1.0f;
}

// Rotation Matrix
// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml
//
void CMatrix::Rotate(float a, float rx, float ry, float rz)
{
	float x, y, z, w, sine;
	double angle;

	angle = a / 180.0 * M_PI;
	sine = (float)sin(angle / 2.0);

	x = rx * sine;
	y = ry * sine;
	z = rz * sine;
	w = (float)cos(angle / 2.0f);

	m.a[0] = 1.0f - 2.0f * (y * y + z * z);  m.a[4] = 2.0f * (x * y - z * w);         m.a[8] = 2.0f * (x * z + y * w);        m.a[12] = 0.0f;
	m.a[1] = 2.0f * (x * y + z * w);         m.a[5] = 1.0f - 2.0f * (x * x + z * z);  m.a[9] = 2.0f * (y * z - x * w);        m.a[13] = 0.0f;
	m.a[2] = 2.0f * (x * z - y * w);         m.a[6] = 2.0f * (z * y + x * w);         m.a[10] = 1.0f - 2.0f * (x * x + y * y); m.a[14] = 0.0f;
	m.a[3] = 0.0f;                           m.a[7] = 0.0f;                           m.a[11] = 0.0f;                          m.a[15] = 1.0f;
}

// overloaded = operator
void CMatrix::operator =(CMatrix m)
{
	int i;

	for (i = 0; i < NA; ++i)
		this->m.a[i] = m.m.a[i];
}

// overloaded * operator
CMatrix CMatrix::operator*(CMatrix m)
{
	int i, j, k;
	mat_type sop;

	for (i = 0; i < NB; i++)
	{
		for (j = 0; j < NB; j++)
		{
			sop.b[j][i] = 0.0f;

			for (k = 0; k < NB; k++)
			{
				sop.b[j][i] += (this->m.b[k][i] * m.m.b[j][k]);
			}
		}
	}

	return CMatrix(sop.a);
}

// overloaded *= operator
void CMatrix::operator *=(CMatrix m)
{
	*this = (*this) * m;
}

// operator to cast to a float*
CMatrix::operator float* ()
{
	return m.a;
}
