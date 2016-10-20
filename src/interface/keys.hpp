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

#include <SDL.h> // for SDL_Keycode enum only

namespace strata
{
	/** This conversion to upper-level keys is extremely nonportable. Unfortunately, SDL 1.2 (which
	  * the tiny-game-engine basically enforces) does not supply any better.
	  * Different keyboard layouts than mine are thus very poorly supported. Ideally there would be
	  * a way for some configuration (via Lua) to dynamically define the keyboard layout. However,
	  * for now I'll just settle with the non-portability.
	  * This function will return the lowercase SDL_Keycode for a-z characters, or for that matter it will
	  * return the key itself for any SDL_Keycode not specifically redefined in the below statement.
	  */
	inline unsigned char convertSDLinput(const SDL_Keycode & _k, const SDL_Keymod & m)
	{
		unsigned char k = static_cast<unsigned char>(_k);
		if(	   ((m & KMOD_SHIFT) && !(m & KMOD_CAPS))
			|| ((m & KMOD_CAPS) && !(m & KMOD_SHIFT)) )
		{
			if(static_cast<unsigned char>(k) >= 'a' && static_cast<unsigned char>(k) <= 'z')
				k = static_cast<unsigned char>(k) + ('A' - 'a');
			else
			{
				switch(_k)
				{
					case SDLK_BACKQUOTE : k = '~'; break;
					case SDLK_1 : k = '!'; break;
					case SDLK_2 : k = '@'; break;
					case SDLK_3 : k = '#'; break;
					case SDLK_4 : k = '$'; break;
					case SDLK_5 : k = '%'; break;
					case SDLK_6 : k = '^'; break;
					case SDLK_7 : k = '&'; break;
					case SDLK_8 : k = '*'; break;
					case SDLK_9 : k = '('; break;
					case SDLK_0 : k = ')'; break;
					case SDLK_MINUS : k = '_'; break;
					case SDLK_EQUALS : k = '+'; break;
					case SDLK_LEFTBRACKET : k = '{'; break;
					case SDLK_RIGHTBRACKET : k = '}'; break;
					case SDLK_BACKSLASH : k = '|'; break;
					case SDLK_SEMICOLON : k = ':'; break;
					case SDLK_QUOTE : k = '"'; break;
					case SDLK_COMMA : k = '<'; break;
					case SDLK_PERIOD : k = '>'; break;
					case SDLK_SLASH : k = '?'; break;
					default: break;
				}
			}
		}
		return k;
	}

	/** Unite two key sets, creating a set consisting of every key that is in either of the two sets. */
	inline std::set<SDL_Keycode> uniteKeySets(const std::set<SDL_Keycode> &first,
			const std::set<SDL_Keycode> &second)
	{
		std::set<SDL_Keycode> keys = first;
		keys.insert(second.begin(), second.end());
		return keys;
	}

