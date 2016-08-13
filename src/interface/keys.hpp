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

#include <set>

#include <SDL.h> // for SDLKey enum only

namespace strata
{
	/** Unite two key sets, creating a set consisting of every key that is in either of the two sets. */
	inline std::set<SDLKey> uniteKeySets(const std::set<SDLKey> &first, const std::set<SDLKey> &second)
	{
		std::set<SDLKey> keys = first;
		keys.insert(second.begin(), second.end());
		return keys;
	}

	/** A key set of all alphabetical keys. Does not include the space character (' ') or numbers. */
	inline std::set<SDLKey> keySetAlphabetWithoutSpace(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_a);
		keys.emplace(SDLK_b);
		keys.emplace(SDLK_c);
		keys.emplace(SDLK_d);
		keys.emplace(SDLK_e);
		keys.emplace(SDLK_f);
		keys.emplace(SDLK_g);
		keys.emplace(SDLK_h);
		keys.emplace(SDLK_i);
		keys.emplace(SDLK_j);
		keys.emplace(SDLK_k);
		keys.emplace(SDLK_l);
		keys.emplace(SDLK_m);
		keys.emplace(SDLK_n);
		keys.emplace(SDLK_o);
		keys.emplace(SDLK_p);
		keys.emplace(SDLK_q);
		keys.emplace(SDLK_r);
		keys.emplace(SDLK_s);
		keys.emplace(SDLK_t);
		keys.emplace(SDLK_u);
		keys.emplace(SDLK_v);
		keys.emplace(SDLK_w);
		keys.emplace(SDLK_x);
		keys.emplace(SDLK_y);
		keys.emplace(SDLK_z);
		return keys;
	}

	/** The key set of all alphabetical keys. Includes a space by default, if spaces are not allowed,
	  * use keySetAllphabetWithoutSpace instead. */
	inline std::set<SDLKey> keySetAlphabet(void)
	{
		std::set<SDLKey> keys = keySetAlphabetWithoutSpace();
		keys.emplace(SDLK_SPACE);
		return keys;
	}

	inline std::set<SDLKey> keySetNumbers(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_0);
		keys.emplace(SDLK_1);
		keys.emplace(SDLK_2);
		keys.emplace(SDLK_3);
		keys.emplace(SDLK_4);
		keys.emplace(SDLK_5);
		keys.emplace(SDLK_6);
		keys.emplace(SDLK_7);
		keys.emplace(SDLK_8);
		keys.emplace(SDLK_9);
		return keys;
	}

	/** The key set of all alphabetical and numerical keys, without spaces. */
	inline std::set<SDLKey> keySetAlphanumericWithoutSpace(void)
	{
		std::set<SDLKey> keys = uniteKeySets(keySetAlphabetWithoutSpace(),keySetNumbers());
		return keys;
	}

	/** The key set of all alphabetical keys. Includes a space by default, if spaces are not allowed,
	  * use keySetAllphabetWithoutSpace instead. */
	inline std::set<SDLKey> keySetAlphanumeric(void)
	{
		std::set<SDLKey> keys = uniteKeySets(keySetAlphabet(),keySetNumbers());
		return keys;
	}

	inline std::set<SDLKey> keySetFunctionKeys(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_F1);
		keys.emplace(SDLK_F2);
		keys.emplace(SDLK_F3);
		keys.emplace(SDLK_F4);
		keys.emplace(SDLK_F5);
		keys.emplace(SDLK_F6);
		keys.emplace(SDLK_F7);
		keys.emplace(SDLK_F8);
		keys.emplace(SDLK_F9);
		keys.emplace(SDLK_F10);
		keys.emplace(SDLK_F11);
		keys.emplace(SDLK_F12);
		return keys;
	}

	/** Punctuation characters, seen in normal text usage. */
	inline std::set<SDLKey> keySetPunctuation(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_EXCLAIM);
		keys.emplace(SDLK_QUOTEDBL);
		keys.emplace(SDLK_QUOTE);
		keys.emplace(SDLK_LEFTPAREN);
		keys.emplace(SDLK_RIGHTPAREN);
		keys.emplace(SDLK_COMMA);
		keys.emplace(SDLK_MINUS);
		keys.emplace(SDLK_PERIOD);
		keys.emplace(SDLK_COLON);
		keys.emplace(SDLK_SEMICOLON);
		keys.emplace(SDLK_QUESTION);
		return keys;
	}

	/** Symbolic stuff, i.e. keys that are not punctuation or alphanumeric,
	  * but that do have a typical written form. */
	inline std::set<SDLKey> keySetSymbolic(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_HASH);
		keys.emplace(SDLK_DOLLAR);
		keys.emplace(SDLK_AMPERSAND);
		keys.emplace(SDLK_ASTERISK);
		keys.emplace(SDLK_PLUS);
		keys.emplace(SDLK_SLASH);
		keys.emplace(SDLK_LESS);
		keys.emplace(SDLK_EQUALS);
		keys.emplace(SDLK_GREATER);
		keys.emplace(SDLK_AT);
		keys.emplace(SDLK_LEFTBRACKET);
		keys.emplace(SDLK_RIGHTBRACKET);
		keys.emplace(SDLK_CARET);
		keys.emplace(SDLK_UNDERSCORE);
		keys.emplace(SDLK_BACKQUOTE);
		keys.emplace(SDLK_BACKSLASH);
		return keys;
	}

	/** Special input that is used when inputting text but in unusual ways. */
	inline std::set<SDLKey> keySetSpecial(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_BACKSPACE);
		keys.emplace(SDLK_TAB);
		keys.emplace(SDLK_RETURN);
		keys.emplace(SDLK_ESCAPE);
		keys.emplace(SDLK_DELETE);
		keys.emplace(SDLK_INSERT);
		keys.emplace(SDLK_HOME);
		keys.emplace(SDLK_END);
		keys.emplace(SDLK_PAGEUP);
		keys.emplace(SDLK_PAGEDOWN);
		return keys;
	}

	/** Arrow keys. */
	inline std::set<SDLKey> keySetArrows(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_UP);
		keys.emplace(SDLK_DOWN);
		keys.emplace(SDLK_LEFT);
		keys.emplace(SDLK_RIGHT);
		return keys;
	}

	/** Modifier keys. */
	inline std::set<SDLKey> keySetModifier(void)
	{
		std::set<SDLKey> keys;
		keys.emplace(SDLK_RSHIFT);
		keys.emplace(SDLK_LSHIFT);
		keys.emplace(SDLK_RCTRL);
		keys.emplace(SDLK_LCTRL);
		keys.emplace(SDLK_RALT);
		keys.emplace(SDLK_LALT);
		return keys;
	}
} // end namespace strata