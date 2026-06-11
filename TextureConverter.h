#pragma once

#include <iostream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <string>
#include <stdexcept>
#include <format>
#include <array>

#include "Texture.h"
#include "Color.h"
#include "Debug.h"

constexpr int32_t BSPVERSION = 29;
constexpr size_t LUMP_TEXTURES = 2;

struct BSPLump
{
	int32_t offset;
	int32_t size;
};

struct BSPHeader
{
	int32_t version; // 29
	BSPLump lumps[15];
};

struct MipTexLump
{
	int32_t numTextures;
	int32_t* textureOffsets;
};

struct MipTex
{
	char name[16];

	uint32_t width;
	uint32_t height;

	uint32_t mip0Offset;
	uint32_t mip1Offset;
	uint32_t mip2Offset;
	uint32_t mip3Offset;
};

struct RGB
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};



class TextureConverter
{
public:

	~TextureConverter();

	bool load(const std::string& path, const std::string& palettePath);

	const TextureList& getTextures() {
		return textures;
	}

	const TextureTable& getTable() {
		return table;
	}

private:

	std::array<RGB, 256> palette;
	TextureTable table;
	TextureList textures;
	std::vector<Color*> pointers;

	void readPalette(const std::string& palettePath);

	template<typename T>
	T read(std::ifstream& file)
	{
		T value;
		file.read(reinterpret_cast<char*>(&value), sizeof(T));
		return value;
	}
};