	/** A key set of all alphabetical keys. Does not include the space character (' ') or numbers. */
	inline std::set<SDL_Keycode> keySetAlphabetWithoutSpace(void)
	{
		std::set<SDL_Keycode> keys;
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
	inline std::set<SDL_Keycode> keySetAlphabet(void)
	{
		std::set<SDL_Keycode> keys = keySetAlphabetWithoutSpace();
		keys.emplace(SDLK_SPACE);
		return keys;
	}

	inline std::set<SDL_Keycode> keySetNumbers(void)
	{
		std::set<SDL_Keycode> keys;
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
	inline std::set<SDL_Keycode> keySetAlphanumericWithoutSpace(void)
	{
		std::set<SDL_Keycode> keys = uniteKeySets(keySetAlphabetWithoutSpace(),keySetNumbers());
		return keys;
	}

	/** The key set of all alphabetical keys. Includes a space by default, if spaces are not allowed,
	  * use keySetAllphabetWithoutSpace instead. */
	inline std::set<SDL_Keycode> keySetAlphanumeric(void)
	{
		std::set<SDL_Keycode> keys = uniteKeySets(keySetAlphabet(),keySetNumbers());
		return keys;
	}

	inline std::set<SDL_Keycode> keySetFunctionKeys(void)
	{
		std::set<SDL_Keycode> keys;
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
	inline std::set<SDL_Keycode> keySetPunctuation(void)
	{
		std::set<SDL_Keycode> keys;
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
	inline std::set<SDL_Keycode> keySetSymbolic(void)
	{
		std::set<SDL_Keycode> keys;
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
	inline std::set<SDL_Keycode> keySetSpecial(void)
	{
		std::set<SDL_Keycode> keys;
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

	/** The key set of all alphanumeric characters and punctuation. */
	inline std::set<SDL_Keycode> keySetText(void)
	{
		std::set<SDL_Keycode> keys = uniteKeySets(keySetAlphanumeric(), keySetPunctuation());
		return keys;
	}

	/** The key set of all alphanumeric characters, punctuation and symbolic characters.
	  * Alternatively, the key set of all displayable (text-editing) keys. */
	inline std::set<SDL_Keycode> keySetTextSymbolic(void)
	{
		std::set<SDL_Keycode> keys = uniteKeySets(keySetText(), keySetSymbolic());
		return keys;
	}

	/** The key set of all displayable symbols, joined with text manipulation keys. */
	inline std::set<SDL_Keycode> keySetTextCompleteWithEscape(void)
	{
		std::set<SDL_Keycode> keys = uniteKeySets(keySetTextSymbolic(), keySetSpecial());
		return keys;
	}

	/** The key set of all displayable symbols, joined with text manipulation keys, but without the
	  * Escape key that is used as a special input character that closes active UI elements. */
	inline std::set<SDL_Keycode> keySetTextComplete(void)
	{
		std::set<SDL_Keycode> keys = keySetTextCompleteWithEscape();
		keys.erase(SDLK_ESCAPE);
		return keys;
	}
	/** Arrow keys. */
	inline std::set<SDL_Keycode> keySetArrows(void)
	{
		std::set<SDL_Keycode> keys;
		keys.emplace(SDLK_UP);
		keys.emplace(SDLK_DOWN);
		keys.emplace(SDLK_LEFT);
		keys.emplace(SDLK_RIGHT);
		return keys;
	}

	/** Modifier keys. */
	inline std::set<SDL_Keycode> keySetModifier(void)
	{
		std::set<SDL_Keycode> keys;
		keys.emplace(SDLK_RSHIFT);
		keys.emplace(SDLK_LSHIFT);
		keys.emplace(SDLK_RCTRL);
		keys.emplace(SDLK_LCTRL);
		keys.emplace(SDLK_RALT);
		keys.emplace(SDLK_LALT);
		return keys;
	}

	/** Conversion from chars to SDL_Keycode's. */
	inline SDL_Keycode toSDLKey(std::string k)
	{
		SDL_Keycode key = SDLK_UNKNOWN;
		if(k == "a") key = SDLK_a;
		else if(k == "b") key = SDLK_b;
		else if(k == "c") key = SDLK_c;
		else if(k == "d") key = SDLK_d;
		else if(k == "e") key = SDLK_e;
		else if(k == "f") key = SDLK_f;
		else if(k == "g") key = SDLK_g;
		else if(k == "h") key = SDLK_h;
		else if(k == "i") key = SDLK_i;
		else if(k == "j") key = SDLK_j;
		else if(k == "k") key = SDLK_k;
		else if(k == "l") key = SDLK_l;
		else if(k == "m") key = SDLK_m;
		else if(k == "n") key = SDLK_n;
		else if(k == "o") key = SDLK_o;
		else if(k == "p") key = SDLK_p;
		else if(k == "q") key = SDLK_q;
		else if(k == "r") key = SDLK_r;
		else if(k == "s") key = SDLK_s;
		else if(k == "t") key = SDLK_t;
		else if(k == "u") key = SDLK_u;
		else if(k == "v") key = SDLK_v;
		else if(k == "w") key = SDLK_w;
		else if(k == "x") key = SDLK_x;
		else if(k == "y") key = SDLK_y;
		else if(k == "z") key = SDLK_z;
		else if(k == "0") key = SDLK_0;
		else if(k == "1") key = SDLK_1;
		else if(k == "2") key = SDLK_2;
		else if(k == "3") key = SDLK_3;
		else if(k == "4") key = SDLK_4;
		else if(k == "5") key = SDLK_5;
		else if(k == "6") key = SDLK_6;
		else if(k == "7") key = SDLK_7;
		else if(k == "8") key = SDLK_8;
		else if(k == "9") key = SDLK_9;
		else if(k == "F1") key = SDLK_F1;
		else if(k == "F2") key = SDLK_F2;
		else if(k == "F3") key = SDLK_F3;
		else if(k == "F4") key = SDLK_F4;
		else if(k == "F5") key = SDLK_F5;
		else if(k == "F6") key = SDLK_F6;
		else if(k == "F7") key = SDLK_F7;
		else if(k == "F8") key = SDLK_F8;
		else if(k == "F9") key = SDLK_F9;
		else if(k == "F10") key = SDLK_F10;
		else if(k == "F11") key = SDLK_F11;
		else if(k == "F12") key = SDLK_F12;
		else if(k == "SPACE") key = SDLK_SPACE;
		else if(k == "EXCLAIM") key = SDLK_EXCLAIM;
		else if(k == "QUOTEDBL") key = SDLK_QUOTEDBL;
		else if(k == "QUOTE") key = SDLK_QUOTE;
		else if(k == "LEFTPAREN") key = SDLK_LEFTPAREN;
		else if(k == "RIGHTPAREN") key = SDLK_RIGHTPAREN;
		else if(k == "COMMA") key = SDLK_COMMA;
		else if(k == "MINUS") key = SDLK_MINUS;
		else if(k == "PERIOD") key = SDLK_PERIOD;
		else if(k == "COLON") key = SDLK_COLON;
		else if(k == "SEMICOLON") key = SDLK_SEMICOLON;
		else if(k == "QUESTION") key = SDLK_QUESTION;
		else if(k == "HASH") key = SDLK_HASH;
		else if(k == "DOLLAR") key = SDLK_DOLLAR;
		else if(k == "AMPERSAND") key = SDLK_AMPERSAND;
		else if(k == "ASTERISK") key = SDLK_ASTERISK;
		else if(k == "PLUS") key = SDLK_PLUS;
		else if(k == "SLASH") key = SDLK_SLASH;
		else if(k == "LESS") key = SDLK_LESS;
		else if(k == "EQUALS") key = SDLK_EQUALS;
		else if(k == "GREATER") key = SDLK_GREATER;
		else if(k == "AT") key = SDLK_AT;
		else if(k == "LEFTBRACKET") key = SDLK_LEFTBRACKET;
		else if(k == "RIGHTBRACKET") key = SDLK_RIGHTBRACKET;
		else if(k == "CARET") key = SDLK_CARET;
		else if(k == "UNDERSCORE") key = SDLK_UNDERSCORE;
		else if(k == "BACKQUOTE") key = SDLK_BACKQUOTE;
		else if(k == "BACKSLASH") key = SDLK_BACKSLASH;
		else if(k == "BACKSPACE") key = SDLK_BACKSPACE;
		else if(k == "TAB") key = SDLK_TAB;
		else if(k == "RETURN") key = SDLK_RETURN;
		else if(k == "ESCAPE") key = SDLK_ESCAPE;
		else if(k == "DELETE") key = SDLK_DELETE;
		else if(k == "INSERT") key = SDLK_INSERT;
		else if(k == "HOME") key = SDLK_HOME;
		else if(k == "END") key = SDLK_END;
		else if(k == "PAGEUP") key = SDLK_PAGEUP;
		else if(k == "PAGEDOWN") key = SDLK_PAGEDOWN;
		else if(k == "UP") key = SDLK_UP;
		else if(k == "DOWN") key = SDLK_DOWN;
		else if(k == "LEFT") key = SDLK_LEFT;
		else if(k == "RIGHT") key = SDLK_RIGHT;
		else if(k == "RSHIFT") key = SDLK_RSHIFT;
		else if(k == "LSHIFT") key = SDLK_LSHIFT;
		else if(k == "RCTRL") key = SDLK_RCTRL;
		else if(k == "LCTRL") key = SDLK_LCTRL;
		else if(k == "RALT") key = SDLK_RALT;
		else if(k == "LALT") key = SDLK_LALT;
		// Where applicable, also map character input
		else if(k == " ") key = SDLK_SPACE;
		else if(k == "!") key = SDLK_EXCLAIM;
		else if(k == "\"") key = SDLK_QUOTEDBL;
		else if(k == "'") key = SDLK_QUOTE;
		else if(k == "(") key = SDLK_LEFTPAREN;
		else if(k == ")") key = SDLK_RIGHTPAREN;
		else if(k == ",") key = SDLK_COMMA;
		else if(k == "-") key = SDLK_MINUS;
		else if(k == ".") key = SDLK_PERIOD;
		else if(k == ":") key = SDLK_COLON;
		else if(k == ";") key = SDLK_SEMICOLON;
		else if(k == "?") key = SDLK_QUESTION;
		else if(k == "#") key = SDLK_HASH;
		else if(k == "$") key = SDLK_DOLLAR;
		else if(k == "&") key = SDLK_AMPERSAND;
		else if(k == "*") key = SDLK_ASTERISK;
		else if(k == "+") key = SDLK_PLUS;
		else if(k == "/") key = SDLK_SLASH;
		else if(k == "<") key = SDLK_LESS;
		else if(k == "=") key = SDLK_EQUALS;
		else if(k == ">") key = SDLK_GREATER;
		else if(k == "@") key = SDLK_AT;
		else if(k == "[") key = SDLK_LEFTBRACKET;
		else if(k == "]") key = SDLK_RIGHTBRACKET;
		else if(k == "^") key = SDLK_CARET;
		else if(k == "_") key = SDLK_UNDERSCORE;
		else if(k == "`") key = SDLK_BACKQUOTE;
		else if(k == "\\") key = SDLK_BACKSLASH;
		// Map uppercase characters.
		else if(k == "A") key = SDLK_a;
		else if(k == "B") key = SDLK_b;
		else if(k == "C") key = SDLK_c;
		else if(k == "D") key = SDLK_d;
		else if(k == "E") key = SDLK_e;
		else if(k == "F") key = SDLK_f;
		else if(k == "G") key = SDLK_g;
		else if(k == "H") key = SDLK_h;
		else if(k == "I") key = SDLK_i;
		else if(k == "J") key = SDLK_j;
		else if(k == "K") key = SDLK_k;
		else if(k == "L") key = SDLK_l;
		else if(k == "M") key = SDLK_m;
		else if(k == "N") key = SDLK_n;
		else if(k == "O") key = SDLK_o;
		else if(k == "P") key = SDLK_p;
		else if(k == "Q") key = SDLK_q;
		else if(k == "R") key = SDLK_r;
		else if(k == "S") key = SDLK_s;
		else if(k == "T") key = SDLK_t;
		else if(k == "U") key = SDLK_u;
		else if(k == "V") key = SDLK_v;
		else if(k == "W") key = SDLK_w;
		else if(k == "X") key = SDLK_x;
		else if(k == "Y") key = SDLK_y;
		else if(k == "Z") key = SDLK_z;
		else std::cout << " toSDLKey() : No key defined for string '"<<k<<"'!"<<std::endl;
		return key;
	}
} // end namespace strata
