#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct MeshRegion {
    std::string name; // "Cervical", "Thoracic", "Lumbar", "Skeleton_Body"
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

class ObjLoader {
public:
    static std::vector<MeshRegion> LoadSkeleton(const std::string& filepath);
};