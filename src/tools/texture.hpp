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

#include <tiny/math/vec.h>
#include <tiny/draw/texture2d.h>

namespace strata
{
	namespace tools
	{
		/** Create an opaque test texture of type RGBTexture2D. */
		inline tiny::draw::RGBTexture2D * createTestTexture(const unsigned int &size = 64, const unsigned char &r = 255, const unsigned char &g = 255, const unsigned char &b = 255)
		{
			tiny::img::Image image(size, size);
			unsigned char * data = &image.data[0];

			//Create a simple test image.
			for (size_t i = 0; i < size; ++i)
			{
				for (size_t j = 0; j < size; ++j)
				{
					float shade = sqrt(16*(i*(size-i)*j*(size-j)/(1.0f*size*size*size*size)));
					shade = 0.5+0.5*shade;

					*data++ = (unsigned char)(r*shade);
					*data++ = (unsigned char)(g*shade);
					*data++ = (unsigned char)(b*shade);
					*data++ = 255;
				}
			}

			return new tiny::draw::RGBTexture2D(image);
		}

		/** Create a test texture with added alpha channel, of type RGBATexture2D. */
		inline tiny::draw::RGBATexture2D * createTestTextureAlpha(const unsigned int &size = 64, const unsigned char &r = 255, const unsigned char &g = 255, const unsigned char &b = 255, const unsigned char &a = 255)
		{
			tiny::img::Image image(size, size);
			unsigned char * data = &image.data[0];

			//Create a simple test image.
			for (size_t i = 0; i < size; ++i)
			{
				for (size_t j = 0; j < size; ++j)
				{
					float shade = sqrt(16*(i*(size-i)*j*(size-j)/(1.0f*size*size*size*size)));
					shade = 0.5+0.5*shade;

					*data++ = (unsigned char)(r*shade);
					*data++ = (unsigned char)(g*shade);
					*data++ = (unsigned char)(b*shade);
					*data++ = a;
				}
			}

			return new tiny::draw::RGBATexture2D(image, tiny::draw::tf::filter);
		}
	}
}
