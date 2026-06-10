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

	TextureConverter(TextureManager& manager)
		: manager(manager) {
	}

	std::array<uint8_t, 64 * 256> colormap;
	std::array<RGB, 256> palette;

	TextureManager& manager;

	//std::ifstream file;

	void load(const std::string& path)
	{
		readColormap();
		readPalette();

		std::ifstream file(path, std::ios::binary);

		if (!file) {
			throw std::runtime_error(std::format("Error opening '{}'.", path));
		}
		
		BSPHeader header = read<BSPHeader>(file);

		std::cout << std::format("Version: {}\n", header.version);

		size_t lumpOffset = header.lumps[LUMP_TEXTURES].offset;

		file.seekg(lumpOffset, std::ios::beg);

		int32_t numTextures = read<int32_t>(file);

		std::cout << std::format("Texture count: {}\n", numTextures);

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

			std::cout << std::format("Texture {}x{}: '{}'\n", texHeader.width, texHeader.height, texHeader.name);

			int32_t pixelsPos = lumpOffset + offset + texHeader.mip0Offset;

			uint8_t* pixels = (uint8_t*)std::malloc(texHeader.width * texHeader.height);

			file.seekg(pixelsPos, std::ios::beg);
			file.read(reinterpret_cast<char*>(pixels), texHeader.width * texHeader.height);

			Color* dst = (Color*)std::malloc(texHeader.width * texHeader.height * sizeof(Color));
			std::memset(dst, 255, texHeader.width * texHeader.height * sizeof(Color));

			for (int j = 0; j < texHeader.width * texHeader.height; j++) {
				uint8_t colorIdx = pixels[j];
				uint8_t colormapIdx = colormap[256 * 0 + colorIdx];
				
				RGB val = palette[colorIdx];

				dst[j].r = val.r;
				dst[j].g = val.g;
				dst[j].b = val.b;
				dst[j].a = 255;

			}

			manager.add(texHeader.name, texHeader.width, texHeader.height, texHeader.width * sizeof(Color), (uint8_t*)dst);

			std::free(pixels);
			std::free(dst);
		}

	}

	void readColormap()
	{
		std::ifstream file("colormap.lmp", std::ios::binary);

		if (!file) {
			throw std::runtime_error("Error opening 'colormap.lmp'.");
		}

		file.read(reinterpret_cast<char*>(colormap.data()), colormap.size());
	}

	void readPalette()
	{
		std::ifstream file("palette.lmp", std::ios::binary);

		if (!file) {
			throw std::runtime_error("Error opening 'palette.lmp'.");
		}

		for (int i = 0; i < 256; i++) {
			file.read(reinterpret_cast<char*>(&palette[i].r), 1);
			file.read(reinterpret_cast<char*>(&palette[i].g), 1);
			file.read(reinterpret_cast<char*>(&palette[i].b), 1);
		}
	}

	template<typename T>
	T read(std::ifstream& file)
	{
		T value;
		file.read(reinterpret_cast<char*>(&value), sizeof(T));
		return value;
	}



};

