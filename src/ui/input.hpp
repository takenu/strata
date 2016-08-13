/*
This file is part of Chathran Strata: https://github.com/takenu/strata
Copyright 2015-2016, Matthijs van Dorp.

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

#include "../interface/ui.hpp"

namespace strata
{
	namespace ui
	{
		/** An interpreter that maps specific inputs to calls to subscribed listeners. */
		class InputInterpreter
		{
			private:
				std::vector<intf::UIListener> listeners;
			public:
				InputInterpreter(void)
				{
				}

				~InputInterpreter(void)
				{
				}
		};
	}
} // end namespace strata