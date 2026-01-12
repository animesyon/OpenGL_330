
#pragma once

class CShader
{
private:
    GLuint program;

    void OutputShaderStatus(GLuint shader, GLenum pname, const char* name);
    void OutputProgramStatus(GLuint program, GLenum pname);

public:
    CShader();
    ~CShader();

    bool Create(const char* source1, const char* source2);
    void Destroy();
    GLuint Get();

    void Use();
};

