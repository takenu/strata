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

#include "textbox.hpp"
#include "screensquare.hpp"
#include "button.hpp"

namespace strata
{
	namespace ui
	{
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
		class Window : public TextBox, public intf::UIListener
		{
			private:
				ScreenSquare * background; /**< Background texture object. */
				ScreenSquare * highlight; /**< Highlight object, for active (parts of the) Window. */
				std::set<SDLKey> triggerKeys; /**< Keys that activate/close the Window. */
				std::set<SDLKey> activeKeys; /**< Keys the Window listens to while active. */
				std::map<std::string, Button> buttons; /**< Buttons on the Window. */
				bool visible;
				tiny::vec4 windowBox; /**< The box that the window is inside of. */
				intf::UIInterface * uiInterface;
				intf::InputSet * inputKeys;
				std::string title;

				typedef std::map<std::string, Button>::iterator ButtonIterator;

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

				inline bool hasButton(std::string key) const
				{
					return (buttons.count(key)>0);
				}

				/** Get a Button object. */
				Button * getButton(std::string key)
				{
					if(hasButton(key)) return &(buttons.find(key)->second);
					else return 0;
				}

				/** Receive mouse input on the Window.
				  * There are two types of mouse input: clicks and movement. This function is only used
				  * for clicks, as movement is managed through the ApplManager (and basically only
				  * switches between static-centered and free).
				  * The return value of the function should be whether or not the click was 'processed'.
				  * Clicks can only be processed once, so it is important that a Window signals it when
				  * it can interpret the mouse click. */
				virtual bool receiveMouseEvent(float x, float y, unsigned int b)
				{
					//std::cout << " Window::receiveMouseEvent() : Window box = ("<<windowBox.x<<", "
					//	<<windowBox.z<<") x ("<<windowBox.y<<", "<<windowBox.w<<")"<<std::endl;
					if(x > windowBox.x && x < windowBox.z && y < windowBox.y && y > windowBox.w)
					{
						if(b > 0)
							std::cout << " Window::receiveMouseEvent() : Click on "<<title<<"!"<<std::endl;
						return true;
					}
					else return false;
				}

				void setVisible(bool v)
				{
					visible = v;
					background->setAlpha(v);
					if(!isVisible())
					{
						clear();
						resetInputKeys();
						for(ButtonIterator it = buttons.begin(); it != buttons.end(); it++)
							it->second.setVisible(false);
					}
					else
					{
						activateInputKeys();
						inputKeys->addKey(SDLK_ESCAPE); // to allow closing via Esc
						uiInterface->bump(this);
						for(ButtonIterator it = buttons.begin(); it != buttons.end(); it++)
							it->second.setVisible(true);
					}
				}
			protected:
				bool isVisible(void) const { return visible; } /**< Check whether window is visible. */
				void setInvisible(void) { setVisible(false); } /**< Derived window can close itself. */
				intf::UIInterface * getUIInterface(void) { return uiInterface; }

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
					TextBox(_fontTexture),
					intf::UIListener(_ui),
					background(0), visible(false),
					uiInterface(_ui), inputKeys(0), title(_title)
				{
					inputKeys = uiInterface->subscribe(this);
				}

				void loadButton(std::string buttonId)
				{
					if(hasButton(buttonId))
						std::cout << " Window::loadButton() : '"<<buttonId<<"' exists!"<<std::endl;
					else buttons.emplace(buttonId, Button());
				}

				void setButtonTextBox(std::string buttonId, tiny::draw::IconTexture2D * _fontTexture)
				{
					if(hasButton(buttonId))
						buttons[buttonId].setTextBox(_fontTexture);
				}

