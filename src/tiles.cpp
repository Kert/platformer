#include "tiles.h"
#include <SDL.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "graphics.h"
#include "level.h"
#include "utils.h"

extern std::vector<std::vector<int>> tiles;

extern Level *level;

std::vector<std::vector<std::vector<Tile*>>> tileLayers;

std::vector<CustomTile> tileset;

void LoadTileSet()
{
	int width, height;
	width = Graphics::GetLevelTextureSurface()->w / TILESIZE;
	height = Graphics::GetLevelTextureSurface()->h / TILESIZE;
	for(int i = 0; i < width * height; i++)
	{
		// Calculating tile ID
		int rem = i % width; // remainder
		int row = i / width;
		AddDataToTileSet(PHYSICS_AIR, rem * TILESIZE, row * TILESIZE);
	}
}

void AddDataToTileSet(int type, int x_offset, int y_offset)
{
	CustomTile c = { type, x_offset, y_offset };
	tileset.push_back(c);
}

void DeleteAllTiles()
{
	for(auto &layer : tileLayers)
	{
		for(auto &i : layer)
		{
			for(auto &j : i)
			{
				if(j)
				{
					delete j;
					j = nullptr;
				}
			}
		}
	}

	tileLayers.clear();
}

void LoadTileTypesFromFile(std::string filename)
{
	std::ifstream infile(filename);
	std::string line;

	int i = 0;
	while(std::getline(infile, line))
	{
		std::stringstream iss(line);
		size_t pos = 0;
		std::string delimiter = "\t";
		pos = line.find(delimiter);
		while(pos != std::string::npos)
		{
			pos = line.find('\t');
			std::string token;
			token = line.substr(0, pos);
			std::stringstream str(token);
			int val;
			str >> val;
			tileset[i].type = val;
			i++;
			if(pos == std::string::npos)
				line.erase(0, line.size());
			else
				line.erase(0, pos + delimiter.size());
		}
	}
}

Tile::Tile(int x, int y, int layer, CustomTile *data, bool replace)
{
	if(x >= (int)tileLayers[layer].size() || y >= (int)tileLayers[layer][0].size())
	{
		PrintLog(LOG_IMPORTANT, "Attempted to place a tile outside of level boundaries: %d %d", x, y);
		delete this;
		return;
	}

	this->x = x;
	this->y = y;
	this->layer = layer;
	this->tex_x = data->x_offset;
	this->tex_y = data->y_offset;
	src_tex = Graphics::GetLevelTextureSurface();
	animation = &data->animationData;
	customTile = data;
	this->type = data->type;

	if(tiles[x][y] != PHYSICS_UNOCCUPIED)
	{
		if(!replace)
		{
			delete this;
			return;
		}
		else
		{
			delete tileLayers[layer][x][y];
			tiles[x][y] = PHYSICS_UNOCCUPIED;
		}
	}
	tiles[x][y] = type;
	tileLayers[layer][x][y] = this;
}

Tile::Tile(int x, int y, int layer, CustomTile *data, char type, bool replace)
{
	if(x >= (int)tileLayers[layer].size() || y >= (int)tileLayers[layer][0].size())
	{
		PrintLog(LOG_IMPORTANT, "Attempted to place a tile outside of level boundaries: %d %d", x, y);
		delete this;
		return;
	}

	this->x = x;
	this->y = y;
	this->layer = layer;
	this->tex_x = data->x_offset;
	this->tex_y = data->y_offset;	
	src_tex = Graphics::GetLevelTextureSurface();
	animation = &data->animationData;
	customTile = data;
	this->type = type;

	if(tiles[x][y] != PHYSICS_UNOCCUPIED)
	{
		if(!replace)
		{
			delete this;
			return;
		}
		else
		{
			delete tileLayers[layer][x][y];
			tiles[x][y] = PHYSICS_UNOCCUPIED;
		}
	}

	tiles[x][y] = type;
	tileLayers[layer][x][y] = this;
}


int Tile::GetID()
{
	return (this->tex_y / TILESIZE) * 11 + this->tex_x / TILESIZE;
}

bool Tile::HasAnimation()
{
	return !!(int)this->animation->sequence.size();
}

void TilesCleanup()
{
	tileLayers.~vector();
	tileset.~vector();
}

Tile::~Tile()
{
	tileLayers[layer][x][y] = nullptr;
	// Setting tile type of a tile below current one
	bool foundTile = false;
	for(int i = layer-1; i >= 0; i--)
	{
		if(tileLayers[i][x][y] != nullptr)
		{
			tiles[x][y] = tileLayers[i][x][y]->type;
			foundTile = true;
			break;
		}
	}
	if(!foundTile)
		tiles[x][y] = PHYSICS_AIR;
}

PHYSICS_TYPES GetTileTypeAtTiledPos(int x, int y)
{
	return GetTileTypeAtTiledPos({ x, y });
}

PHYSICS_TYPES GetTileTypeAtTiledPos(SDL_Point at)
{
	if(at.x < 0 || at.x >= level->width_in_tiles || at.y < 0 || at.y >= level->height_in_tiles)
		return PHYSICS_TYPES::PHYSICS_OB;
	return static_cast<PHYSICS_TYPES>(tiles[at.x][at.y]);
}

PHYSICS_TYPES GetTileTypeAtPos(int x, int y)
{
	return GetTileTypeAtTiledPos({ x / TILESIZE, y / TILESIZE });
}

PHYSICS_TYPES GetTileTypeAtPos(SDL_Point at)
{
	return GetTileTypeAtPos(at.x, at.y);
}