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

#include "ui.hpp"

using namespace strata::core;

/** Interpret key press events. */
void UIManager::keyEvent(const SDLKey & keyIndex, bool isDown)
{
	inputInterpreter.receiveInput(keyIndex, (isDown ? SDL_GetModState() : KMOD_NONE), isDown);
}

void UIManager::registerLuaFunctions(sel::State & luaState)
{
	luaState["ui"].SetObj(*this,
			"loadFont", &UIManager::loadFont,
			"loadFlatTexture", &UIManager::loadFlatTexture,
			"loadWindowAttribute", &UIManager::loadWindowAttribute,
			"loadWindowFontColour", &UIManager::loadWindowFontColour,
			"loadWindowDimensions", &UIManager::loadWindowDimensions,
			"loadMonitorWindow", &UIManager::loadMonitorWindow,
			"loadMainMenuWindow", &UIManager::loadMainMenuWindow
			);
}

void UIManager::loadMainMenuWindow(std::string id)
{
	if(windows.count(id) > 0)
	{
		std::cout << " UIManager::loadMainMenuWindow() : ID not unique - skipped! "<<std::endl;
	}
	ui::Window * mainMenu = new ui::MainMenu(static_cast<intf::UIInterface*>(this), applInterface,
			fontTexture);
	windows.emplace(id, mainMenu);
	renderInterface->addScreenRenderable(mainMenu->getRenderable(), false, false, tiny::draw::BlendMix);
}

void UIManager::loadMonitorWindow(std::string id)
{
	if(windows.count(id) > 0)
	{
		std::cout << " UIManager::loadMainMenuWindow() : ID not unique - skipped! "<<std::endl;
	}
	ui::Window * monitor = new ui::Monitor(static_cast<intf::UIInterface*>(this), applInterface,
			fontTexture);
	windows.emplace(id, monitor);
	renderInterface->addScreenRenderable(monitor->getRenderable(), false, false, tiny::draw::BlendMix);
}

void UIManager::loadFlatTexture(std::string target, unsigned int size, unsigned int red,
		unsigned int green, unsigned int blue, unsigned int alpha)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window) std::cout << " loadFlatTexture() : Can't load texture for window "<<target<<"!"<<std::endl;
	else
	{
		ui::ScreenSquare * background = new ui::ScreenSquare( tools::createTestTextureAlpha(
					size, red, green, blue, alpha) );
		window->setBackground(background);
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
		std::cout << " loadMonitorWindowFontColour() : Can't load "<<attribute<<"=("
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
		std::cout << " loadMonitorWindowDimensions() : Can't load "<<attribute<<"=("
			<<left<<","<<top<<","<<right<<","<<bottom<<") for window "<<target<<"!"<<std::endl;
	}
	else window->setDimensions(attribute, left, top, right, bottom);
}

void UIManager::loadWindowAttribute(std::string target, std::string attribute, std::string value)
{
	ui::Window * window = (windows.count(target) > 0 ? windows[target] : 0);
	if(!window)
	{
		std::cout << " loadMonitorWindowAttribute() : Can't load "<<attribute<<"="<<value
			<< " for window "<<target<<"!"<<std::endl;
	}
	else window->setAttribute(attribute, value);
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
	tiny::draw::Renderable * oldTextBox = 0;
	tiny::draw::Renderable * newTextBox = window->reserve(oldTextBox);
	if(oldTextBox) renderInterface->freeScreenRenderable(oldTextBox);
	if(newTextBox)
	{
		renderInterface->addScreenRenderable(newTextBox, false, false, tiny::draw::BlendMix);
	}
	else if(oldTextBox) std::cout << " UIManager::reserve() : ERROR: No new renderable! "<<std::endl;
}

void UIManager::update(double)
{
	for(std::map<std::string, ui::Window*>::iterator it = windows.begin(); it != windows.end(); it++)
	{
		it->second->update();
		reserve(it->second);
		it->second->setText();
	}
}
