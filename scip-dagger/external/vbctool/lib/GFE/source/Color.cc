/////////////////////////////////////////////////////////////////////////
//
//
//    Filename :  Color.cc
//
//    Version  :  30.11.93
//
//    Author   :  Martin Diehl
//
//    Language :  C++
//
//    Purpose  :  An abstract color class.
// 
/////////////////////////////////////////////////////////////////////////
#include "DrawArea.h" // which includes Color.h

Color::Color(char *customName)
	 {
	    _valuesChanged = True;
	    
	    _locked = False;
	    _display = theDrawArea->display();
	    _customName = strdup(customName);
	    
	    _red = _green = _blue = UNDEFINED_RGBVALUE;
	    _name = NULL;
	    _pixelValue = 0;
	    
	    if (!_colorInformation)
	    {
	       _colorInformation = theDrawArea->colorInformation();
	       _colorInformationIndex = -1;
	       int screenNumber = DefaultScreen(_display);
	       colorMap = DefaultColormap(_display, screenNumber);
	    }
	 }


Color::Color(unsigned long pV)
      {
	 _valuesChanged = True;
	 
	 _locked = False;
	 _display = theDrawArea->display();
	 
	 _red = _green = _blue = UNDEFINED_RGBVALUE;
	 _name = NULL;
	 _customName = NULL;
	 _pixelValue	= pV;	 

	 if (!_colorInformation)
	 {
	    _colorInformation = theDrawArea->colorInformation();
	    _colorInformationIndex = -1;
	    int screenNumber = DefaultScreen(_display);
	    colorMap = DefaultColormap(_display, screenNumber);
	 }
      }

int Color::red()
{
   if (_colorInformation->isBlackWhite())
   {
      int v;
      if ((v = _colorInformation->red(_colorInformationIndex)) >= 0)
	 return v * 257; // because v is between 0 and 255, expected 0 and 65535
   }
   return _red;
}

int Color::green()
{
   if (_colorInformation->isBlackWhite())
   {
      int v;
      if ((v = _colorInformation->green(_colorInformationIndex)) >= 0)
	 return v * 257; // see above
   }
   return _green;
}

int Color::blue()
{
   if (_colorInformation->isBlackWhite())
   {
      int v;
      if ((v = _colorInformation->blue(_colorInformationIndex)) >= 0)
	 return v * 257; // see above
   }
   return _blue;
}

