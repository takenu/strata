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

#include "../interface/ui.hpp"
#include "../interface/appl.hpp"

#include "textbox.hpp"
#include "screensquare.hpp"

namespace strata
{
	namespace ui
	{
		/** The Button is a clickable object. It can be linked to an arbitrary function. */
		class Button // : public intf::UIListener
		{
			private:
				ScreenSquare * background; /**< Background texture object. */
				bool visible; /**< Whether to draw the Button or hide it. */
				tiny::vec4 buttonBox; /**< The box that the button is inside of. */
				std::string text; /**< Text to be displayed on the button. */
				TextBox * textbox; /**< A textbox to hold text. */
				tiny::draw::RGBATexture2D * texture; /**< A texture for the button. */

				/** Receive mouse input (clicks) on the Button.
				  * The return value of the function should be whether or not the click was 'processed'.
				  * Clicks can only be processed once, so it is important that a Button signals it when
				  * it can interpret the mouse click. Buttons may not have rectangular shapes in all
				  * cases, therefore this approach should enable buttons to partially 'overlap' as
				  * rectangles, while still be properly clickable (e.g. in a hexagonal tiling). In
				  * addition, non-rectangular buttons should only receive clicks when these are well
				  * placed, and not when they are just next to them. */
				virtual bool receiveMouseTrigger(float x, float y)
				{
					//std::cout << " Button::receiveMouseEvent() : Button box = ("<<buttonBox.x<<", "
					//	<<buttonBox.z<<") x ("<<buttonBox.y<<", "<<buttonBox.w<<")"<<std::endl;
					if(x > buttonBox.x && x < buttonBox.z && y < buttonBox.y && y > buttonBox.w)
					{
						// TODO: Use texture alpha channel to determine if click hits visible part of button
						std::cout << " Button::receiveMouseEvent() : Click on "<<text<<"!"<<std::endl;
						return true;
					}
					else return false;
				}

			public:
				void setVisible(bool v)
				{
					visible = v;
					background->setAlpha(v);
					if(!isVisible())
					{
						if(textbox) textbox->clear();
					}
					else
					{
					}
				}
				bool isVisible(void) const { return visible; } /**< Check whether button is visible. */

				Button(void) :
					background(0), visible(false), buttonBox(0.0f,1.0f,1.0f,0.0f),
					text(""), textbox(0)
				{
				}

				tiny::draw::Renderable * getRenderable(void)
				{
					if(textbox) return textbox->getRenderable();
					else return 0;
				}

				// Not necessary yet
/*				tiny::draw::Renderable * getBackgroundRenderable(void)
				{
					if(background) return background->getRenderable();
					else return 0;
				}*/

				void setBackground(ScreenSquare * ss)
				{
					background = ss;
					background->setBoxDimensions(buttonBox.x, buttonBox.y, buttonBox.z, buttonBox.w);
				}

				void setTextBox(tiny::draw::IconTexture2D * _fontTexture)
				{
					textbox = new TextBox(_fontTexture);
				}

				TextBox * getTextBox(void)
				{
					return textbox;
				}

				virtual void update(void)
				{
					if(!isVisible()) return;
					if(textbox)
					{
						textbox->clear();
						textbox->addTextFragment(text, textbox->getColour());
					}
				}

				/** Allow setting of Button font colours. */
				void setFontColour(std::string attribute, const tiny::draw::Colour & _colour)
				{
					if(!textbox) return;
					if(attribute == "fontcolour") textbox->setColour(_colour);
					else if(attribute == "fonthighlight") textbox->setSecondaryColour(_colour);
					else std::cout << " Button::setFontColour() : Attribute '"<<attribute<<"' not defined!"<<std::endl;
				}

				/** Allow modifying the dimensions of Buttons. */
				void setDimensions(float left, float top, float right, float bottom)
				{
					buttonBox = tiny::vec4(left, top, right, bottom);
					if(textbox) textbox->setTextboxDimensions(left, top, right, bottom);
					if(background) background->setBoxDimensions(
							left, top, right, bottom);
				}

				/** Allow setting of various attributes. */
				void setAttribute(std::string attribute, std::string value)
				{
					if(textbox)
					{
						if(attribute == "text") text = value;
						else if(attribute == "fontsize") textbox->setFontSize( tool::toFloat(value) );
						else if(attribute == "fontaspectratio") textbox->setAspectRatio( tool::toFloat(value) );
						// Send all attributes, even those that already affect the Window's base parameters.
						// The derived class may have textboxes too that also may want to adjust their font
					}
					else std::cout << " Button::setAttribute() : No textbox, nothing done! "<<std::endl;
				}
		};
	}
} // end namespace strata
