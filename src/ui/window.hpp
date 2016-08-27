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
					setAlpha(0);
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
		  * Window objects directly is normally not useful.
		  * Windows have a few responsibilities of their own that are intended to be shared among
		  * all windows. They manage opening and closing of the window, they encapsulate a large
		  * part of the management of the InputSet that it inherits from the UIListener, and they
		  * contain two text colours, both a default colour and a secondary colour that can be
		  * used for e.g. highlighting.
		  */
		class Window : public tiny::draw::TextBox, public intf::UIListener
		{
			private:
				tiny::draw::Colour colour; /**< Text colour. */
				tiny::draw::Colour secondaryColour; /**< Secondary text colour. */
				ScreenSquare * background; /**< Background texture object. */
				std::set<SDLKey> triggerKeys; /**< Keys that activate/close the Window. */
				std::set<SDLKey> activeKeys; /**< Keys the Window listens to while active. */
				bool visible;
				tiny::vec4 windowBox; /**< The box that the window is inside of. */

				/** Reset the Window's input, such that it only responds to its trigger keys. */
				void resetInputKeys(void)
				{
					inputKeys->resetKeySet(triggerKeys);
				}

				/** Activate the entire 'activeKeys' key set as applicable window input. */
				void activateInputKeys(void)
				{
					inputKeys->resetKeySet(activeKeys);
					inputKeys->addKeySet(triggerKeys); // for de-activation of window
				}

				/** Receive key input on the Window. First the Window gets to do its generic
				  * operations. If the Window is in visible mode and SDLK_ESCAPE is not
				  * explicitly registered as an input key, then SDLK_ESCAPE will trigger a
				  * window close. If SDLK_ESCAPE is registered, the only way for the window to
				  * close is for the window itself to call setVisible(false). However, this
				  * registration should be avoided whenever possible, in order to keep the
				  * Window's close key as universal as possible.
				  * Similar to SDLK_ESCAPE, every trigger key that activates a window can also
				  * deactivate it. However, if the trigger key is part of the active keys, it
				  * will not deactivate the window (unless the derived class explicitly defines
				  * it to do so).
				  */
				virtual void receiveKeyInput(const SDLKey & k, const SDLMod & m, bool isDown)
				{
					if(isVisible())
					{
						if(isDown &&
							   ((k == SDLK_ESCAPE && activeKeys.count(SDLK_ESCAPE)==0 ) ||
								(triggerKeys.count(k) > activeKeys.count(k))))
						{
							setVisible(false);
						}
						else receiveWindowInput(k, m, isDown);
					}
					else if(isDown && !isVisible() && triggerKeys.count(k) > 0)
					{
						setVisible(true);
					}
					else if(isDown)
					{
						std::cout << " Window::receiveKeyInput() : Received unsubscribed info! "<<std::endl;
					}
				}

				void setVisible(bool v)
				{
					visible = v;
					background->setAlpha(v);
					if(!isVisible())
					{
						clear();
						resetInputKeys();
					}
					else
					{
						activateInputKeys();
						inputKeys->addKey(SDLK_ESCAPE); // to allow closing via Esc
						uiInterface->bump(this);
					}
				}
			protected:
				intf::UIInterface * uiInterface;
				intf::InputSet * inputKeys;
				std::string title;
				bool isVisible(void) const { return visible; } /**< Check whether window is visible. */
				void setInvisible(void) { setVisible(false); } /**< Derived window can close itself. */
				const tiny::draw::Colour & getSecondaryColour(void) const { return secondaryColour; }

				virtual void receiveWindowInput(const SDLKey & k, const SDLMod & m, bool isDown) = 0;

				void drawTitle(void)
				{
					if(title.length() > 0)
					{
						addTextFragment(title, getColour());
						addNewline();
					}
				}
			public:
				Window(intf::UIInterface * _ui, tiny::draw::IconTexture2D * _fontTexture,
						std::string _title = "") :
					tiny::draw::TextBox(_fontTexture, 0.1f, 2.0f),
					intf::UIListener(_ui),
					colour(0,0,0), secondaryColour(100,100,100),
					background(0), visible(false),
					uiInterface(_ui), inputKeys(0), title(_title)
				{
					inputKeys = uiInterface->subscribe(this);
				}

				/** Register a key as a trigger that opens this Window. This function also
				  * pushes the trigger to the initial input key set. At a later stage, the
				  * trigger keys can be re-set as the only keys whose input can trigger
				  * activation of the Window, using resetInputKeys(). */
				void registerTriggerKey(const SDLKey & k)
				{
					triggerKeys.emplace(k);
					inputKeys->addKey(k); // Always add to input keys (also for deactivation)
				}

				/** Register a key as a trigger to be received by the Window when it is active.
				  * The deriving Window must implement every registered active key in its
				  * virtual receiveWindowInput() function.
				  */
				void registerActiveKey(const SDLKey &k)
				{
					activeKeys.emplace(k);
					if(isVisible()) inputKeys->addKey(k);
				}

				void registerActiveKeySet(const std::set<SDLKey> &k)
				{
					activeKeys = uniteKeySets(k, activeKeys);
				}

				void setBackground(ScreenSquare * ss)
				{
					background = ss;
					background->setBoxDimensions(windowBox.x, windowBox.y, windowBox.z, windowBox.w);
				}

				virtual void update(void) = 0;

				tiny::draw::Colour getColour(void) const { return colour; }

				/** Allow setting of arbitrary Lua-derived attributes through key-value pairs.
				  * Classes deriving the Window must implement all their customizable attributes
				  * through interpretation of the information passed through this function. */
				void setAttribute(std::string attribute, std::string value)
				{
					if(attribute == "fontsize") setFontSize( tool::toFloat(value) );
					else if(attribute == "fontaspectratio") setAspectRatio( tool::toFloat(value) );
					else setWindowAttribute(attribute, value);
				}

				/** Set attributes not defined by the Window itself but by the derived class. */
				virtual void setWindowAttribute(std::string attribute, std::string value) = 0;

				/** Allow setting of font colours. */
				virtual void setFontColour(std::string attribute, const tiny::draw::Colour & _colour)
				{
					if(attribute == "fontcolour") colour = _colour;
					else if(attribute == "fontcolour2") secondaryColour = _colour;
					else setWindowFontColour(attribute, _colour);
				}

				/** Set a window font colour not already implemented. Given that many windows won't
				  * need more than one or two colours, this function is merely virtual (not purely
				  * so) and one does not need to override it. */
				virtual void setWindowFontColour(std::string, const tiny::draw::Colour &)
				{
				}

				/** Allow modifying the dimensions of the Textbox. */
				void setDimensions(std::string attribute, float left, float top, float right, float bottom)
				{
					if(attribute == "box")
					{
						windowBox = tiny::vec4(left, top, right, bottom);
						setBoxDimensions(windowBox.x, windowBox.y, windowBox.z, windowBox.w);
						if(background) background->setBoxDimensions(
								windowBox.x, windowBox.y, windowBox.z, windowBox.w);
					}
					else setWindowDimensions(attribute, left, top, right, bottom);
				}

				/** A function for child windows to receive instructions as to their dimensions.
				  * The arguments are: a string specifying the recipient of the dimensions, and
				  * a pair of 2D points representing the upper left and lower right of a square
				  * on the screen (which runs from (-1, 1) to (1,-1)).
				  */
				virtual void setWindowDimensions(std::string, float, float, float, float)
				{
				}
		};
	}
} // end namespace strata