				tiny::draw::Renderable * getButtonRenderable(std::string buttonId)
				{
					if(hasButton(buttonId)) return getButton(buttonId)->getRenderable();
					else
					{
						std::cout << " Window::getButtonRenderable() : No '"<<buttonId<<"'!"<<std::endl;
						return 0;
					}
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

				void setBackground(std::string type, ScreenSquare * ss)
				{
					if(type == "background")
					{
						background = ss;
						background->setBoxDimensions(windowBox.x, windowBox.y, windowBox.z, windowBox.w);
					}
					else if(type == "highlight")
					{
						highlight = ss;
						highlight->setBoxDimensions(windowBox.x, windowBox.y, windowBox.z, windowBox.w);
						highlight->setAlpha(0);
					}
					else if(buttons.count(type) > 0)
					{
						buttons.find(type)->second.setBackground(ss);
					}
					else std::cout << " Window::setBackground() : Type '"<<type<<"' not found!"<<std::endl;
				}

				// Not yet implemented - currently not necessary to retrieve background renderables
/*				tiny::draw::Renderable * getBackgroundRenderable(std::string type)
				{
					if(type == "background") return getRenderable();
					else if(hasButton(type)) return buttons[type].getBackgroundRenderable();
				}*/

				virtual void updateWindow(void) = 0;
				void update(void)
				{
					updateWindow();
					for(ButtonIterator it = buttons.begin(); it != buttons.end(); it++)
						it->second.update();
				}

				/** Reserve text boxes for the Window and all its Buttons. */
				void reserveTextBoxes(std::vector<tiny::draw::Renderable *> &oldTextBoxes,
						std::vector<tiny::draw::Renderable *> &newTextBoxes)
				{
					reserveTextBox(oldTextBoxes, newTextBoxes);
					for(ButtonIterator it = buttons.begin(); it != buttons.end(); it++)
						it->second.getTextBox()->reserveTextBox(oldTextBoxes, newTextBoxes);
				}

				void setTexts(void)
				{
					setText();
					for(ButtonIterator it = buttons.begin(); it != buttons.end(); it++)
						it->second.getTextBox()->setText();
				}

				/** Allow setting of arbitrary Lua-derived attributes through key-value pairs.
				  * Classes deriving the Window must implement all their customizable attributes
				  * through interpretation of the information passed through this function. */
				void setAttribute(std::string attribute, std::string value)
				{
					if(attribute == "title") title = value;
					else if(attribute == "fontsize") setFontSize( tool::toFloat(value) );
					else if(attribute == "fontaspectratio") setAspectRatio( tool::toFloat(value) );
					// Send all attributes, even those that already affect the Window's base parameters.
					// The derived class may have textboxes too that also may want to adjust their font
					// size and aspect ratio.
					setWindowAttribute(attribute, value);
				}

				/** Allow setting of Button attributes. */
				void setButtonAttribute(std::string button, std::string attribute, std::string value)
				{
					if(hasButton(button)) buttons[button].setAttribute(attribute, value);
					else
					{
						std::cout << " Window::setButtonAttribute() : No button "<<button
							<< " to set "<<attribute<<"="<<value<<"!"<<std::endl;
					}
				}

				/** Set attributes not defined by the Window itself but by the derived class. */
				virtual void setWindowAttribute(std::string attribute, std::string value) = 0;

				/** Allow setting of font colours. */
				void setFontColour(std::string attribute, const tiny::draw::Colour & _colour)
				{
					if(attribute == "fontcolour") setColour(_colour);
					else if(attribute == "fonthighlight") setSecondaryColour(_colour);
					// Similar to attributes, send all font colours (including default ones)
					// to derived windows.
					setWindowFontColour(attribute, _colour);
				}

				/** Set a window font colour not already implemented. Given that many windows won't
				  * need more than one or two colours, this function is merely virtual (not purely
				  * so) and one does not need to override it. */
				virtual void setWindowFontColour(std::string, const tiny::draw::Colour &)
				{
				}

				/** Allow modifying the dimensions of Window elements. */
				void setDimensions(std::string attribute, float left, float top, float right, float bottom)
				{
					if(attribute == "box")
					{
						windowBox = tiny::vec4(left, top, right, bottom);
						setTextboxDimensions(left, top, right, bottom);
						if(background) background->setBoxDimensions(
								left, top, right, bottom);
					}
					else if(hasButton(attribute))
						getButton(attribute)->setDimensions(left, top, right, bottom);
					// Elements of the derived window may also need to be adjusted
					setWindowDimensions(attribute, left, top, right, bottom);
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
