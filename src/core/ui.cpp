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

#include "ui.hpp"

using namespace strata::core;

void UIManager::registerLuaFunctions(sel::State & luaState)
{
	luaState["ui"].SetObj(*this,
			"loadFont", &UIManager::loadFont,
			"loadMonitorWindow", &UIManager::loadMonitorWindow,
			"loadMonitorWindowAttribute", &UIManager::loadMonitorWindowAttribute
			);
}

void UIManager::loadMonitorWindow(float left, float top, float right, float bottom,
		unsigned int r, unsigned int g, unsigned int b, std::string title)
{
	if(left < -1.0f || left > 1.0f || top < -1.0f || top > 1.0f
			|| right < -1.0f || right > 1.0f || bottom < -1.0f || bottom > 1.0f)
	{
		std::cerr << " UIManager::loadMonitorWindow() : Window box "<<left<<","<<top<<","
			<<right<<","<<bottom<<" is invalid! "<<std::endl;
		return;
	}
//	std::cout << " UIManager::loadMonitorWindow() : Monitor "<<left<<","<<top<<","
//			<<right<<","<<bottom<<" to be created with title "<<title
//			<<" and colour "<<r<<","<<g<<","<<b<<"... "<<std::endl;

	monitor = new ui::Monitor(fontTexture, defaultFontSize, defaultAspectRatio,
			tiny::draw::Colour(static_cast<unsigned char>(r),
				static_cast<unsigned char>(g),static_cast<unsigned char>(b)), title);
	monitor->setBoxDimensions(left, top, right, bottom);
	renderer->addScreenRenderable(monitor->getRenderable(), false, false, tiny::draw::BlendMix);
}

void UIManager::loadMonitorWindowAttribute(std::string attribute, std::string value)
{
	if(attribute == "fps") monitor->displayFPS( tool::toBoolean(value) );
	else if(attribute == "memusage") monitor->displayMemoryUsage( tool::toBoolean(value) );
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
	if(oldTextBox) renderer->freeScreenRenderable(oldTextBox);
	if(newTextBox)
	{
		renderer->addScreenRenderable(newTextBox, false, false, tiny::draw::BlendMix);
	}
	else if(oldTextBox) std::cout << " UIManager::reserve() : ERROR: No new renderable! "<<std::endl;
}

void UIManager::update(double dt)
{
	if(monitor)
	{
		monitor->update(dt);
		reserve(monitor);
		monitor->setText();
	}
}
