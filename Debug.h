#pragma once

#include <string>
#include <SDL3/SDL.h>

class Debug
{
public:

	Debug(SDL_Renderer** renderer)
		: renderer(renderer) {
	}

	void drawText(int x, int y, std::string text) const;

private:
	SDL_Renderer** renderer;
};