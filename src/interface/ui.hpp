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

#include <set>
#include <SDL.h> // for SDLKey enum only

#include <tiny/algo/typecluster.h>

#include "keys.hpp"

namespace strata
{
	namespace intf
	{
		class UIInterface;
		class UIListener;

		/** The InputSet is an object where a set of (keyboard/mouse) input commands can be defined,
		  * via their respective SDLKey symbols. */
		class InputSet
		{
			private:
				UIListener * listener; /**< A pointer to the class that will respond to key input. */
				std::set<SDLKey> keys; /**< The keys that are registered by the listener. */
			public:
				InputSet(UIListener * _listener) : listener(_listener) {}
				~InputSet(void) {}

				/** Get the UIListener that the InputSet is defined for. */
				UIListener * getListener(void)
				{
					return listener;
				}

				void addKey(SDLKey k)
				{
					keys.emplace(k);
				}

				void addKeySet(std::set<SDLKey> ks)
				{
					keys.insert(ks.begin(), ks.end());
				}

				void clearKeySet(void)
				{
					keys.clear();
				}

				void resetKeySet(std::set<SDLKey> ks)
				{
					clearKeySet();
					addKeySet(ks);
				}

				inline bool isSubscribed(const SDLKey &k)
				{
					return (keys.count(k) > 0);
				}
		};

		/** The UIListener is a base class to every object that can react to user input.
		  * Such listeners can subscribe to certain inputs, which are only applicable in
		  * specific contexts.
		  * Subscription is done through its link to the UI that governs it. The UI contains
		  * an InputInterpreter that processes the input and tracks who is subscribed to what.
		  */
		class UIListener
		{
			private:
				UIInterface * ui; /**< A link to the UI that governs the listener. */
			public:
				UIListener(UIInterface * _ui) : ui(_ui)
				{
				}

				~UIListener(void) {}

				/** Signal that a key is pressed down or no longer pressed down. */
				virtual void receiveKeyInput(const SDLKey & k, const SDLMod & m, bool isDown) = 0;
		};

		/** The UIInformation holds information to be used by the UI (for displaying
		  * the information on-screen). It is stored as attribute-value pairs. */
		class UIInformation
		{
			public:
				std::vector<std::pair<std::string, std::string> > pairs;

				inline void addPair(std::string attr, std::string val)
				{
					pairs.push_back( make_pair(attr, val) );
				}
		};

		/** The UISource is a base class to every object that is UI-representable.
		  * Such objects can be given UI elements (e.g. windows, health bars) to
		  * represent them.
		  * The determination of whether the UI element is visible is always done
		  * by the UI. The UISource-derived object should then respond by completing
		  * the UISource's data fields as applicable.
		  * Note that the UISource, being a TypeCluster-governed object, will automatically
		  * add and free itself by calling upon the TypeCluster owned by the UIInterface.
		  * Consequently, it is not possible to create UISource objects before the
		  * UIManager is created, and it is not possible to delete them after the
		  * UIManager is destroyed.
		  */
		class UISource : public tiny::algo::TypeClusterObject<std::string, UISource>
		{
			private:
			protected:
				UISource(std::string _id, UIInterface * _ui);
			public:
				virtual ~UISource(void);
				virtual UIInformation getUIInfo(void) = 0;
		};

		typedef tiny::algo::TypeCluster<std::string, UISource> UISourceTC;

		/** The UIInterface allows non-UI objects to register themselves with the UI,
		  * after which the UI can retrieve the information it needs directly from them. */
		class UIInterface
		{
			private:
				friend class UISource;
				UISourceTC & getSourceTypeCluster(void) { return sources; }
			protected:
				UISourceTC sources;

				UIInterface(void);
				~UIInterface(void) {}
			public:
				/** Get UI info from the UISource with identifier '_id'. */
				UIInformation getUIInfo(std::string _id);

				virtual void keyEvent(const SDLKey & k, bool) = 0;

				/** Subscribe the UIListener to input received by the application. The InputSet
				  * must be filled by the UIListener, and specifies which input the UIListener
				  * will result in the UI sending the UIListener an input event. */
				virtual InputSet * subscribe(UIListener *) = 0;
				virtual void unsubscribe(UIListener *) = 0;
				virtual void bump(UIListener *) = 0;
		};
	}
}
