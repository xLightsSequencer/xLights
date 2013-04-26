/***************************************************************
 * Name:      RgbEffects.cpp
 * Purpose:   Implements RGB effects
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-12-23
 * Copyright: 2012 by Matt Brown
 * License:
 This file is part of xLights.

 xLights is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 xLights is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with xLights.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/
#include <cmath>
#include "RgbEffects.h"


void RgbEffects::RenderFireworks(int Number_Explosions,int Count,float Velocity)
{
	int idxFlakes=0;
	int i=0,x,y,mod100;
	int x25,x75,y25,y75;
	const int maxFlakes = 1000;
	//float velocity = 3.5;
	int startX;
	int startY;
	wxImage::HSVValue hsv;
	wxColour color,rgbcolor;

	mod100 = state%((101-Number_Explosions)*10);
	if(mod100 == 0)
	{

		x25=(int)BufferWi*0.25;
		x75=(int)BufferWi*0.75;
		y25=(int)BufferHt*0.25;
		y75=(int)BufferHt*0.75;
		startX=(int)BufferWi/2;
		startY=(int)BufferHt/2;
		startX = x25 + rand()%(x75-x25);
		startY = y25 + rand()%(y75-y25);
		// turn off all bursts
		for(i=0;i<maxFlakes;i++)
		{
			fireworkBursts[i]._bActive = false;
		}
		// Create new bursts
		for(i=0;i<Count;i++)
		{
			do
			{
				idxFlakes = (idxFlakes + 1) % maxFlakes;
			}
			while (fireworkBursts[idxFlakes]._bActive);
			fireworkBursts[idxFlakes].Reset(startX, startY, true,Velocity);
		}
	}
	else
	{
		for (i=0; i<maxFlakes; i++)
		{
			// ... active flakes:
			if (fireworkBursts[i]._bActive)
			{
				// Update position
				fireworkBursts[i]._x += fireworkBursts[i]._dx;
				fireworkBursts[i]._y += (-fireworkBursts[i]._dy - fireworkBursts[i]._cycles*fireworkBursts[i]._cycles/10000000.0);
				// If this flake run for more than maxCycle, time to switch it off
				fireworkBursts[i]._cycles+=20;
				if (1000 == fireworkBursts[i]._cycles) // if (10000 == fireworkBursts[i]._cycles)
				{
					fireworkBursts[i]._bActive = false;
					continue;
				}
				// If this flake hit the earth, time to switch it off
				if (fireworkBursts[i]._y>=BufferHt)
				{
					fireworkBursts[i]._bActive = false;
					continue;
				}
				// Draw the flake, if its X-pos is within frame
				if (fireworkBursts[i]._x>=0. && fireworkBursts[i]._x<BufferWi)
				{
					// But only if it is "under" the roof!
					if (fireworkBursts[i]._y>=0.)
					{
						// sean we need to set color here
					}
				}
				else
				{
					// otherwise it just got outside the valid X-pos, so switch it off
					fireworkBursts[i]._bActive = false;
					continue;
				}
			}
		}
	}

	// Clear all Pixels
	color = wxColour(0,0,0);
	Color2HSV(color,hsv);
	for(y=0; y < BufferHt; y++)
	{
		for(x=0; x < BufferWi; x++)
		{
			SetPixel(x,y,hsv);
		}
	}
	// Draw bursts with fixed color

	if(state%300<=300) rgbcolor = wxColour(255,0,255);
	if(state%300<=200) rgbcolor = wxColour(255,255,0);
	if(state%300<=100) rgbcolor = wxColour(255,0,0);

	Color2HSV(rgbcolor,hsv);
	for(i=0; i < 1000; i++)
	{
		if(fireworkBursts[i]._bActive == true)
		{
			SetPixel(fireworkBursts[i]._x,fireworkBursts[i]._y,hsv);
		}
	}
}
