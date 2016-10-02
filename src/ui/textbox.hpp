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

#include <sstream>
#include <vector>

#include <tiny/draw/textbox.h>

namespace strata
{
	namespace ui
	{
		class TextBox : public tiny::draw::TextBox
		{
			private:
				tiny::draw::Colour colour; /**< Text colour. */
				tiny::draw::Colour secondaryColour; /**< Secondary text colour. */
				tiny::vec4 textBox; /**< The box that the text is inside of. */
			protected:
			public:
				TextBox(tiny::draw::IconTexture2D * _fontTexture, float _fontsize = 0.1f,
						float _aspectratio = 2.0f) :
					tiny::draw::TextBox(_fontTexture, _fontsize, _aspectratio),
					colour(0,0,0), secondaryColour(100,100,100) {}

				/** Allow modifying the dimensions of the Textbox. */
				void setTextboxDimensions(float left, float top, float right, float bottom)
				{
					textBox = tiny::vec4(left, top, right, bottom);
					setBoxDimensions(textBox.x, textBox.y, textBox.z, textBox.w);
				}

				/** Reserve a text box for a single textbox object. */
				void reserveTextBox(std::vector<tiny::draw::Renderable *> &oldTextBoxes,
						std::vector<tiny::draw::Renderable *> &newTextBoxes)
				{
					tiny::draw::Renderable * oldTextBox = 0;
					tiny::draw::Renderable * newTextBox = 0;
					newTextBox = reserve(oldTextBox); // call tiny::draw::TextBox::reserve()
					if(oldTextBox)
					{
						oldTextBoxes.push_back(oldTextBox);
						if(newTextBox) newTextBoxes.push_back(newTextBox);
						else std::cout << " Window::reserveTextBoxes() : No new textbox! "<<std::endl;
					}
				}

				void setColour(const tiny::draw::Colour & _colour) { colour = _colour; }
				void setSecondaryColour(const tiny::draw::Colour & _colour) { secondaryColour = _colour; }
				const tiny::draw::Colour & getColour(void) const { return colour; }
				const tiny::draw::Colour & getSecondaryColour(void) const { return secondaryColour; }
		};
	} // end namespace ui
} // end namespace strata
