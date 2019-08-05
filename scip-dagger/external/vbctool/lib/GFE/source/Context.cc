/////////////////////////////////////////////////////////////////////////
//
// Filename :  Context.cc
//
// Version  :  08.02.94
//
// Author   :  Martin Diehl & Joachim Kupke
//
// Language : 	C++ 
//
// Purpose  :  A template class that represent properties graphic
//	       primitives. It is a wrap around the X11 Graphic Context.
//
////////////////////////////////////////////////////////////////////////   

#include "GFEResourceManager.h"
#include "Context.h"
#include "GFE.inc"
#include "Postscript.h"
#include "GFE_C-Code.h"


void Context::loadFontName(char *shortName)
{
   kill (_shortName);
   kill (_fontName);
   kill (_psName);
   kill (_scalableName);
   
   _shortName = strdup(shortName);

   _oldlen = -1;

   GFEResourceManager resource(FONTFILENAME);
   _fontName = resource.getstring(_shortName);

   if (!_fontName)
      fprintf(stderr, "Cannot find Font associated with '%s'\n", _shortName);
   else
   {
      _fontName = strdup(_fontName);
      
      char sname[300];
      strcpy (sname, shortName);
      strcat (sname, ".ps");
      _psName = resource.getstring(sname);
      if (_psName) _psName = strdup(_psName);
	    
      int h;
      char fn[280];
      strcpy(fn, _fontName);
      
      strcat (fn, "*0-0*");


      char **FontList = XListFonts(_display, fn, 1, &h);
	 
      if (h)
	 _scalableName = strdup(FontList[0]);
      else
	 _scalableName = NULL;
	    
      XFreeFontNames(FontList);

   }
}


Status Context::calculateFontSize(const char *text, int &x, int &y, int &w, int &h, int pos,
				  int factor, int len, float tolerance)
{
   int toleranceWidth = w + round(((float) w) * tolerance / 100.0);
   Boolean notTheSameRect = False;

   // set Tolerance-values
   if (_oldh > h+1 || _oldh < h-1 || _oldw > w+1 || _oldw < w-1)
      notTheSameRect = True;
   
   if(_fontName)
   {
      if (_oldlen != len || notTheSameRect)
      {
	 _st = ERROR;
 	 _oldw = w;
	 _oldh = h;
	 _oldlen = len;
	 
	 int size;
	 int font;

	 if (theDrawArea->postscript()->printFlag())
	    len = round(len * 0.9);

	 if (w<4) return _st;
	 size = (int) (h * 7);
	 if ((0.5 * h * len) > w)
	    size = (w / len) * 14;

	 if (size < 60)
	    return _st;

	 if (size > 1500)
	    size = 1500;

	 if (size < 240)
	 {
	    size = ( (int) (size/10)) * 10;
	    char cSize[7];

	    do
	    {
	       sprintf(cSize, "%d", size);
	       strcat (cSize, "*");

	       char fn[280];
	       strcpy(fn, _fontName);

	       strcat (fn, "*-");
	       strcat (fn, cSize);
	    
	       if (_fontInfo)
	       {
		  XFreeFont(_display, _fontInfo);
		  _fontInfo = NULL;
	       }

	       _fontInfo = XLoadQueryFont(_display, fn);

	       size -= 10;
	       if (size < 60) return _st;

	    }
	    while (!_fontInfo ||
		  ((_fontInfo->ascent) + (_fontInfo->descent) > h) ||
		  (XTextWidth(_fontInfo, text, len) > toleranceWidth));
	 }
	 else
	 {
	    if (!_scalableName) return _st;
	 
	    while (TRUE)
	    {
	       if (_fontInfo)
	       {
		  XFreeFont(_display, _fontInfo);
		  _fontInfo = NULL;
	       }
	       _fontInfo = (XFontStruct *)LoadQueryScalableFont(_display, 0, _scalableName, size);
	       if (!_fontInfo)
	       {
		  fprintf(stderr, "ERROR: font %s not found!\n", _fontName);
		  return _st;
	       }

	       if (((_fontInfo->ascent) + (_fontInfo->descent) <= h) &&
		 (XTextWidth(_fontInfo, text, len) <= toleranceWidth))
	       break;
	       size = (int) (size * 0.9);
	    }
	 }
      
	 _textheight = _fontInfo->ascent + _fontInfo->descent;
	 _st = SUCCESS;
      }
      if (_st == SUCCESS)
      {
	 y -= (factor*descent());
	 _textwidth  = XTextWidth(_fontInfo, text, strlen(text));
      }

      return _st;
   }
   else return (_st = ERROR);
}

Status Context::applyFontToGC(GC &gc, const char *text, int &x, int &y, int &w, int &h, int pos,
			      int factor, int len, float tolerance)
{
   if (calculateFontSize(text, x, y, w, h, pos, factor, len, tolerance) == ERROR)
      return _st;

   XSetFont(_display, gc, _fontInfo->fid);

   switch (pos)
   {
       case 1: break;
       case 5: y -= (factor*(h - _textheight)/2);
       case 2: x += (w - _textwidth)/2;
	       break;
       case 6: y -= (factor*(h - _textheight)/2);
       case 3: x += (w - _textwidth);
	       break;
       case 7: y -= (factor*(h - _textheight));
	       break;
       case 4: y -= (factor*(h - _textheight)/2);
	       break;
       case 8: y -= (factor*(h - _textheight));
	       x += (w - _textwidth)/2;
	       break;
       case 9: y -= (factor*(h - _textheight));
	       x += (w - _textwidth);
   }

   return _st;
}

