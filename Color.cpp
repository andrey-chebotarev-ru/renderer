#include "Color.h"

#include <SDL3/SDL.h>

Color::Color()
{
	r = SDL_rand(256);
	g = SDL_rand(256);
	b = SDL_rand(256);
	a = 255;
}
