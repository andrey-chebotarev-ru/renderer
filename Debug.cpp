#include "Debug.h"

void Debug::drawText(int x, int y, std::string text) const
{
	if (*renderer != nullptr) {
		SDL_RenderDebugText(*renderer, x, y, text.c_str());
	}
}