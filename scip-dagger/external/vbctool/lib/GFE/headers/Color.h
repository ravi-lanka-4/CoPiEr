/////////////////////////////////////////////////////////////////////////
//
//
//    Filename :  Color.h
//
//    Version  :  16.02.94
//
//    Author   :  Martin Diehl
//
//    Language :  C++
//
//    Purpose  :  An abstract color class.
// 
//    
/////////////////////////////////////////////////////////////////////////
#ifndef COLOR_H
#define COLOR_H

#include "GFE.inc"
#include "ColorInformation.h"

#include <X11/X.h>

#define UNDEFINED_RGBVALUE -1

class ColorInformation;

class Color
{
   friend class Context;

   private:

      Boolean _locked;
      
   protected:

      Boolean _valuesChanged; // true, if any value is changed

      // this is the Pointer to the ColorInformation-Object
      // used to specify the RGB-Values for Postscript-Output
      // if a black/white-Monitor is used

      static ColorInformation *_colorInformation;
      int _colorInformationIndex;

      // RGB values of the color

      int			_red;
      int			_green;
      int			_blue;


      // Name from the X database of colors
      char			*_name;	

      // Name given from the user of the class
      // for ex. Background or Nodes
      char			*_customName;

      // Actual pixel value that can be used is 
      // a Xlib call
      unsigned long _pixelValue;

      // Display this color will be used with
      Display  *_display;

      // give the DrawArea class the possibility to set the pixelValue
      void setPixelValue(unsigned long pV){ _pixelValue = pV; _valuesChanged = True; }

      // the color map all colors should be allocated from
      static Colormap colorMap;

      // Derived classes should add the actual allocation of the
      // color cell to its constructor after calling this
      // basic constructor, which is protected to prevent
      // direct instantiation.

      // determines the new Index for the desired Color in the Color-List
      // of ColorInformation

      void setColorInformationIndex()
      {
	 _colorInformationIndex = _colorInformation->colorNamePosition(_name) - 1;
      }

      Color(char *customName);

   public:


      // Second constructor which creates a color object only by a given
      // pixel value, without name or RGB-Values
      Color(unsigned long pV);

      int red();
      int green();
      int blue();

      unsigned long pixelValue() {return _pixelValue;}

      Boolean isLocked() {return _locked;}

      void lock()    {_locked = True;}
      void unlock()  {_locked = False;}

      char *const customName(){ return _customName;}
      char *const xName()     { return _name; }

      ~Color()
      {
	 kill (_name);
	 kill (_customName);
      }

      void applyToGC(GC &gc)
      {
	 _valuesChanged = False;
	 
	 XSetLineAttributes(_display, gc,
			    0,
			    LineSolid,
			    CapButt,
			    JoinMiter);

	 XSetForeground(_display, gc,
			pixelValue());
      }

      Boolean valuesChanged() { return _valuesChanged; }

}; // Color

#endif
