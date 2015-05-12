/*
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

#include "interface/appl.hpp"
#include "interface/render.hpp"

namespace ch
{
	namespace core
	{
		/** Take care of user input and user interface. */
		class UIManager
		{
			private:
				intf::ApplInterface * applInterface;
				intf::RenderInterface * renderer;
			public:
				UIManager(intf::ApplInterface * _appl, intf::RenderInterface * _renderer) : applInterface(_appl), renderer(_renderer)
				{
				}

				void update(double)
				{
				}
		};
	}
}
