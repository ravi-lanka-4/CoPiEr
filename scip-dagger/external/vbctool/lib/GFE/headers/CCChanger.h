/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	CCChanger.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see .cc
// 
//
////////////////////////////////////////////////////////////////////////	

#ifndef CCCHANGER_H
#define CCCHANGER_H


#include "PopupManager.h"
#include "Status.h"

class ColorContext;
class SelectionList;
class ROColor;
class Color;
class Context;

class CCChanger : public PopupManager
{

   private:

      Widget	  _sep1;
      Widget	  _sep2;

      Widget	  _mainRow;
      Widget	  _ccLabel;
      Widget	  _subRow;
      Widget	  _firstSubRow;
      Widget	  _secondSubRow;
      Widget	  _thirdSubRow;
      Widget	  _forthSubRow;

      Widget	  _firstColorLabel;
      Widget	  _secondColorLabel;
      Widget	  _fontListLabel;
      Widget	  _fontTextLabel;
      
      Widget	  _contextScale;

      Widget	  _colorList;
      Widget	  _colorDisplay;
      Widget	  _fontList;
      Widget	  _fontDisplay;


      RadioField  *_lineStyle;
      RadioField  *_capStyle;
      RadioField  *_joinStyle;


      static char *line[];
      static char *cap[];
      static char *join[];

      
      ColorContext   *_colorContext;

      int	     _thisNumber;
      int	     _firstNumber;
      char	     *_customName;
      char	     *_colorName;
      char	     *_fontName;

      CoordType	     _maxscaleValue;
      int	     _decimalPoints;
      double	     _decimalFaktor;
      static int     _lwsnov;

      Color	     *_pointerToColor;
      Context	     *_pointerToContext;

      XmFontList     _font;
      XFontStruct    *_xfont;
      

      // list-Callbacks

      static void colorSelectedCallback(Widget, XtPointer, XtPointer);
      void colorSelected();

      static void fontSelectedCallback(Widget, XtPointer, XtPointer);
      void fontSelected();

      void setActualColor();
      void setActualFont();

   protected:

      void createControlArea();
      void createActionArea();

      void enableCallbacks();
      void disableCallbacks();
      
      void ok();
      void apply();
      void cancel();

   public:

      CCChanger(char *c, Widget w, int nr, ColorContext *cc);

      ~CCChanger();

      Status setCC(char*);
      void setNr(int);
};

#endif

