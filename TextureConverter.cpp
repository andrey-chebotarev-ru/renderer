#include "TextureConverter.h"

#include "Debug.h"

TextureConverter::~TextureConverter()
{
	for (auto pointer : pointers) {
		std::free(pointer);
	}
}

bool TextureConverter::load(const std::string& path, const std::string& palettePath)
{
	readPalette(palettePath);

	std::ifstream file(path, std::ios::binary);

	if (!file) {
		return false;
	}

	BSPHeader header = read<BSPHeader>(file);

	if (header.version != 29) {
		return false;
	}

	size_t lumpOffset = header.lumps[LUMP_TEXTURES].offset;

	file.seekg(lumpOffset, std::ios::beg);

	int32_t numTextures = read<int32_t>(file);

	std::cout << std::format("Textures in '{}': {}\n", path, numTextures);

	int32_t* offsets = (int32_t*)std::malloc(numTextures * sizeof(int32_t));

	for (int i = 0; i < numTextures; i++) {
		offsets[i] = read<int32_t>(file);
	}

	for (int i = 0; i < numTextures; i++) {
		int32_t offset = offsets[i];

		if (offset == -1) {
			continue;
		}

		int32_t miptexPos = lumpOffset + offset;
		file.seekg(miptexPos, std::ios::beg);

		MipTex texHeader = read<MipTex>(file);

		

		int32_t pixelsPos = lumpOffset + offset + texHeader.mip0Offset;

		uint8_t* pixels = (uint8_t*)std::malloc(texHeader.width * texHeader.height);

		Debug::printf("\tReading texture '{}' ({}x{})", texHeader.name, texHeader.width, texHeader.height);

		file.seekg(pixelsPos, std::ios::beg);
		file.read(reinterpret_cast<char*>(pixels), texHeader.width * texHeader.height);

		Color* dst = static_cast<Color*>(std::malloc(texHeader.width * texHeader.height * sizeof(Color)));
		std::memset(dst, 255, texHeader.width * texHeader.height * sizeof(Color));

		pointers.push_back(dst);

		for (int j = 0; j < texHeader.width * texHeader.height; j++) {

			RGB val = palette[pixels[j]];

			dst[j].r = val.r;
			dst[j].g = val.g;
			dst[j].b = val.b;
			dst[j].a = 255;
		}


		Texture tex = Texture(texHeader.width, texHeader.height, dst);

		size_t idx = textures.size() + 1;
		textures.push_back(tex);

		table.set(texHeader.name, idx);

		std::free(pixels);
	}

	return true;
}

void TextureConverter::readPalette(const std::string& palettePath)
{
	std::ifstream file(palettePath, std::ios::binary);

	if (!file) {
		throw std::runtime_error("Error opening 'palette.lmp'.");
	}

	for (int i = 0; i < 256; i++) {
		file.read(reinterpret_cast<char*>(&palette[i].r), 1);
		file.read(reinterpret_cast<char*>(&palette[i].g), 1);
		file.read(reinterpret_cast<char*>(&palette[i].b), 1);
	}
}
