
#include "framework.h"
#include "shader.h"

CShader::CShader()
{
    program = NULL;
}

CShader::~CShader()
{
}


void CShader::OutputShaderStatus(GLuint shader, GLenum pname, const char* name)
{
    GLint result, size;
    char* infolog = NULL;

    glGetShaderiv(shader, pname, &result);

    if (!result) {

        OutputDebugStringA(name);
        OutputDebugStringA("\n");

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);

        infolog = new char[size];

        glGetShaderInfoLog(shader, size, NULL, infolog);

        OutputDebugStringA(infolog);
        OutputDebugStringA("\n");
    }

    if (infolog != NULL) delete[] infolog;
}

void CShader::OutputProgramStatus(GLuint program, GLenum pname)
{
    GLint result, size;
    char* infolog = NULL;

    glGetProgramiv(program, pname, &result);

    if (!result) {

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);

        infolog = new char[size];

        glGetProgramInfoLog(program, size, NULL, infolog);

        OutputDebugStringA(infolog);
        OutputDebugStringA("\n");
    }

    if (infolog != NULL) delete[] infolog;
}


bool CShader::Create(char* source1, char* source2)
{
    GLuint shader1, shader2;

    shader1 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader1, 1, (const char**)&source1, NULL);
    glCompileShader(shader1);

    OutputShaderStatus(shader1, GL_COMPILE_STATUS, "Vertex Shader");

    shader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader2, 1, (const char**)&source2, NULL);
    glCompileShader(shader2);

    OutputShaderStatus(shader2, GL_COMPILE_STATUS, "Fragment Shader");

    program = glCreateProgram();

    glAttachShader(program, shader1);
    glAttachShader(program, shader2);

    glLinkProgram(program);

    OutputProgramStatus(program, GL_LINK_STATUS);

    glDetachShader(program, shader1);
    glDetachShader(program, shader2);

    glDeleteShader(shader1);
    glDeleteShader(shader2);

    return true;
}

void CShader::Destroy()
{
    glDeleteProgram(program);
}

GLuint CShader::Get()
{
    return program;
}

void CShader::Use()
{
    glUseProgram(program);
}
