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
					setAlpha(0);
				}

				void setBoxDimensions(float left, float top, float right, float bottom)
				{
					// call tiny::draw::ScreenFillingSquare's setSquareDimensions
					setSquareDimensions(left, top, right, bottom);
				}

				~ScreenSquare(void) {}
		};
	} // end namespace ui
} // end namespace strata

