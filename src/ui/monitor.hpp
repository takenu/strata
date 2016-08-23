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
#include "../interface/appl.hpp"

#include "ui/window.hpp"

namespace strata
{
	namespace ui
	{
		/** The Monitor is a specific Window that displays (a subset of) a limited number of Strata
		  * parameters, such as the fps and the memory usage. Such parameters are a consequence of
		  * the choices made when creating a Terrain, but they are not adjustable and merely give
		  * an indication of the effective complexity of the landscape that is being generated. */
		class Monitor : public Window
		{
			private:
				intf::ApplInterface * applInterface;
				bool showFramesPerSecond;
				bool showMemoryUsage;
			public:
				Monitor(intf::UIInterface * _ui, intf::ApplInterface * _appl,
						tiny::draw::IconTexture2D * _fontTexture) :
					Window(_ui, _fontTexture),
					applInterface(_appl),
					showFramesPerSecond(false),
					showMemoryUsage(false)
				{
					registerTriggerKey(SDLK_m);
				}

				virtual void receiveWindowInput(const SDLKey &, const SDLMod &, bool)
				{
				}

				/** Update the text displayed by the monitor window.
				  * Since we cannot use tiny::draw::TextBox::reserve() from
				  * here, we do not use setText(), which instead must be
				  * done by whoever created the Monitor and has the ability
				  * to add and remove renderable objects. */
				virtual void update(void)
				{
					if(!isVisible()) return;
					clear();
					drawTitle();
					if(showFramesPerSecond)
					{
						std::stringstream ss;
						ss << "Running at "<<1.0/applInterface->getFPS()<<" fps.";
						addTextFragment(ss.str(), getColour());
						addNewline();
					}
					if(showMemoryUsage)
					{
						intf::UIInformation meminfo = uiInterface->getUIInfo("Terrain");
						for(unsigned int i = 0; i < meminfo.pairs.size(); i++)
						{
							addTextFragment("Terrain: "+meminfo.pairs[i].first+" is "+meminfo.pairs[i].second, getColour());
							addNewline();
						}
					}
				}

				virtual void setWindowAttribute(std::string attribute, std::string value)
				{
					if(attribute=="fps") showFramesPerSecond = tool::toBoolean(value);
					if(attribute=="memusage") showMemoryUsage = tool::toBoolean(value);
				}
		};
	} // end namespace ui
} // end namespace strata
