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

#include "../tools/convertstring.hpp"

#include "../interface/ui.hpp"
#include "../interface/keys.hpp"
#include "../interface/appl.hpp"

#include "ui/window.hpp"

namespace strata
{
	namespace ui
	{
		class Console : public Window
		{
			private:
				intf::ApplInterface * applInterface;

				/** This string can contain an arbitrary chunk of executable Lua code. */
				std::string command;

				/** Execute currently entered Lua code, and hide the Console. */
				void executeAndHide(void)
				{
//					applInterface->executeLua(command);
					command.clear();
				}

				/** Convert SDL input into characters. */
				unsigned char convertSDLinput(const SDLKey & k, const SDLMod & m)
				{
					if(static_cast<unsigned char>(k) >= 'a' && static_cast<unsigned char>(k) <= 'z'
							&& (   ((m & KMOD_SHIFT) && !(m & KMOD_CAPS))
								|| ((m & KMOD_CAPS) && !(m & KMOD_SHIFT)) ))
						return static_cast<unsigned char>(k) + ('A' - 'a');
					else return static_cast<unsigned char>(k);
				}
			public:
				Console(intf::UIInterface * _ui, intf::ApplInterface * _appl,
						tiny::draw::IconTexture2D * _fontTexture) :
					Window(_ui, _fontTexture),
					applInterface(_appl)
				{
					registerTriggerKey(SDLK_RETURN);
					registerActiveKeySet( keySetAlphanumeric() );
					registerActiveKey(SDLK_RETURN);
				}

				virtual void receiveWindowInput(const SDLKey & k, const SDLMod & m, bool isDown)
				{
					if(isDown)
					{
						if(k == SDLK_RETURN) executeAndHide();
						else command.push_back( convertSDLinput(k,m) );
					}
				}

				virtual void update(void)
				{
					if(!isVisible()) return;
					clear();
				}

				virtual void setWindowAttribute(std::string, std::string)
				{
				}
		};
	} // end namespace ui
} // end namespace strata
