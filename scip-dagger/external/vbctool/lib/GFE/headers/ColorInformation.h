//
//
//
#ifndef COLORINFORMATION_H
#define COLORINFORMATION_H

#include "GFE.inc"
#include "ColorContext.h"   // only for the Variables MAXCHANGERS and MAXCOLORS


class ColorInformation
{
   private:
   
      Display     *_display;
      Window	   _window;

      int	   _screenNum;
      int	   _defaultDepth;
      Visual	  *_defaultVisual;
      Colormap	   _GFEColormap;

      char	       **_colorNames;
      int	       *_red;
      int	       *_green;
      int	       *_blue;
      int	       _numberOfColors;

      unsigned long	_pixels[MAXCHANGERS + 4];

   protected:

   public:

      ColorInformation(Display*, Window);
      ~ColorInformation();

      int numberOfColors() { return _numberOfColors; }
      char **colorNames()  { return _colorNames; }
      Colormap *GFEColormap() { return &_GFEColormap; }

      unsigned long allocateColor(char*, int);
      void   freeColor(unsigned long);

      int   colorNamePosition(char*);

      Boolean isBlackWhite() { if (_defaultDepth == 1) return True; else return False; }
      int red(int i);
      int green(int i);
      int blue(int i);
};

#endif

