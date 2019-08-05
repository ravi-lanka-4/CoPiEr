
#include "ColorInformation.h"
#include "GFEResourceManager.h"


ColorInformation::ColorInformation(Display *display, Window window)
{
   _display = display;
   _window  = window;

   _screenNum = DefaultScreen(display);
   _defaultVisual = DefaultVisual(_display, _screenNum);
   _defaultDepth  = DefaultDepth (_display, _screenNum);

//   _GFEColormap = XCreateColormap(_display, _window, _defaultVisual, AllocNone);
   _GFEColormap = DefaultColormap(_display, _screenNum);

// allocate one read/write Colorcell for displaying all the Colors in the Color-chooser

   unsigned long plane_mask[1];

   XAllocColorCells(_display, _GFEColormap, False, plane_mask, 0, _pixels, MAXCHANGERS + 4);


/*
// read the aviable Color-Names

   if (_defaultDepth == 1) // then you have a black/white Monitor
   {
      _numberOfColors = 2;
      _colorNames = new char*[2];
      _colorNames[0] = "white";
      _colorNames[1] = "black";
      return;
   }
*/

   GFEResourceManager	rgbFile(RGBRESOURCE);

   char *c;
   int r, g, b;
   int rOld = -1, gOld = -1, bOld = -1;

   int i = 0;

   while (c=rgbFile.nextRgbName(r, g, b))
      //if (r == rOld && g == gOld && b == bOld) continue;
      //else
      {
	 i++;
	 if (i == MAXCOLORS) break;
	 rOld = r, gOld = g, bOld = b;
      }

   _numberOfColors = i;
   rgbFile.rewindFile();
   i = 0;
   _colorNames = new char*[_numberOfColors];
   
   _red = new int[_numberOfColors];
   _green = new int[_numberOfColors];
   _blue = new int[_numberOfColors];

   rOld = -1, gOld = -1, bOld = -1;

   while (c=rgbFile.nextRgbName( r, g, b))
      //if (r == rOld && g == gOld && b == bOld) continue;
      //else
      {
	 if (i == _numberOfColors) break;
	       
	 _colorNames[i] = strdup(c);
	 _red[i] = r;
	 _green[i] = g;
	 _blue[i++] = b;

	 if (i == MAXCOLORS) break;
	       
	 rOld = r, gOld = g, bOld = b;
      }
}

ColorInformation::~ColorInformation()
{
   for (int j = 0; j<_numberOfColors; j++)
      delete _colorNames[j];

   delete _colorNames;
   delete _red;
   delete _green;
   delete _blue;

   XFreeColormap(_display, _GFEColormap);
}

unsigned long ColorInformation::allocateColor(char *name, int nr)
{
   if (_defaultDepth != 1) // not a black/white-monitor
   {
       if (_defaultDepth == 8) {
//   XColor value, dummy;

	   //XAllocNamedColor(_display, _GFEColormap, name, &value, &dummy);

	   XStoreNamedColor(_display, _GFEColormap, name, _pixels[nr], (DoRed | DoGreen | DoBlue));

//   value.pixel = _pixels[nr];

	   return _pixels[nr];
       }
       else {
	   XColor colorcell_def;
	   XColor rgb_db_def;
	   
	   XAllocNamedColor(_display, _GFEColormap, name, &colorcell_def, &rgb_db_def);
	   
	   return colorcell_def.pixel;
       }
   }
   else
   {
      XColor colorcell_def;
      XColor rgb_db_def;
      
      XAllocNamedColor(_display, _GFEColormap, name, &colorcell_def, &rgb_db_def);

      return colorcell_def.pixel;
   }
   
}

void ColorInformation::freeColor(unsigned long pixel)
{
   if (_defaultDepth != 1) // not a black/white-monitor
   {
      unsigned long pixelArray[1];

      pixelArray[0] = pixel;

      XFreeColors(_display, _GFEColormap, pixelArray, 1, 8);
   }

// need not to be freed, because you get pV 0 and pV 1 with a black/white-Monitor
/*   else
   {
      unsigned long pixels[1];
      pixels[0] = pixel;

      XFreeColors(_display, _GFEColormap, pixels, 1, 0);
   }
*/
}

int ColorInformation::colorNamePosition(char *name)
{
   if (!name) return 0;
   
   int i = 0;
   for (; i<_numberOfColors; i++)
      if (!strcmp(_colorNames[i], name)) break;

   if (i == _numberOfColors) return 0;
   return ++i;
}

int ColorInformation::red (int i)
{
   if (i < 0 || i >= _numberOfColors) return -1;
   return _red[i];
}

int ColorInformation::green (int i)
{
   if (i < 0 || i >= _numberOfColors) return -1;
   return _green[i];
}

int ColorInformation::blue (int i)
{
   if (i < 0 || i >= _numberOfColors) return -1;
   return _blue[i];
}

