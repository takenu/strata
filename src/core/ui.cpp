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

#include "ui.hpp"

using namespace strata::core;

void UIManager::registerLuaFunctions(sel::State & luaState)
{
	luaState["ui"].SetObj(*this,
			"loadUI", &UIManager::loadUI);
}

void UIManager::loadUI(std::string fontTex, float fontSize, float fontAspectRatio,
		unsigned int fontPixels, unsigned int fontResolution)
{
	if(fontTex == "" || fontSize < 0.01f || fontSize > 1.0f || fontAspectRatio < 0.2f
			|| fontAspectRatio > 10.0f || fontPixels < 12 || fontPixels > 1024
			|| fontResolution < 128 || fontResolution > 16384)
	{
		std::cerr << " UIManager::loadUI() : ERROR: Lua call with invalid arguments: ";
		std::cerr << fontTex << ", " << fontSize << ", " << fontAspectRatio << ", ";
		std::cerr << fontPixels << ", " << fontResolution << std::endl;
	}
	fontTexture = new tiny::draw::IconTexture2D(fontResolution,fontResolution);
	fontTexture->packIcons(tiny::img::io::readFont(DATA_DIRECTORY + "font/" + fontTex,fontPixels));
	textBox = new tiny::draw::TextBox(fontTexture, fontSize, fontAspectRatio);
	renderer->addScreenRenderable(textBox->getRenderable(), false, false, tiny::draw::BlendMix);
	textBox->setBoxDimensions(-1.0f,1.0f,0.0f,0.8f);
	std::string str("Chathran Strata");
	textBox->addTextFragment(str, tiny::draw::Colour(180,255,180));
	tiny::draw::Renderable * oldTextBox = 0;
	tiny::draw::Renderable * newTextBox = textBox->reserve(oldTextBox);
	if(oldTextBox) renderer->freeScreenRenderable(oldTextBox);
	if(newTextBox)
		renderer->addScreenRenderable(newTextBox, false, false, tiny::draw::BlendMix);
	else std::cout << " UIManager() : No new renderable! "<<std::endl;
	textBox->setText();
}
