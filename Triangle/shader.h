
#pragma once

class CShader
{
private:
    GLuint program;

    void OutputShaderStatus(GLuint shader, GLenum pname, const char* name);
    void OutputProgramStatus(GLuint program, GLenum pname);

    bool GetSource(char** source, wchar_t* filename);

public:
    CShader();
    ~CShader();

    bool Create(wchar_t* filename1, wchar_t* filename2);
    void Destroy();
    GLuint Get();

    void Use();
};

