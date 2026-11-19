#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "common/Common.h"
#include "common/Color.h"
#include "common/Texture.h"
#include "common/Debug.h"

class WadLoader
{
public:
	explicit WadLoader(TextureManager& textureManager);

	// Индекс в textureNames соответствует texIdx поверхности
	// Возвращает таблицу local texIdx -> textureManager idx
	// idx 0 означает плейсхолдер
	IndicesList load(StringList textureNames, const std::filesystem::path& mapWad, const std::filesystem::path& commonWadDirectory);

private:

#pragma pack(push, 1)

	struct WadHeader
	{
		char magic[4];
		uint32_t lumpCount;
		uint32_t directoryOffset;
	};

	struct WadLump
	{
		uint32_t offset;
		uint32_t diskSize;
		uint32_t size;
		uint8_t type;
		uint8_t compression;
		uint8_t padding[2];
		char name[16];
	};

	struct MipTexHeader
	{
		char name[16];
		uint32_t width;
		uint32_t height;
		uint32_t offsets[4];
	};

#pragma pack(pop)

	static_assert(sizeof(WadHeader) == 12);
	static_assert(sizeof(WadLump) == 32);
	static_assert(sizeof(MipTexHeader) == 40);
	static_assert(sizeof(Color) == 4);

	static std::string normalizeName(std::string_view name);

	uint32_t loadTexture(std::ifstream& file, const WadLump& lump, uint64_t fileSize);
	void loadWad(const std::filesystem::path& path, const StringList& names, IndicesList& remap, size_t& remaining);

	TextureManager& textureManager;
};

