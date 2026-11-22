#pragma once

#include <string>

// Cari file asset/shader relatif terhadap working directory.
// Dicoba "<path>" lalu "../<path>", supaya jalan baik dari folder build/ maupun root repo.
// Melempar std::runtime_error kalau file tidak ditemukan.
std::string resolveAssetPath(const char* relativePath);
