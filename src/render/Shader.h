#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <string>
#include <unordered_map>

class Shader
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use() const;
    unsigned int id() const { return m_program; }

    void setMat4(const char* name, const glm::mat4& value);
    void setVec3(const char* name, const glm::vec3& value);
    void setFloat(const char* name, float value);
    void setInt(const char* name, int value);

private:
    unsigned int m_program = 0;
    std::unordered_map<std::string, int> m_uniformLocationCache;

    static std::string readFileText(const std::string& path);
    static unsigned int compileShader(unsigned int type, const std::string& source, const std::string& debugPath);
    static unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader,
                                    const std::string& vertexPath, const std::string& fragmentPath);

    int uniformLocation(const char* name);
};
