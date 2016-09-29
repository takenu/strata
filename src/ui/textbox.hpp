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

				void setColour(const tiny::draw::Colour & _colour) { colour = _colour; }
				void setSecondaryColour(const tiny::draw::Colour & _colour) { secondaryColour = _colour; }
				const tiny::draw::Colour & getColour(void) const { return colour; }
				const tiny::draw::Colour & getSecondaryColour(void) const { return secondaryColour; }
		};
	} // end namespace ui
} // end namespace strata
