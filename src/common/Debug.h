#pragma once

#include <string>
#include <format>
#include <iostream>

#include <SDL3/SDL.h>

#include "Color.h"

class Debug
{
public:

	Debug(SDL_Renderer** renderer)
		: renderer(renderer) {
	}

	void drawText(int x, int y, const Color& color, std::string text) const;

	template<typename... Args>
	static void printf(std::format_string<Args...> format, Args&&... args)
	{
		std::string text = std::format(format, std::forward<Args>(args)...);
		std::cout << text << "\n";
	}

	static void print(const std::string& text)
	{
		std::cout << text << "\n";
	}

private:
	SDL_Renderer** renderer;
};