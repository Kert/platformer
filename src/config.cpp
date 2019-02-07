#include "config.h"
#include <SDL.h>
#include <deque>
#include <fstream>
#include <string>
#include "INIReader.h"
#include "globals.h"
#include "utils.h"

// Pretending joystick input codes never overlap keyboard's
std::deque<int> bindList;

extern int playerLives;
extern int fullscreenMode;

const std::map<std::string, int> configNames = {
	{"UP", 0},
	{"DOWN", 1},
	{"LEFT", 2},
	{"RIGHT", 3},
	{"JUMP", 4},
	{"FIRE", 5},
	{"SWITCH WEAPON", 6},
	{"OK", 7},
	{"BACK", 8}
};

std::map<std::string, int> fullscreenModes = {
	{"Off", 0},
	{"On", 1},
	{"Borderless", 2}
};

void InitConfig()
{
	LoadDefaultBindings();
	LoadConfig();

	if(playerLives < 1 || playerLives > MAX_LIVES) playerLives = 3;
}

void LoadDefaultBindings()
{
	// reserve memory for binds
	for(int i = 0; i < NUM_BINDS; i++)
		bindList.push_back(0);
	SetBinding(SDLK_UP, BIND_UP);
	SetBinding(SDLK_DOWN, BIND_DOWN);
	SetBinding(SDLK_LEFT, BIND_LEFT);
	SetBinding(SDLK_RIGHT, BIND_RIGHT);
	SetBinding(SDLK_z, BIND_JUMP);
	SetBinding(SDLK_x, BIND_FIRE);
	SetBinding(SDLK_c, BIND_SWITCH);
	SetBinding(SDLK_RETURN2, BIND_OK);
	SetBinding(SDLK_ESCAPE, BIND_BACK);
	SetBinding(SDLK_UP, BIND_ARROWUP);
	SetBinding(SDLK_DOWN, BIND_ARROWDOWN);
	SetBinding(SDLK_LEFT, BIND_ARROWL);
	SetBinding(SDLK_RIGHT, BIND_ARROWR);
	SetBinding(SDLK_ESCAPE, BIND_ESCAPE);
	SetBinding(SDLK_RETURN2, BIND_ENTER);
	/*
	// Joystick default binds
	SetBinding(0, BIND_UP);
	SetBinding(1, BIND_DOWN);
	SetBinding(2, BIND_LEFT);
	SetBinding(3, BIND_RIGHT);
	SetBinding(10, BIND_JUMP);
	SetBinding(12, BIND_FIRE);
	SetBinding(13, BIND_SWITCH);
	SetBinding(6, BIND_OK);
	SetBinding(5, BIND_BACK);
	*/
}

void LoadConfig()
{
	INIReader reader("config.ini");

	if(reader.ParseError() < 0)
	{
		PrintLog(LOG_IMPORTANT, "Can't load config.ini");
		return;
	}

	for(auto &i : configNames)
	{
		std::string keyName = reader.Get("Keys", i.first, "");
		if(keyName == "")
			PrintLog(LOG_IMPORTANT, "Wrong bind");
		else
		{
			int keycode = SDL_GetKeyFromName(keyName.c_str());
			SetBinding(keycode, i.second);
		}
	}
	std::string fullscreenStr = reader.Get("Game", "Fullscreen", "Off");
	fullscreenMode = fullscreenModes[fullscreenStr];
}

void SaveConfig()
{
	// This overwrites all previous content in the file with current bindings
	// OK for now but if we want to add non-binding stuff later it becomes much more complicated to do
	std::ofstream file("config.ini");
	if(!file.good())
	{
		PrintLog(LOG_IMPORTANT, "File saving failed wtf");
		return;
	}
	file << "[Keys]" << std::endl;
	for(int i = 0; i < NUM_CONFIGURABLE_BINDS; i++)
	{
		file << GetBindingName(i) << "=" << GetDeviceBindName(bindList[i]) << std::endl;
	}
	file << "[Game]" << std::endl;
	file << "Lives=" << playerLives << std::endl;
	file << "Fullscreen=" << GetFullscreenMode(fullscreenMode) << std::endl;
}

void SetBinding(int code, int bind)
{
	bindList[bind] = code;
}

int GetBindingFromCode(int code)
{
	int bind = 0;
	bool found = false;
	for(auto i : bindList)
	{
		if(i == code)
		{
			found = true;
			break;
		}
		bind++;
	}
	return found ? bind : -1;
}

int GetBindingCode(int bind)
{
	return bindList.at(bind);
}

const char *GetBindingName(int bind)
{
	switch(bind)
	{
		case 0:
			return "UP";
		case 1:
			return "DOWN";
		case 2:
			return "LEFT";
		case 3:
			return "RIGHT";
		case 4:
			return "JUMP";
		case 5:
			return "FIRE";
		case 6:
			return "SWITCH WEAPON";
		case 7:
			return "OK";
		case 8:
			return "BACK";
		default:
			return "";
			break;
	}
}

const char *GetFullscreenMode(int code)
{
	switch(code)
	{
	case 0:
		return "Off";
	case 1:
		return "On";
	case 2:
		return "Borderless";
	default:
		return "";
	}
}

std::string GetDeviceBindName(int code)
{
	std::string str;
	if(code < 15)
		str = "Joy " + std::to_string(code);
	else
		str.append(SDL_GetKeyName(code));
	return str;
}

void BindingsCleanup()
{
	std::deque<int>().swap(bindList); // forcibly deallocate memory
}