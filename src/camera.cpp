#include "camera.h"
#include "gamelogic.h"
#include "graphics.h"
#include "level.h"
#include "utils.h"

const int VIRTUAL_CAM_WIDTH = 16 * TILESIZE;
const int VIRTUAL_CAM_HEIGHT = 15 * TILESIZE;

Camera::Camera()
{
	x = 0;
	y = 0;
	h = 0;
	w = 0;
	at = nullptr;
	offsetX = 0;
	offsetY = 0;
	virtualCam.x = virtualCam.y = 0;
	virtualCam.w = VIRTUAL_CAM_WIDTH;
	virtualCam.h = VIRTUAL_CAM_HEIGHT;
}

Camera::~Camera()
{

}

Camera::Camera(double x, double y, double w, double h)
{
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	at = nullptr;
	offsetX = 0;
	offsetY = 10;
	virtualCam.x = virtualCam.y = 0;
	virtualCam.w = VIRTUAL_CAM_WIDTH;
	virtualCam.h = VIRTUAL_CAM_HEIGHT;
}

SDL_Rect Camera::GetRect()
{
	SDL_Rect r;
	r.h = (int)(h);
	r.w = (int)(w);
	r.x = (int)(x);
	r.y = (int)(y);
	return r;
}

PrecisionRect Camera::GetPRect()
{
	return{ x, y, h, w };
}

void Camera::SetRect(SDL_Rect &r)
{
	h = r.h;
	w = r.w;
	x = r.x;
	y = r.y;
}

void Camera::Attach(Entity &p)
{
	at = &p;
	SetOffsetX(0);
	SetOffsetY(0);
	x = at->hitbox->GetPRect().x - Graphics::GetGameSceneWidth() / 2;
	y = at->hitbox->GetPRect().y - Graphics::GetGameSceneHeight() / 2;

	// Fit into bounds
	SDL_Rect entityRect = p.hitbox->GetRect();
	for(auto bound : Game::GetLevel()->CameraBounds)
	{
		if(SDL_HasIntersection(&entityRect, &bound))
		{
			virtualCam.x = bound.x;
			virtualCam.y = bound.y;
			x = virtualCam.x + VIRTUAL_CAM_WIDTH / 2 - Graphics::GetGameSceneWidth() / 2;
			y = virtualCam.y + VIRTUAL_CAM_HEIGHT / 2 - Graphics::GetGameSceneHeight() / 2;
			break;
		}
	}
}

void Camera::Detach()
{
	at = nullptr;
}

void Camera::SetOffsetX(int x)
{
	//offsetX = x;
}

void Camera::SetOffsetY(int y)
{
	//offsetY = y;
}

void Camera::Update()
{
	if(at == nullptr)
		return;

	double playerX = at->hitbox->GetRect().x;
	double playerY = at->hitbox->GetRect().y + at->hitbox->GetRect().h;

	SDL_Rect newRectX, newRectY;
	newRectX.x = playerX - VIRTUAL_CAM_WIDTH / 2;
	newRectX.y = virtualCam.y;
	newRectX.w = virtualCam.w;
	newRectX.h = virtualCam.h;

	newRectY.x = virtualCam.x;
	newRectY.y = playerY - VIRTUAL_CAM_HEIGHT / 2;
	newRectY.w = virtualCam.w;
	newRectY.h = virtualCam.h;

	int deltaX = newRectX.x - virtualCam.x;
	int deltaY = newRectY.y - virtualCam.y;
	bool xFine, yFine;
	xFine = yFine = false;

	Level *level = Game::GetLevel();
	for(auto bound : level->CameraBounds)
	{
		if(!xFine)
		{
			for(int i = deltaX; i != 0; deltaX > 0 ? i-- : i++)
			{
				newRectX.x = virtualCam.x + i;
				SDL_Rect intersectX;
				SDL_IntersectRect(&bound, &newRectX, &intersectX);
				if(SDL_RectEquals(&intersectX, &newRectX))
				{
					xFine = true;
					break;
				}
			}				
		}

		if(!yFine)
		{
			for(int i = deltaY; i != 0; deltaY > 0 ? i-- : i++)
			{
				newRectY.y = virtualCam.y + i;
				SDL_Rect intersectY;
				SDL_IntersectRect(&bound, &newRectY, &intersectY);
				if(SDL_RectEquals(&intersectY, &newRectY))
				{
					yFine = true;
					break;
				}
			}
				
		}
			
		if(xFine && yFine)
			break;
	}

	if(xFine)
	{
		virtualCam.x = newRectX.x;
		x = virtualCam.x + VIRTUAL_CAM_WIDTH / 2 - Graphics::GetGameSceneWidth() / 2;
		PrintLog(LOG_SUPERDEBUG, "Camera X set to %lf", x);
	}
	if(yFine)
	{
		virtualCam.y = newRectY.y;
		if(Graphics::GetScalingMode() == SCALING_LETTERBOXED)
		{
			// center cam without considering anything else
			y = virtualCam.y + VIRTUAL_CAM_HEIGHT / 2 - Graphics::GetGameSceneHeight() / 2;
		}
		else
		{
			// center camera but avoid cropped tiles at the bottom of the screen
			int diff = abs(virtualCam.h - this->h);
			int mod = diff % TILESIZE;
			int rem = ceil((diff / (double)TILESIZE) / 2);
			y = virtualCam.y + rem * TILESIZE + mod;
		}
		PrintLog(LOG_SUPERDEBUG, "Camera Y set to %lf", y);
	}
		
	if(Graphics::GetScalingMode() == SCALING_LETTERBOXED)
		return;

	//Keep the camera in bounds.
	if(x < 0)
	{
		x = 0;
	}
	if(y < 0)
	{
		y = 0;
	}
	if(x > level->width_in_pix - w)
	{
		x = level->width_in_pix - w;
	}
	if(y > level->height_in_pix - h)
	{
		y = level->height_in_pix - h;
	}
}

bool Camera::IsAttachedTo(Entity *e)
{
	return e == this->at;
}

SDL_Rect Camera::GetVirtualCamRect()
{
	return virtualCam;
}