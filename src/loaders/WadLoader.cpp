#include "WadLoader.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <system_error>
#include <unordered_map>
#include <utility>

namespace fs = std::filesystem;

namespace {

	constexpr uint8_t MIPTEX_TYPE = 0x43;
	constexpr uint32_t MAX_TEXTURE_SIZE = 8192;

	bool fits(uint64_t offset, uint64_t size, uint64_t limit)
	{
		return (offset <= limit && size <= limit - offset);
	}

	bool readAt(std::ifstream& file, uint64_t offset, void* destination, size_t size)
	{
		if (offset > static_cast<uint64_t>(std::numeric_limits<std::streamoff>::max()) || size > static_cast<size_t>(std::numeric_limits<std::streamsize>::max())) {
			return false;
		}

		file.clear();
		file.seekg(static_cast<std::streamoff>(offset));

		if (!file) {
			return false;
		}

		return bool(file.read(static_cast<char*>(destination), static_cast<std::streamsize>(size)));
	}

}

WadLoader::WadLoader(TextureManager& textureManager)
	: textureManager(textureManager) {
}

std::string WadLoader::normalizeName(std::string_view name)
{
	std::string result(name);

	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) {
			return static_cast<char>(std::toupper(c));
		});

	return result;
}

uint32_t WadLoader::loadTexture(std::ifstream& file, const WadLump& lump, uint64_t fileSize)
{
	if (lump.diskSize < sizeof(MipTexHeader)) {
		return 0;
	}

	MipTexHeader header;

	if (!readAt(file, lump.offset, &header, sizeof(header))) {
		return 0;
	}

	const uint32_t width = header.width;
	const uint32_t height = header.height;

	if (width == 0 || height == 0 || width > MAX_TEXTURE_SIZE || height > MAX_TEXTURE_SIZE) {
		return 0;
	}

	if (width % 8 != 0 || height % 8 != 0) {
		return 0;
	}

	std::array<uint64_t, 4> mipSizes;

	for (uint32_t mip = 0; mip < 4; mip++) {
		const uint64_t w = width >> mip;
		const uint64_t h = height >> mip;

		mipSizes[mip] = w * h;

		if (header.offsets[mip] < sizeof(MipTexHeader)) {
			return 0;
		}

		if (!fits(header.offsets[mip], mipSizes[mip], lump.diskSize)) {
			return 0;
		}
	}

	// Палитра располагается сразу после mip[3]
	const uint64_t paletteOffset = uint64_t(header.offsets[3]) + mipSizes[3];

	constexpr uint64_t paletteBytes = 2 + 256 * 3;

	if (!fits(paletteOffset, paletteBytes, lump.diskSize)) {
		return 0;
	}

	// Количество цветов в палитре (little-endian)
	uint8_t countBytes[2];

	if (!readAt(file, uint64_t(lump.offset) + paletteOffset, countBytes, sizeof(countBytes))) {
		return 0;
	}

	uint16_t colorCount = uint16_t(countBytes[0]) | (uint16_t(countBytes[1]) << 8);

	if (colorCount != 256) {
		return 0;
	}

	std::array<uint8_t, 256 * 3> rawPalette;
	std::array<Color, 256> palette;

	// Читаем сырую палитру
	if (!readAt(file, uint64_t(lump.offset) + paletteOffset + 2, rawPalette.data(), sizeof(rawPalette))) {
		return 0;
	}

	for (size_t i = 0; i < 256; i++) {
		palette[i] = Color(
			rawPalette[i * 3 + 0],
			rawPalette[i * 3 + 1],
			rawPalette[i * 3 + 2]
		);
	}

	// В GoldSrc текстуры с прозрачностью начинаются с '{'
	const bool masked = (lump.name[0] == '{');

	// 4 массива пикселей RGB888
	std::array<std::vector<Color>, 4> mipPixels;

	for (uint32_t mip = 0; mip < 4; mip++) {
		const size_t pixelCount = static_cast<size_t>(mipSizes[mip]);

		std::vector<uint8_t> indices(pixelCount);

		const uint64_t dataOffset = uint64_t(lump.offset) + header.offsets[mip];

		if (!fits(dataOffset, pixelCount, fileSize)) {
			return 0;
		}

		if (!readAt(file, dataOffset, indices.data(), indices.size())) {
			return 0;
		}

		auto& pixels = mipPixels[mip];
		pixels.resize(pixelCount);

		// Маппинг индекс -> Color
		// Индекс прозрачных пикселей - 255
		for (size_t p = 0; p < pixelCount; p++) {
			const uint8_t idx = indices[p];
			pixels[p] = palette[idx];
			if (masked && idx == 255) {
				pixels[p].a = 0;
			}
		}
	}

	Color* pointer[4];

	pointer[0] = mipPixels[0].data();
	pointer[1] = mipPixels[1].data();
	pointer[2] = mipPixels[2].data();
	pointer[3] = mipPixels[3].data();

	return textureManager.add(lump.name, width, height, pointer, masked);
}

