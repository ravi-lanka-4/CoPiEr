////////////////////////////////////////////////////////////////////////
//    Filename :  ROColor.h
//
//    Version  :  16.02.94
//
//    Author   :  Martin Diehl
//
//    Language :  C++ 
//
//    Purpose  :  Read-Only Color.
//		  A color that allocates a read-only color cell,
//		  derived from Color.h.
//
////////////////////////////////////////////////////////////////////////
#include "GFE.inc"
#include "Application.h"
#ifndef ROCOLOR_H
#define ROCOLOR_H

#include "Color.h"

class ROColor : public Color
{
   public:

   ROColor (char *name, char *customName) : Color (customName)
   {
      if (!name) name = "black";
      _name = strdup(name);
      setColorInformationIndex();

      // Allocate actual color
      XColor   actColor;
      XColor   RGBValues;
      XAllocNamedColor(_display, colorMap, _name,
		       &actColor, &RGBValues);
     
      // pass Pixel Value to Color Data Structur
      _pixelValue = actColor.pixel;
      _red	  = (int) actColor.red;
      _green	  = (int) actColor.green;
      _blue	  = (int) actColor.blue;
   }

   ~ROColor()
   {
      unsigned long pixels[1];
      pixels[0] = _pixelValue;
      
      XFreeColors(_display, colorMap, pixels, 1, 0);
   }

   void changeColor(char *name)
   {
      _valuesChanged = True;
      
      kill (_name);
      _name = strdup(name);
      setColorInformationIndex();
      
      unsigned long pixels[1];
      pixels[0] = _pixelValue;
      XFreeColors(_display, colorMap, pixels, 1, 0);

      // Allocate actual color
      XColor   actColor;
      XColor   RGBValues;
      XAllocNamedColor(_display, colorMap, _name,
		       &actColor, &RGBValues);
     
      // pass Pixel Value to Color Data Structur
      _pixelValue = actColor.pixel;
      _red	  = (int) actColor.red;
      _green	  = (int) actColor.green;
      _blue	  = (int) actColor.blue;
   }

   void allocateNewColor(char *name)
   {
      _valuesChanged = True;
      
      kill (_name);
      _name = strdup(name);
      setColorInformationIndex();
      
      // Allocate actual color
      XColor   actColor;
      XColor   RGBValues;
      XAllocNamedColor(_display, colorMap, _name,
		       &actColor, &RGBValues);
     
      // pass Pixel Value to Color Data Structur
      _pixelValue = actColor.pixel;
      _red	  = (int) actColor.red;
      _green	  = (int) actColor.green;
      _blue	  = (int) actColor.blue;
   }
};

#endif
