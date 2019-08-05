/////////////////////////////////////////////////////////////////////////
//
// Filename :  Context.h
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
#ifndef CONTEXT_H
#define CONTEXT_H

#include "GFE.inc"
#include "DrawArea.h"
#include "ROColor.h"

// Default values for a Line properties
#define	DEF_LINE_WIDTH		0
#define	DEF_LINE_STYLE		LineSolid
#define	DEF_CAP_STYLE		CapButt
#define	DEF_JOIN_STYLE		JoinMiter

class Context
{
   private:
   
      Boolean  _locked;
      Boolean  _valuesChanged;
      Boolean  _fontOnly;

      unsigned long _oldPixelValue;

      
      Display  *_display;
      char     *_customName;

     // Line properties
        CoordType   _lineWidth;
        int _lineStyle;
        int _capStyle;
        int _joinStyle;

	CoordType    _maxLineWidth; // for the Displaying the Linewidth

     // Fill properties


     // Color

        ROColor          *_color;
        Context		 *_defaultContext;	// Points to an instance of the
						// class that holds the appropriate
						// default values.

     // Fonts

	XFontStruct	*_fontInfo;
	char		*_shortName;
	char		*_fontName;
	char		*_scalableName;
	char		*_psName;
	int		_screenNum;
	int		_textwidth;
	int		_textheight;
	Status		_st;
	int		_oldlen;
	int		_oldw;
	int		_oldh;
	

   public:

      void loadFontName(char*);
      Status calculateFontSize(const char*, int &x, int &y, int &w, int &h, int, int, int, float);
      Status applyFontToGC(GC &gc, const char*, int &x, int &y, int &w, int &h, int, int, int, float);

      // Inline Constructor and Destructor

      int oldLen() { return _oldlen; }

      Context(Display *display, Context *dC, char *colorName,
	      char *customName)
      {
	 _valuesChanged = True;
	 _fontOnly = False;
	 
	 _display    = display;
	 _screenNum  = 0;
	 _customName = NULL;

	 _maxLineWidth = minimum(theDrawArea->coordinateRange()->width(),
				 theDrawArea->coordinateRange()->height());

	 _fontInfo = NULL;
	 _fontName = NULL;
	 _scalableName = NULL;
	 _shortName = NULL;
	 _psName = NULL;
	 _textwidth = _textheight = 0;
	 _st = ERROR;
	 _oldlen = _oldw = _oldh = -1;
	 
	 if (customName)
	    _customName = strdup(customName);

	 _defaultContext = dC;
               

	  if (_defaultContext == NULL)         // I.e. THIS is the default context.
          {
	    lock();                            // The default context should be locked
	    _lineWidth     = DEF_LINE_WIDTH;   // because it is only a storage for
	    _lineStyle     = DEF_LINE_STYLE;   // default values.
	    _capStyle      = DEF_CAP_STYLE;
	    _joinStyle     = DEF_JOIN_STYLE;
	    _color         = NULL;
	  }
	 else                                
         {
	    unlock();                 			// the default values.
	    setToDefault();
	    _color = new ROColor(colorName, customName);
	 }
      }// Context Constructor


      ~Context()
      {
	 kill (_customName);
	 if (_shortName)
	 {
	    kill (_fontName);
	    kill (_scalableName);
	    kill (_psName);
	    delete _shortName;
	 }
	 kill (_color);

	 if (_fontInfo)
	 {
	    XFreeFont(_display, _fontInfo);
	    _fontInfo = NULL;
	 }
      }


      // Simple Inline Functions

      void applyToGC(GC &gc)
      {
	 _valuesChanged = False;
      
	 XSetLineAttributes(_display, gc,
			    theDrawArea->scaleLength(_lineWidth),
			    _lineStyle,
			    _capStyle,
			    _joinStyle);

	 XSetForeground(_display, gc,
			_color->pixelValue());
      }

      CoordType	     lineWidth()    { return _lineWidth;}
      int	     lineStyle()    { return _lineStyle;}
      int	     capStyle()     { return _capStyle;}
      int	     joinStyle()    { return _joinStyle;}

      void setToDefault()
      {
	 _valuesChanged = True;
      
	 _lineWidth     = _defaultContext->lineWidth();
	 _lineStyle     = _defaultContext->lineStyle();
	 _capStyle      = _defaultContext->capStyle();
	 _joinStyle     = _defaultContext->joinStyle();
      }
   
   

      void setLineWidth(CoordType width)
      // Line width can be given in the user defined CoordType
      // it will be scaled with the current scaler of the DrawArea.
      {
	 _valuesChanged = True;
	 _lineWidth = width;
      }

      void setLineStyle(int style)
      {
	 _valuesChanged = True;
	 _lineStyle = style;
      }

      void setCapStyle(int style)
      {
	 _valuesChanged = True;
	 _capStyle = style;
      }

      void setJoinStyle(int style)
      {
	 _valuesChanged = True;
	 _joinStyle = style;
      }

      char *const customName()	 { return _customName;}
      char *const xName()	 { return _color->xName(); }

      char *const fontName()	 { return _fontName; }
      char *const shortName()	 { return _shortName; }
      char *const psName()	 { return _psName; }
   
      void lock()	  { _locked = True;}
      void unlock()	  { _locked = False;}

      Boolean isLocked() {return _locked;}

      int RGBred()   { return _color->red(); }
      int RGBgreen() { return _color->green(); }
      int RGBblue()  { return _color->blue(); }

      int textwidth()	{ return _textwidth; }
      int textheight()	{ return _textheight; }
      int descent()	{ if (_fontInfo) return _fontInfo->descent; else return 0; }
      int fontheight()	{ if (_fontInfo) return _fontInfo->ascent + _fontInfo->descent; else return 0; }

      Status st()	{ return _st; }

      void changeColor(char *name) { _color->changeColor(name); _valuesChanged = True; }
      
      void allocateNewColor(char *name) { _color->allocateNewColor(name); _valuesChanged = True; }
      
      void setMaxLineWidth(CoordType m) { _maxLineWidth = m; }
      CoordType getMaxLineWidth()	{ return _maxLineWidth; }

      Boolean valuesChanged() { return _valuesChanged; }

      Boolean fontOnly() { return _fontOnly; }
      void onlyForFonts() { _fontOnly = True; }
      void forAll() { _fontOnly = False; }

      void backgroundToPixelValue() { _valuesChanged = True;
	    _oldPixelValue = _color->_pixelValue;
	    _color->_pixelValue =
	       theDrawArea->getColor(theDrawArea->getColorID("Background"))->_pixelValue; }
	    
      void restoreOldPixelValue() { _valuesChanged = True; _color->_pixelValue = _oldPixelValue; }

}; // Context

#endif
