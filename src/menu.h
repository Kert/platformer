#ifndef _menu_h_
#define _menu_h_

#include <SDL.h>
#include <SDL_ttf.h>
#include <map>
#include <vector>
#include "config.h"
#include "globals.h"

class Menu;

void DoMenuAction(int kbkey, int jbutton, int bind);
void NavigateMenu(int bind);
void LoadMenus();
void SetCurrentMenu(MENUS menu);
MENUS GetCurrentMenu();
void MenusCleanup();
KEYBINDS GetCurrentKeyToBind();
std::map<MENUS, Menu*>* GetMenus();

class MenuItem;

class Menu
{
	private:
		std::vector<MenuItem*> items;
	public:
		int selected = 0;
		bool IsSwitchable = false;
	public:
		Menu();
		~Menu();
		void AddMenuItem(MenuItem *item);
		int GetItemCount();
		MenuItem* GetItemInfo(int number);
};

class MenuItem
{
	public:
		SDL_Point pos;
		std::string text;
		FONTS font;
		SDL_Color standardColor;
		SDL_Color selectedColor;
		TEXT_ALIGN align;

	public:
		MenuItem(int x, int y, std::string text, FONTS font, SDL_Color standardColor, SDL_Color selectedColor, TEXT_ALIGN align = TEXT_ALIGN_CENTER);
		MenuItem(SDL_Point pos, std::string text, FONTS font, SDL_Color standardColor, SDL_Color selectedColor);
		void SetText(std::string text);
		~MenuItem();
};

#endif
