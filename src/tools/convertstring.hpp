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

#include <string>
#include <sstream>

namespace strata
{
	namespace tool
	{
		/** A function for converting a string to another basic type. */
		template <typename T>
		T convertFromString(const std::string &s)
		{
			std::stringstream ss(s);
			T val;
			ss >> val;
			return val;
		}

		/** A function for converting another basic type to a string. */
		template <typename T>
		std::string convertToString(T val)
		{
			std::stringstream ss;
			ss << val;
			return ss.str();
		}

		/** A function for converting another basic type to a string, using
		  * a delimiter for large numbers. */
		template <typename T>
		std::string convertToStringDelimited(T val)
		{
			std::string str = convertToString<T>(val);
			if(str.length()>3)
			{
				while(str.find_first_of("., ")>3)
				{
					str.insert(std::min(str.length(),str.find_first_of("., "))-3,1,' ');
				}
			}
			return str;
		}

		template <typename T>
		T adjustToBounds(const std::string &s, T min, T max, bool printAdjustment = false)
		{
			T val = convertFromString<T>(s);
			if(val < min)
			{
				if(printAdjustment)
					std::cout << " adjustToBounds() : Increase "<<s<<" to "<<min<<"."<<std::endl;
				val = min;
			}
			if(!(val <= max))
			{
				if(printAdjustment)
					std::cout << " adjustToBounds() : Decrease "<<s<<" to "<<max<<"."<<std::endl;
				val = max;
			}
			return val;
		}

		inline float toFloat(const std::string &s) { return convertFromString<float>(s); }
		inline int toSignedInteger(const std::string &s) { return convertFromString<int>(s); }
		inline unsigned int toUnsignedInteger(const std::string &s) { return convertFromString<unsigned int>(s); }
		inline unsigned char toUnsignedChar(const std::string &s) { return convertFromString<unsigned char>(s); }
		inline bool toBoolean(const std::string &s) { return (s == "true" || s == "True" || s == "1"); }

		/** Convert string to float, and ensure value is within bounds. */
		inline float toFloatBounded(const std::string &s, float min, float max,
				bool printAdjustment = false)
		{
			return adjustToBounds<float>(s, min, max, printAdjustment);
		}

		/** Convert string to int, and ensure value is within bounds. */
		inline int toSignedIntegerBounded(const std::string &s, int min, int max,
				bool printAdjustment = false)
		{
			return adjustToBounds<int>(s, min, max, printAdjustment);
		}

		/** Convert string to unsigned int, and ensure value is within bounds. */
		inline unsigned int toUnsignedIntegerBounded(const std::string &s, unsigned int min, unsigned int max,
				bool printAdjustment = false)
		{
			return adjustToBounds<unsigned int>(s, min, max, printAdjustment);
		}
	}
}
