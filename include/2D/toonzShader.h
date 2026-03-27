#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QMatrix4x4>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <types.h>

//TODO: Create shader managers to handle these objects ... 

class toonzShader : protected QOpenGLFunctions {
public:
    //----- variables -----
    UINT ID;

    //----- constructor -----
    toonzShader(const char* vertexPath, const char* fragmentPath);
   
    //---- Shader activation ---
    inline void use() { glUseProgram(ID); };


    //--- Uniform utility functions ---
    void setBool(const std::string &name, bool value)  { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); };

    void setInt(const std::string &name, int value)  { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); };

    void setFloat(const std::string &name, float value)  { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); };

    void set4X4M(const std::string &name,  QMatrix4x4 &value) {glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value.constData()); };

private:

    //----- utility function for checking shader compilation/linking errors -----
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif