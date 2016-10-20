/*
This file is part of Chathran Strata: https://github.com/takenu/strata
Copyright 2015, Matthijs van Dorp.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <tiny/draw/textbox.h>
#include <tiny/img/io/image.h>

#include <config.h>

#include <selene.h>

#include "../tools/convertstring.hpp"
#include "../tools/texture.hpp"

#include "../ui/console.hpp"
#include "../ui/mainmenu.hpp"
#include "../ui/monitor.hpp"

#include "ui.hpp"

using namespace strata::core;

/** Interpret key press events. */
void UIManager::keyEvent(const SDL_Keycode & keyIndex, bool isDown)
{
	inputInterpreter.receiveInput(keyIndex, (isDown ? SDL_GetModState() : KMOD_NONE), isDown);
}

/** Interpret mouse events. */
void UIManager::mouseEvent(float x, float y, unsigned int buttons)
{
	inputInterpreter.receiveInput(x, y, buttons);
}

/** Call functions of non-UI classes through UI functionality. */
void UIManager::callExternalFunction(std::string receiver, std::string args)
{
	if(receivers.count(receiver) > 0)
		receivers[receiver]->receiveUIFunctionCall(args);
	else if(windows.count(receiver) > 0)
		windows[receiver]->receiveUIFunctionCall(args);
	else std::cout << " UIManager::callExternalFunction() : No receiver '"<<receiver<<"'!"<<std::endl;
}

void UIManager::registerLuaFunctions(sel::State & luaState)
{
	luaState["ui"].SetObj(*this,
			"loadFont", &UIManager::loadFont,
			"loadFlatTexture", &UIManager::loadFlatTexture,
			"loadButton", &UIManager::loadButton,
			"loadAttribute", &UIManager::loadAttribute,
			"loadWindowFunction", &UIManager::loadWindowFunction,
			"loadWindowAttribute", &UIManager::loadWindowAttribute,
			"loadButtonAttribute", &UIManager::loadButtonAttribute,
			"loadWindowFontColour", &UIManager::loadWindowFontColour,
			"loadWindowDimensions", &UIManager::loadWindowDimensions,
			"loadConsoleWindow", &UIManager::loadConsoleWindow,
			"loadMonitorWindow", &UIManager::loadMonitorWindow,
			"loadMainMenuWindow", &UIManager::loadMainMenuWindow
			);
}

void UIManager::initializeWindow(ui::Window * window, std::string id)
{
	window->setAttribute("title",id); // Initialise 'title' as 'id' - Windows may or may not render this
	window->setCloseKey(closeKey);
	windows.emplace(id, window);
	loadWindowAttribute(id, "fontsize", tool::convertToString(defaultFontSize));
	loadWindowAttribute(id, "fontaspectratio", tool::convertToString(defaultAspectRatio));
	renderInterface->addScreenRenderable(window->getRenderable(), false, false, tiny::draw::BlendMix);
}

/** Load a Console window from Lua. Unlike other windows, the UIManager tracks the Console window
  * specifically, since it will provide a service to other parts of the program to print messages.
  * Note that the Console window cannot be loaded on UIManager creation, since it depends on Lua
  * parameters and it also needs access to the Lua interface. */
void UIManager::loadConsoleWindow(std::string id)
{
	if(windows.count(id) > 0 || receivers.count(id) > 0)
	{
		std::cout << " UIManager::loadConsoleWindow() : ID not unique - skipped! "<<std::endl;
	}
	if(console)
	{
		std::cout << " UIManager::loadConsoleWindow() : Deleting existing console! "<<std::endl;
		delete console; console = 0;
	}
	console = new ui::Console(static_cast<intf::UIInterface*>(this), luaInterface, fontTexture);
	initializeWindow(console, id);
}

void UIManager::loadMainMenuWindow(std::string id)
{
	if(windows.count(id) > 0 || receivers.count(id) > 0)
	{
		std::cout << " UIManager::loadMainMenuWindow() : ID not unique - skipped! "<<std::endl;
	}
	ui::Window * mainMenu = new ui::MainMenu(static_cast<intf::UIInterface*>(this), applInterface,
			fontTexture);
	initializeWindow(mainMenu, id);
}

void UIManager::loadMonitorWindow(std::string id)
{
	if(windows.count(id) > 0 || receivers.count(id) > 0)
	{
		std::cout << " UIManager::loadMainMenuWindow() : ID not unique - skipped! "<<std::endl;
	}
	ui::Window * monitor = new ui::Monitor(static_cast<intf::UIInterface*>(this), applInterface,
			fontTexture);
	initializeWindow(monitor, id);
}

void UIManager::loadFlatTexture(std::string target, std::string type, unsigned int size, unsigned int red,
		unsigned int green, unsigned int blue, unsigned int alpha)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window) std::cout << " UIManager::loadFlatTexture() : Can't load texture for window "<<target<<"!"<<std::endl;
	else
	{
		ui::ScreenSquare * background = new ui::ScreenSquare( tools::createTestTextureAlpha(
					size, red, green, blue, alpha) );
		window->setBackground(type, background);
		renderInterface->addScreenRenderable(background, false, false, tiny::draw::BlendMix);
		// Re-add renderable to ensure that the rest of the window goes before the background.
		// NOTE: This often isn't necessary since reserve() tends to re-add the text screen renderable.
		// But it is in principle possible to first set the text and then add the background, and in that
		// situation it is necessary to re-add the text to get it in front of the background.
		renderInterface->freeScreenRenderable(window->getRenderable());
		renderInterface->addScreenRenderable(window->getRenderable(), false, false, tiny::draw::BlendMix);
	}
}

void UIManager::loadWindowFontColour(std::string target, std::string attribute,
		unsigned int red, unsigned int green, unsigned int blue)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window)
	{
		std::cout << " UIManager::loadWindowFontColour() : Can't load "<<attribute<<"=("
			<<red<<","<<green<<","<<blue<<") for window "<<target<<"!"<<std::endl;
	}
	else window->setFontColour(attribute, tiny::draw::Colour(red,green,blue));
}

void UIManager::loadWindowDimensions(std::string target, std::string attribute,
		float left, float top, float right, float bottom)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window)
	{
		std::cout << " UIManager::loadWindowDimensions() : Can't load "<<attribute<<"=("
			<<left<<","<<top<<","<<right<<","<<bottom<<") for window "<<target<<"!"<<std::endl;
	}
	else window->setDimensions(attribute, left, top, right, bottom);
}

void UIManager::loadWindowFunction(std::string target, std::string key, std::string function)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window)
	{
		std::cout << " UIManager::loadWindowFunction() : Can't map key '"<<key<<"' -> function '"<<function
			<<"' for non-existing window "<<target<<"!"<<std::endl;
	}
	else if(toSDLKey(key) == SDLK_UNKNOWN)
	{
		std::cout << " UIManager::loadWindowFunction() : Can't map key '"<<key<<"' -> function '"<<function
			<<"' for window "<<target<<" because key is unknown!"<<std::endl;
	}
	else window->setFunctionMapping(toSDLKey(key), function);
}

void UIManager::setCloseKey(const SDL_Keycode & k)
{
	closeKey = k;
}

void UIManager::loadAttribute(std::string attribute, std::string value)
{
	if(attribute == "closeKey") setCloseKey(toSDLKey(value));
	else std::cout << " UIManager::loadAttribute() : No mapping for attr = '"<<attribute<<"'!"<<std::endl;
}

void UIManager::loadWindowAttribute(std::string target, std::string attribute, std::string value)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window)
	{
		std::cout << " UIManager::loadWindowAttribute() : Can't load "<<attribute<<"="<<value
			<< " for window "<<target<<"!"<<std::endl;
	}
	else window->setAttribute(attribute, value);
}

void UIManager::loadButtonAttribute(std::string target, std::string button,
		std::string attribute, std::string value)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window)
	{
		std::cout << " UIManager::loadButtonAttribute() : Can't load "<<attribute<<"="<<value
			<< " for window "<<target<<" button "<<button<<"!"<<std::endl;
	}
	else window->setButtonAttribute(button, attribute, value);
}

void UIManager::loadButton(std::string target, std::string buttonId)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window)
	{
		std::cout << " UIManager::loadButton() : Can't load button "<<buttonId
			<< " for target "<<target<<"!"<<std::endl;
	}
	else
	{
		window->loadButton(buttonId);
		window->setButtonTextBox(buttonId, fontTexture);
		loadButtonAttribute(target, buttonId, "fontsize", tool::convertToString(defaultFontSize));
		loadButtonAttribute(target, buttonId, "fontaspectratio", tool::convertToString(defaultAspectRatio));
		tiny::draw::Renderable * buttonRenderable = window->getButtonRenderable(buttonId);
		if(buttonRenderable) renderInterface->addScreenRenderable(buttonRenderable);
		else std::cout << " UIManager::loadButton() : No renderable! "<<std::endl;
	}
}

void UIManager::loadFont(std::string fontTex, float fontSize, float fontAspectRatio,
		unsigned int fontPixels, unsigned int fontResolution)
{
	if(fontTex == "" || fontSize < 0.01f || fontSize > 1.0f || fontAspectRatio < 0.2f
			|| fontAspectRatio > 10.0f || fontPixels < 12 || fontPixels > 1024
			|| fontResolution < 128 || fontResolution > 16384)
	{
		std::cerr << " UIManager::loadFont() : ERROR: Lua call with invalid arguments: ";
		std::cerr << fontTex << ", " << fontSize << ", " << fontAspectRatio << ", ";
		std::cerr << fontPixels << ", " << fontResolution << std::endl;
	}
	fontTexture = new tiny::draw::IconTexture2D(fontResolution,fontResolution);
	fontTexture->packIcons(tiny::img::io::readFont(DATA_DIRECTORY + "font/" + fontTex,fontPixels));
	if(defaultFontSize < 0.02f)
	{
		// If no defaults yet, use the first loaded Font as default font.
		defaultFontSize = fontSize;
		defaultAspectRatio = fontAspectRatio;
	}
}

void UIManager::reserve(ui::Window * window)
{
	std::vector<tiny::draw::Renderable *> oldTextBoxes;
	std::vector<tiny::draw::Renderable *> newTextBoxes;
	window->reserveTextBoxes(oldTextBoxes, newTextBoxes);
	for(unsigned int i = 0; i < oldTextBoxes.size(); i++)
		renderInterface->freeScreenRenderable(oldTextBoxes[i]);
	for(unsigned int i = 0; i < newTextBoxes.size(); i++)
	{
		renderInterface->addScreenRenderable(newTextBoxes[i], false, false, tiny::draw::BlendMix);
	}
}

void UIManager::update(double)
{
	for(std::map<std::string, ui::Window*>::iterator it = windows.begin(); it != windows.end(); it++)
	{
		it->second->update();
		reserve(it->second);
		it->second->setTexts();
	}
}
