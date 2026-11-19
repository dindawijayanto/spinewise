#define TINYOBJLOADER_IMPLEMENTATION
#include "../../external/tinyobjloader/tiny_obj_loader.h"
#include "ObjLoader.h"
#include <iostream>

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

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    for (const auto& shape : shapes) {
        MeshRegion region;
        region.name = shape.name;

        for (const auto& index : shape.mesh.indices) {
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

            region.indices.push_back(static_cast<unsigned int>(region.vertices.size()));
            region.vertices.push_back(v);
        }

        regions.push_back(std::move(region));
    }

    return regions;
}