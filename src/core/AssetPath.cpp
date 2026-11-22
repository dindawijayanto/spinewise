#include "core/AssetPath.h"

#include <filesystem>
#include <stdexcept>

std::string resolveAssetPath(const char* relativePath)
{
    namespace fs = std::filesystem;

    const fs::path direct(relativePath);
    if (fs::exists(direct))
    {
        return direct.string();
    }

    const fs::path fromBuildDir = fs::path("..") / relativePath;
    if (fs::exists(fromBuildDir))
    {
        return fromBuildDir.string();
    }

    throw std::runtime_error(std::string("File tidak ditemukan: ") + relativePath);
}
