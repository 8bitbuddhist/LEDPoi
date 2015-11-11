/*
	LEDPOI.cpp - Library for controlling wireless LED poi
	Requires RGBArray libraries

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef LEDPOI_H
#define LEDPOI_H

class LEDPoi {
		
	public:
		enum Actions {
			PING,					// {Actions action}
			CHANGE_COLOR,			// {Actions action, byte index, byte r, byte g, byte b}
			GENERATE_COLOR_ARRAY,	// {Actions action, byte size, byte r, byte g, byte b}
			GENERATE_SCALING_COLOR_ARRAY,	// {Actions action, byte array size, byte color1.r, byte color1.g, byte color1.b, byte color2.r, byte color2.g, byte color2.b, bool reverse}
			SET_INTERVAL,			// {Actions action, byte interval}
			SET_MODE,				// {Actions action, byte mode, byte opts}
			SET_PATTERN				// {Actions action, byte pattern_index}
		};
};

#endif // LEDPOI_H
