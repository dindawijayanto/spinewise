#include "render/Shader.h"

#include <glad/gl.h>

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    const std::string vertexSource = readFileText(vertexPath);
    const std::string fragmentSource = readFileText(fragmentPath);

    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

    try
    {
        m_program = linkProgram(vertexShader, fragmentShader, vertexPath, fragmentPath);
    }
    catch (...)
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        throw;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_program(std::exchange(other.m_program, 0)),
      m_uniformLocationCache(std::move(other.m_uniformLocationCache))
{
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (m_program != 0)
    {
        glDeleteProgram(m_program);
    }

    m_program = std::exchange(other.m_program, 0);
    m_uniformLocationCache = std::move(other.m_uniformLocationCache);
    return *this;
}

void Shader::use() const
{
    glUseProgram(m_program);
}

void Shader::setMat4(const char* name, const glm::mat4& value)
{
    glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const char* name, const glm::vec3& value)
{
    glUniform3fv(uniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setFloat(const char* name, float value)
{
    glUniform1f(uniformLocation(name), value);
}

void Shader::setInt(const char* name, int value)
{
    glUniform1i(uniformLocation(name), value);
}

std::string Shader::readFileText(const std::string& path)
{
    std::ifstream file(path, std::ios::in);
    if (!file.is_open())
    {
        throw std::runtime_error("[Shader] Gagal membuka file shader: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source, const std::string& debugPath)
{
    const unsigned int shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE)
    {
        return shader;
    }

    int logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<size_t>(logLength > 1 ? logLength : 1));
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    const char* typeText = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";

    std::string message = "[Shader] Compile gagal (" + std::string(typeText) + ") pada " + debugPath + "\n" +
                          std::string(log.data());
    std::fprintf(stderr, "%s\n", message.c_str());

    glDeleteShader(shader);
    throw std::runtime_error(message);
}

unsigned int Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader,
                                 const std::string& vertexPath, const std::string& fragmentPath)
{
    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_TRUE)
    {
        return program;
    }

    int logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<size_t>(logLength > 1 ? logLength : 1));
    glGetProgramInfoLog(program, logLength, nullptr, log.data());

    std::string message = "[Shader] Link program gagal untuk pasangan shader:\n  VS: " + vertexPath +
                          "\n  FS: " + fragmentPath + "\n" + std::string(log.data());
    std::fprintf(stderr, "%s\n", message.c_str());

    glDeleteProgram(program);
    throw std::runtime_error(message);
}

int Shader::uniformLocation(const char* name)
{
    const auto found = m_uniformLocationCache.find(name);
    if (found != m_uniformLocationCache.end())
    {
        return found->second;
    }

    const int location = glGetUniformLocation(m_program, name);
    m_uniformLocationCache.emplace(name, location);
    return location;
}
