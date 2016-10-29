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
				/** A list of registered input sets. Listeners register key sets when they want to
				  * implement certain behavior in response to specific input. If the same input is
				  * registered multiple times, the first occurrence in the list of key sets will be
				  * executed, but any others will not (for now). This way, input sets can temporarily
				  * overrule other input sets.
				  * The InputSet objects are not stored by the list but kept as pointers. This is because
				  * the InputSet is intended to be permanently bound to its UIListener, and to enforce
				  * that it takes its UIListener as an argument to its only constructor. Consequently,
				  * InputSet objects cannot be copied by the std::list and must be created separately.
				  */
				std::list<intf::InputSet*> keySets;

				/** Typedef the iterator type for keySets, to avoid lengthy loop initializers. */
				typedef std::list<intf::InputSet*>::iterator KeySetIterator;
			public:
				InputInterpreter(void)
				{
				}

				~InputInterpreter(void)
				{
				}

				/** Add an InputSet for a new UIListener. This should only be used once per UIListener,
				  * after which one should merely adjust the InputSet (not request a new one). Adding
				  * twice should not break anything, but is considered incorrect and will spawn a warning. */
				intf::InputSet * subscribe(intf::UIListener * listener)
				{
					for(KeySetIterator it = keySets.begin(); it != keySets.end(); it++)
						if((*it)->getListener() == listener)
						{
							std::cout << " InputInterpreter::subscribe() :";
							std::cout << " WARNING: Listener already exists!"<<std::endl;
							return *it;
						}
					keySets.push_front(new intf::InputSet(listener));
					return keySets.front();
				}

				/** Remove an InputSet related to a certain UIListener. This will delete the associated
				  * InputSet and ensure that the InputInterpreter will not forward any input to it
				  * anymore.
				  * A warning will be issued if an attempt is done to delete a listener that is already
				  * deleted or that has never been added. */
				void unsubscribe(intf::UIListener * listener)
				{
					for(KeySetIterator it = keySets.begin(); it != keySets.end(); it++)
						if((*it)->getListener() == listener)
						{
							delete (*it);
							keySets.erase(it);
							return;
						}
					std::cout << " InputInterpreter::unsubscribe() :";
					std::cout << " WARNING: Listener not found in keySets!"<<std::endl;
				}

				/** Bump an InputSet associated with a certain UIListener, such that it becomes the
				  * first in the line of all listeners.
				  * This may be required in situations where UIListeners need to (temporarily) ensure
				  * that they receive all input defined in their InputSet with absolute priority.
				  * Since it is not possible to return the InputSet to its original position, the only
				  * ways to reverse the change are:
				  * - Calling the same function for whatever UIListener receives conflicting input
				  *   (this is difficult);
				  * - Only using this function for UIListeners that have limited effect and that, on
				  *   specific input, will remove (nearly) all keys it defines in its InputSet, such
				  *   that the input can again be received by its usual recipient (recommended).
				  */
				void bump(intf::UIListener * listener)
				{
					for(KeySetIterator it = keySets.begin(); it != keySets.end(); it++)
						if((*it)->getListener() == listener)
						{
							// Move the listener to the front of the list.
							keySets.splice(keySets.begin(), keySets, it);
							return;
						}
					std::cout << " InputInterpreter::bump() :";
					std::cout << " WARNING: Listener not found in keySets!"<<std::endl;
				}

				/** Receive an input event and transmit it to the highest ranked listener.
				  * The input event that is broadcast may alter the InputInterpreter's state (e.g. by
				  * re-sorting the keySets or changing the number of keySets), therefore one cannot
				  * rely on the InputInterpreter to have a state consistent enough to transmit the
				  * input once more to a lower ranked listener.
				  * Obviously one could try to work around this but for now single-input single-result
				  * doesn't look like a harsh restriction. */
				bool receiveInput(const SDL_Keycode & k, const SDL_Keymod & m, bool isDown)
				{
					for(KeySetIterator it = keySets.begin(); it != keySets.end(); it++)
						if((*it)->isSubscribed(k))
						{
							(*it)->getListener()->receiveKeyInput(k, m, isDown);
							return true;
						}
					return false;
				}

				/** Receive a mouse event and transmit it to the highest ranked listener.
				  * Unlike key input, mouse input applies to whatever listener is visible at the current
				  * location of the mouse pointer. The ranking of listeners used is the same as for
				  * key events. */
				bool receiveInput(float x, float y, unsigned int buttons)
				{
					for(KeySetIterator it = keySets.begin(); it != keySets.end(); it++)
						if((*it)->getListener()->receiveMouseEvent(x, y, buttons)) return true;
					return false;
				}
		};
	}
} // end namespace strata
