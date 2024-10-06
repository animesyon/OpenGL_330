
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

bool CShader::GetSource(char** source, wchar_t* filename)
{
    FILE* fp;
    errno_t err;
    int size;

    if ((err = _wfopen_s(&fp, filename, L"rb")) != 0) return false;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *source = new char[size + 1];

    fread(*source, sizeof(char), size, fp);
    *(*source + size) = '\0';

    fclose(fp);

    return true;
}

bool CShader::Create(wchar_t* filename1, wchar_t* filename2)
{
    char* source = NULL;
    GLuint shader1, shader2;

    GetSource(&source, filename1);
    shader1 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader1, 1, (const char**)&source, NULL);
    glCompileShader(shader1);

    if (source != NULL) {
        delete[] source;
        source = NULL;
    }

    OutputShaderStatus(shader1, GL_COMPILE_STATUS, "Vertex Shader");

    GetSource(&source, filename2);
    shader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader2, 1, (const char**)&source, NULL);
    glCompileShader(shader2);

    if (source != NULL) {
        delete[] source;
        source = NULL;
    }

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

//
