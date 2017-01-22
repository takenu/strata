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
				/** An ordered list of UI listeners. This defines the order in which they have a
				  * chance to interpret the input. In principle, we use a one-input one-response
				  * approach, and therefore only the highest ranked listener that subscribed a given
				  * input will be able to act on it. */
				std::list<intf::UIListener*> listeners;

				/** Typedef the iterator type for listeners, to avoid lengthy loop initializers. */
				typedef std::list<intf::UIListener*>::iterator ListenerIterator;
			public:
				InputInterpreter(void)
				{
				}

				~InputInterpreter(void)
				{
				}

				/** Subscribe a new Listener. The InputInterpreter ensures every UIListener is subscribed
				  * only once, but will spawn a warning to signal incorrect usage. */
				void subscribe(intf::UIListener * listener)
				{
					for(ListenerIterator it = listeners.begin(); it != listeners.end(); it++)
						if(*it == listener)
						{
							std::cout << " InputInterpreter::subscribe() :";
							std::cout << " WARNING: Listener already exists!"<<std::endl;
							return;
						}
					listeners.push_front(listener);
				}

				/** Remove a certain UIListener. This will delete its reference from the list of listeners
				  * and ensures that the InputInterpreter will not forward any input to it anymore.
				  * A warning will be issued if an attempt is done to delete a listener that is already
				  * deleted or that has never been added. */
				void unsubscribe(intf::UIListener * listener)
				{
					for(ListenerIterator it = listeners.begin(); it != listeners.end(); it++)
						if(*it == listener)
						{
							listeners.erase(it);
							return;
						}
					std::cout << " InputInterpreter::unsubscribe() :";
					std::cout << " WARNING: Listener not found!"<<std::endl;
				}

				/** Bump a certain UIListener, such that it becomes the first in the line of all listeners.
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
					for(ListenerIterator it = listeners.begin(); it != listeners.end(); it++)
						if(*it == listener)
						{
							// Move the listener to the front of the list.
							listeners.splice(listeners.begin(), listeners, it);
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
					for(ListenerIterator it = listeners.begin(); it != listeners.end(); it++)
						if((*it)->keyIsSubscribed(k))
						{
							(*it)->receiveKeyInput(k, m, isDown);
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
					for(ListenerIterator it = listeners.begin(); it != listeners.end(); it++)
						if((*it)->receiveMouseEvent(x, y, buttons)) return true;
					return false;
				}
		};
	}
} // end namespace strata
