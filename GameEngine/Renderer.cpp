#include "Renderer.h"
#include "Log.h"
#include "Texture.h"
#include "Maths.h"
#include "SpriteComponent.h"
#include "CircleComponent.h"

#include <SDL_image.h>

Renderer::Renderer() : SDLRenderer(nullptr)
{
}

Renderer::~Renderer()
{
}

bool Renderer::initialize(Window& window)
{
	SDLRenderer = SDL_CreateRenderer(window.getSDLWindow(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//SDL_SetWindowFullscreen(window.getSDLWindow(), SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!SDLRenderer)
	{
		Log::error(LogCategory::Video, "Failed to create renderer");
		return false;
	}
	if (IMG_Init(IMG_INIT_PNG) == 0)
	{
		Log::error(LogCategory::Video, "Unable to initialize SDL_image");
		return false;
	}
	return true;
}

void Renderer::beginDraw()
{
	SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 0, 255);
	SDL_RenderClear(SDLRenderer);
}

void Renderer::draw()
{
	drawSprites();
	drawCircles();
}

void Renderer::endDraw()
{
	SDL_RenderPresent(SDLRenderer);
}

void Renderer::drawRect(const Rectangle& rect) const
{
	SDL_SetRenderDrawColor(SDLRenderer, 255, 255, 255, 255);
	SDL_Rect SDLRect = rect.toSDLRect();
	SDL_RenderFillRect(SDLRenderer, &SDLRect);
}

void Renderer::drawCircle(Vector2 positionP, float radiusP) const
{
	SDL_SetRenderDrawColor(SDLRenderer, 255, 255, 255, 255);

	float offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radiusP;
	d = radiusP - 1;
	status = 0;

	while (offsety >= offsetx) {

		status += SDL_RenderDrawLine(SDLRenderer, positionP.x - offsety, positionP.y + offsetx, positionP.x + offsety, positionP.y + offsetx);
		status += SDL_RenderDrawLine(SDLRenderer, positionP.x - offsetx, positionP.y + offsety, positionP.x + offsetx, positionP.y + offsety);
		status += SDL_RenderDrawLine(SDLRenderer, positionP.x - offsetx, positionP.y - offsety, positionP.x + offsetx, positionP.y - offsety);
		status += SDL_RenderDrawLine(SDLRenderer, positionP.x - offsety, positionP.y - offsetx, positionP.x + offsety, positionP.y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2 * offsetx) {
			d -= 2 * offsetx + 1;
			offsetx += 1;
		}
		else if (d < 2 * (radiusP - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}
}

void Renderer::drawSprites()
{
	for (auto sprite : sprites)
	{
		sprite->draw(*this);
	}
}

void Renderer::drawCircles()
{
	for (auto circle : circles)
	{
		circle->draw(*this);
	}
}

void Renderer::drawSprite(const Actor& actor, const Texture& tex, Rectangle srcRect, Vector2 origin, Flip flip) const
{
	SDL_Rect dstRect;
	Vector2 position = actor.getPosition();
	float rotation = actor.getRotation();
	float scale = actor.getScale();
	// Scale the width/height by owner's scale
	dstRect.w = static_cast<int>(tex.getWidth() * scale);
	dstRect.h = static_cast<int>(tex.getHeight() * scale);
	// Center the rectangle around the position of the owner
	dstRect.x = static_cast<int>(position.x - origin.x);
	dstRect.y = static_cast<int>(position.y - origin.y);

	SDL_Rect* srcSDL = nullptr;
	if (srcRect != Rectangle::nullRect)
	{
		srcSDL = new SDL_Rect{
			Maths::round(srcRect.x),
			Maths::round(srcRect.y),
			Maths::round(srcRect.width),
			Maths::round(srcRect.height)
		};
	}

	SDL_RenderCopyEx(
		SDLRenderer,
		tex.toSDLTexture(),
		srcSDL,
		&dstRect,
		-Maths::toDegrees(rotation),
		nullptr,		// rotation point, center by default
		SDL_FLIP_NONE);

	delete srcSDL;
}

void Renderer::close()
{
	SDL_DestroyRenderer(SDLRenderer);
}

void Renderer::addSprite(SpriteComponent* sprite)
{
	// Insert the sprite at the right place in function of drawOrder
	int spriteDrawOrder = sprite->getDrawOrder();
	auto iter = begin(sprites);
	for (; iter != end(sprites); ++iter)
	{
		if (spriteDrawOrder < (*iter)->getDrawOrder()) break;
	}
	sprites.insert(iter, sprite);
}

void Renderer::removeSprite(SpriteComponent* sprite)
{
	auto iter = std::find(begin(sprites), end(sprites), sprite);
	sprites.erase(iter);
}

void Renderer::addCircle(CircleComponent* circle)
{
	// Insert the sprite at the right place in function of drawOrder
	int circleDrawOrder = circle->getDrawOrder();
	auto iter = begin(circles);
	for (; iter != end(circles); ++iter)
	{
		if (circleDrawOrder < (*iter)->getDrawOrder()) break;
	}
	circles.insert(iter, circle);
}

void Renderer::removeCircle(CircleComponent* circle)
{
	auto iter = std::find(begin(circles), end(circles), circle);
	circles.erase(iter);
}
