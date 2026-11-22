#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "ObjLoader.h"

#include <cstdint>
#include <iostream>
#include <unordered_map>

namespace
{
// Kunci unik satu vertex OBJ: kombinasi index posisi dan index normal.
std::uint64_t vertexKey(const tinyobj::index_t& index)
{
    const auto pos = static_cast<std::uint32_t>(index.vertex_index);
    const auto nrm = static_cast<std::uint32_t>(index.normal_index); // -1 -> 0xFFFFFFFF (tanpa normal)
    return (static_cast<std::uint64_t>(pos) << 32) | nrm;
}
} // namespace

std::vector<MeshRegion> ObjLoader::LoadSkeleton(const std::string& filepath) {
    std::vector<MeshRegion> regions;
    tinyobj::ObjReaderConfig config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, config)) {
        if (!reader.Error().empty()) {
            std::cerr << "[ObjLoader] Error: " << reader.Error() << std::endl;
        }
        return regions;
    }
    if (!reader.Warning().empty()) {
        std::cerr << "[ObjLoader] Warning: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    for (const auto& shape : shapes) {
        MeshRegion region;
        region.name = shape.name;
        region.indices.reserve(shape.mesh.indices.size());

        // Vertex yang sama (posisi + normal sama) dipakai ulang lewat index,
        // supaya mesh skeleton tidak di-expand jadi satu vertex per index.
        std::unordered_map<std::uint64_t, unsigned int> uniqueVertices;

        for (const auto& index : shape.mesh.indices) {
            const std::uint64_t key = vertexKey(index);
            const auto found = uniqueVertices.find(key);
            if (found != uniqueVertices.end()) {
                region.indices.push_back(found->second);
                continue;
            }

            Vertex v{};
            v.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.normal_index >= 0) {
                v.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            const auto newIndex = static_cast<unsigned int>(region.vertices.size());
            uniqueVertices.emplace(key, newIndex);
            region.indices.push_back(newIndex);
            region.vertices.push_back(v);
        }

        regions.push_back(std::move(region));
    }

    return regions;
}
