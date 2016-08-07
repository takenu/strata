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
#include <tiny/draw/effects/showimage.h>
#include <tiny/img/io/image.h>

namespace strata
{
	namespace ui
	{
		/** An on-screen square object, e.g. the background of a Window. This is a separate Renderable
		  * object, and as such it needs to be added to the Renderer apart from any object that it is
		  * a part of (such as a Window) to be properly visible. */
		class ScreenSquare : public tiny::draw::effects::ShowImage
		{
			private:
				tiny::draw::RGBATexture2D * texture;
			public:
				ScreenSquare(tiny::draw::RGBATexture2D * _texture) :
					tiny::draw::effects::ShowImage(), texture(_texture)
				{
					setImageTexture(*texture);
					setAlpha(1);
				}

				void setBoxDimensions(float left, float top, float right, float bottom)
				{
					// call tiny::draw::ScreenFillingSquare's setSquareDimensions
					setSquareDimensions(left, top, right, bottom);
				}

				~ScreenSquare(void) {}
		};

		/** The Window is a base class to all in-game window objects. It defines the size of the
		  * window and manages the interaction with the renderable object used to visualize the
		  * Window. The Window itself has little meaning as only derived classes implement actual
		  * functionality (e.g. displaying inventories or skill trees), and therefore creating
		  * Window objects directly is normally not useful. */
		class Window : public tiny::draw::TextBox
		{
			private:
				tiny::draw::Colour colour; /**< Text colour. */
				ScreenSquare * background; /**< Background texture object. */
			protected:
				intf::UIInterface * uiInterface;
			public:
				Window(intf::UIInterface * _ui, tiny::draw::IconTexture2D * _fontTexture,
						float _fontSize, float _aspectRatio,
						tiny::draw::Colour _colour = tiny::draw::Colour(255,255,255)) :
					tiny::draw::TextBox(_fontTexture, _fontSize, _aspectRatio),
					colour(_colour), uiInterface(_ui)
				{
				}

				void setBackground(ScreenSquare * ss) { background = ss; }

				tiny::draw::Colour getColour(void) const { return colour; }
		};
	}
} // end namespace strata
