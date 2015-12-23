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
#pragma once

#include <tiny/draw/textbox.h>
#include <tiny/img/io/image.h>

#include "interface/appl.hpp"
#include "interface/render.hpp"

namespace strata
{
	namespace core
	{
		/** Take care of user input and user interface. */
		class UIManager
		{
			private:
				intf::ApplInterface * applInterface;
				intf::RenderInterface * renderer;

				tiny::draw::IconTexture2D * fontTexture;
				tiny::draw::TextBox * textBox;
			public:
				UIManager(intf::ApplInterface * _appl, intf::RenderInterface * _renderer) : applInterface(_appl), renderer(_renderer), fontTexture(0), textBox(0)
				{
				}

				void setText(void)
				{
					fontTexture = new tiny::draw::IconTexture2D(512,512);
					fontTexture->packIcons(tiny::img::io::readFont(DATA_DIRECTORY + "font/OpenBaskerville-0.0.75.ttf",48));
					textBox = new tiny::draw::TextBox(fontTexture, 0.2, 2);
					renderer->addScreenRenderable(textBox->getRenderable(), false, false, tiny::draw::BlendMix);
					textBox->setBoxDimensions(-1.0f,-0.8f,0.0f,-1.0f);
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

				void update(double)
				{
				}
		};
	}
}