void WadLoader::loadWad(const std::filesystem::path& path, const StringList& names, IndicesList& remap, size_t& remaining)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);

	if (!file) {
		return;
	}

	const auto endPos = file.tellg();

	if (endPos < 0) {
		return;
	}

	const uint64_t fileSize = static_cast<uint64_t>(endPos);

	if (fileSize < sizeof(WadHeader)) {
		return;
	}

	WadHeader header;

	if (!readAt(file, 0, &header, sizeof(header))) {
		return;
	}

	if (std::memcmp(header.magic, "WAD3", 4) != 0) {
		return;
	}

	const uint64_t directorySize = uint64_t(header.lumpCount) * sizeof(WadLump);

	if (!fits(header.directoryOffset, directorySize, fileSize)) {
		return;
	}

	//  Список еще ненайденных текстур
	std::unordered_map<std::string, std::vector<size_t>> wanted;

	for (size_t i = 0; i < names.size(); i++) {
		if (remap[i] == 0) {
			wanted[normalizeName(names[i])].push_back(i);
		}
	}

	for (uint32_t i = 0; i < header.lumpCount; i++) {
		if (remaining == 0 || wanted.empty()) {
			break;
		}

		WadLump lump;

		const uint64_t offset = uint64_t(header.directoryOffset) + uint64_t(i) * sizeof(WadLump);

		if (!readAt(file, offset, &lump, sizeof(lump))) {
			break;
		}

		// Нужны только несжатые MIPTEX
		if (lump.type != MIPTEX_TYPE || lump.compression != 0) {
			continue;
		}

		// Имя 16 байт
		size_t length = 0;

		while (length < 16 && lump.name[length] != '\0') {
			length++;
		}

		const std::string name = normalizeName(std::string_view(lump.name, length));

		auto found = wanted.find(name);

		if (found == wanted.end()) {
			continue;
		}

		if (!fits(lump.offset, lump.diskSize, fileSize)) {
			continue;
		}

		// Загружаем пиксели только нужной текстуры
		uint32_t textureId = loadTexture(file, lump, fileSize);

		if (textureId == 0) {
			continue;
		}

		Debug::printf("Texture '{}' loaded from '{}'", lump.name, path.string());

		// Заполняем локальные индексы карты
		for (size_t index : found->second) {
			remap[index] = textureId;
			remaining--;
		}

		// Удаляем найденую текстуру из списка
		wanted.erase(found);
	}
}

IndicesList WadLoader::load(StringList textureNames, const std::filesystem::path& mapWad, const std::filesystem::path& commonWadDirectory)
{
	IndicesList remap(textureNames.size(), 0);

	if (textureNames.empty()) {
		return remap;
	}

	size_t remaining = textureNames.size();

	// 1. Загрузка WAD самой карты
	loadWad(mapWad, textureNames, remap, remaining);

	if (remaining == 0) {
		return remap;
	}

	// 2. Собираем список общих WAD-ов
	std::vector<fs::path> wadFiles;
	std::error_code errorCode;

	fs::directory_iterator it(commonWadDirectory, errorCode);
	fs::directory_iterator end;

	while (!errorCode && it != end) {
		const auto& entry = *it;

		if (entry.is_regular_file(errorCode)) {
			const auto extension = normalizeName(entry.path().extension().string());

			if (extension == ".WAD") {
				wadFiles.push_back(entry.path());
				Debug::printf("WAD found: '{}'", entry.path().string());
			}
		}

		if (errorCode) {
			break;
		}

		it.increment(errorCode);
	}

	// Порядок загрузки должен быть стабильным
	std::sort(wadFiles.begin(), wadFiles.end());

	for (const auto& path : wadFiles) {
		if (remaining == 0) {
			break;
		}

		// Не читаем WAD карты повторно
		std::error_code pathErrorCode;
		if (fs::equivalent(path, mapWad, pathErrorCode) && !pathErrorCode) {
			continue;
		}

		loadWad(path, textureNames, remap, remaining);
	}

	return remap;
}