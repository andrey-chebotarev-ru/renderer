#include "Debug.h"

void Debug::drawText(int x, int y, const Color& color, std::string text) const
{
	if (*renderer != nullptr) {
		SDL_SetRenderDrawColor(*renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
		SDL_RenderDebugText(*renderer, x, y, text.c_str());
	}
}