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
#include <SDL.h> // for SDL_Keycode enum only

#include <tiny/draw/renderable.h>
#include <tiny/algo/typecluster.h>

#include "keys.hpp"

namespace sel { class State; }

namespace strata
{
	namespace intf
	{
		class UIInterface;
		class UIListener;

		/** The InputSet is an object where a set of (keyboard/mouse) input commands can be defined,
		  * via their respective SDL_Keycode symbols. */
		class InputSet
		{
			private:
				std::set<SDL_Keycode> keys; /**< The keys that are registered by the listener. */
			public:
				InputSet(void) {}
				~InputSet(void) {}

				void addKey(SDL_Keycode k)
				{
					keys.emplace(k);
				}

				void addKeySet(std::set<SDL_Keycode> ks)
				{
					keys.insert(ks.begin(), ks.end());
				}

				void clearKeySet(void)
				{
					keys.clear();
				}

				void resetKeySet(std::set<SDL_Keycode> ks)
				{
					clearKeySet();
					addKeySet(ks);
				}

				inline bool isSubscribed(const SDL_Keycode &k)
				{
					return (keys.count(k) > 0);
				}
		};

		/** The UIListener is a base class to every object that can react to low level user input.
		  * Such listeners can subscribe to certain inputs (namely keyboard and mouse events), which
		  * are only applicable in specific contexts.
		  * Subscription is done through its link to the UI that governs it. The UI contains
		  * an InputInterpreter that processes the input and tracks who is subscribed to what.
		  */
		class UIListener : public tiny::algo::TypeClusterObject<std::string, UIListener>
		{
			private:
				UIInterface * ui; /**< A link to the UI that governs the listener. */
			protected:
				InputSet inputSet; /**< An input set for the UIListener. */
			public:
				UIListener(std::string id, UIInterface * _ui);

				virtual ~UIListener(void);

				/** Signal that a key is pressed down or no longer pressed down. */
				virtual void receiveKeyInput(const SDL_Keycode & k, const SDL_Keymod & m, bool isDown) = 0;

				/** Signal that a mouse event occurred at position (x,y). */
				virtual bool receiveMouseEvent(float x, float y, unsigned int buttons) = 0;

				inline bool keyIsSubscribed(const SDL_Keycode &k) { return inputSet.isSubscribed(k); }
		};

		/** The UIReceiver is a base class to non-UI classes that can receive function calls through
		  * user interaction. They cannot process raw UI input such as button and key presses, which
		  * must first be handled by standard UI elements (e.g. windows). */
		class UIReceiver : public tiny::algo::TypeClusterObject<std::string, UIReceiver>
		{
			private:
				UIInterface * ui; /**< A link to the UI that governs the listener. */
			public:
				UIReceiver(std::string _id, UIInterface * _ui);

				virtual ~UIReceiver(void);

				/** Receive a function call through the UI. Arguments, if any, are provided in 'args'. */
				virtual void receiveUIFunctionCall(std::string args) = 0;
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

		/** A class to hold an atomic UI text fragment, consisting of a piece of text and optionally
		  * a colour. */
		class UITextFragment
		{
			private:
			public:
				std::string text;
				unsigned char red;
				unsigned char green;
				unsigned char blue;
				bool useDefaultColour;

				UITextFragment(std::string _text, bool _useDefaultColour, unsigned char _red = 0,
						unsigned char _green = 0, unsigned char _blue = 0) :
					text(_text), red(_red), green(_green), blue(_blue),
					useDefaultColour(_useDefaultColour)
				{
				}
		};

		/** The UIMessage holds formatted text for display by the UI. It can be generated by any
		  * part of Strata, and it can be supplied to various Window objects that display such messages.
		  * The UIMessage is a more sophisticated form of UIInformation and is intended to replace the
		  * former. */
		class UIMessage
		{
			private:
			public:
				UIMessage(void) {}
				~UIMessage(void) {}

				std::vector<UITextFragment> textFragments;

				void addFragment(std::string _text, bool _useDefaultColour, unsigned char _red = 0,
						unsigned char _green = 0, unsigned char _blue = 0)
				{
					textFragments.push_back( UITextFragment(_text, _useDefaultColour, _red, _green, _blue) );
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
		typedef tiny::algo::TypeCluster<std::string, UIReceiver> UIReceiverTC;
		typedef tiny::algo::TypeCluster<std::string, UIListener> UIListenerTC;

		/** The UIInterface allows non-UI objects to register themselves with the UI,
		  * after which the UI can retrieve the information it needs directly from them. */
		class UIInterface
		{
			private:
				friend class UISource; // to allow using getSourceTypeCluster()
				friend class UIReceiver; // to allow using getReceiverTypeCluster()
				friend class UIListener; // to allow using getListenerTypeCluster()
				UISourceTC & getSourceTypeCluster(void) { return sources; }
				UIReceiverTC & getReceiverTypeCluster(void) { return receivers; }
				UIListenerTC & getListenerTypeCluster(void) { return listeners; }
			protected:
				UISourceTC sources;
				UIReceiverTC receivers;
				UIListenerTC listeners;

				UIInterface(void);
				~UIInterface(void) {}
			public:
				/** Get UI info from the UISource with identifier '_id'. */
				UIInformation getUIInfo(std::string _id);
				UIReceiver * getUIReceiver(std::string _id);

				virtual void keyEvent(const SDL_Keycode & k, bool) = 0;
				virtual SDL_Keymod getKeyMods(void) const = 0;
				virtual void mouseEvent(float x, float y, unsigned int buttons) = 0;
				virtual void callExternalFunction(std::string receiver, std::string args) = 0;

				virtual void registerLuaFunctions(sel::State & luaState) = 0;

				virtual void logConsoleMessage(const UIMessage & message) = 0;

				virtual void subscribe(UIListener *) = 0;
				virtual void unsubscribe(UIListener *) = 0;
				virtual void bump(UIListener *) = 0;
				virtual void bringToFront(tiny::draw::Renderable *) = 0;
		};
	}
}
