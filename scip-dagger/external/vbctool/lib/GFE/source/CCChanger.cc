/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	CCChanger.cc
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	Displays the Color/Context-Changer-Popup
// 
//
////////////////////////////////////////////////////////////////////////	
#include <Xm/Label.h>
#include <Xm/SelectioB.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/List.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleBG.h>


#include "CCChanger.h"
#include "DrawArea.h"
#include "ColorContext.h"
#include "ROColor.h"
#include "Context.h"
#include "ColorInformation.h"
#include "FontInformation.h"
#include "GFEResourceManager.h"

CCChanger::CCChanger(char *c, Widget w, int nr, ColorContext *cc)
   : PopupManager(c, w)
{
   _colorContext = cc;
   _thisNumber = _firstNumber = nr;
   _customName = _colorName = _fontName = NULL;
   _font = NULL;
   _xfont = NULL;
   _lineStyle = NULL;
   _capStyle = NULL;
   _joinStyle = NULL;

   GFEResourceManager rsc(STANDARDRESOURCE);
   char *h = rsc.getstring("LineWidthScaleNumberOfValues");
   if (h)
      if(sscanf(h, "%d", &_lwsnov)<=0) _lwsnov = 1000;
}

CCChanger::~CCChanger()
{
   kill (_customName);
   kill (_colorName);
   kill (_fontName);
   if (_font)
      XmFontListFree(_font);
   if (_xfont)
      XFreeFont(XtDisplay(_fontDisplay), _xfont);
   kill (_lineStyle);
   kill (_joinStyle);
   kill (_capStyle);
}

void CCChanger::createActionArea()
{
   XtManageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
   XtSetSensitive(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON), False);
}

void CCChanger::setNr(int nr)
{
   _thisNumber = nr;
}

void CCChanger::createControlArea()
{
   // begin of frame Picture
   
   _mainRow = XtVaCreateManagedWidget("chooserRow",
				      xmRowColumnWidgetClass,
				      _w,
				      NULL);

   // Widgets in main-row

   _ccLabel = XtVaCreateManagedWidget("ccLabel",
				     xmLabelWidgetClass,
				     _mainRow,
				     NULL);

   XtVaCreateManagedWidget("separator1",
			   xmSeparatorWidgetClass,
			   _mainRow,
			   NULL);

   _subRow = XtVaCreateManagedWidget("subRow",
				     xmRowColumnWidgetClass,
				     _mainRow,
				     XmNorientation, XmHORIZONTAL,
				     NULL);

   // Widgets in _subRow

   _firstSubRow = XtVaCreateManagedWidget("firstSubRow",
					  xmRowColumnWidgetClass,
					  _subRow,
					  NULL);

   _sep1 = XtVaCreateManagedWidget("Sep",
			   xmSeparatorWidgetClass,
			   _subRow,
			   XmNorientation, XmVERTICAL,
			   NULL);

   _secondSubRow = XtVaCreateManagedWidget("secondSubRow",
					   xmRowColumnWidgetClass,
					   _subRow,
					   NULL);

   _thirdSubRow = XtVaCreateManagedWidget("thirdSubRow",
					  xmRowColumnWidgetClass,
					  _subRow,
					  NULL);

   _sep2 = XtVaCreateManagedWidget("Separator2",
			   xmSeparatorWidgetClass,
			   _subRow,
			   XmNorientation, XmVERTICAL,
			   NULL);

   _forthSubRow = XtVaCreateManagedWidget("forthSubRow",
					  xmRowColumnWidgetClass,
					  _subRow,
					  NULL);

   // Widgets in _firstSubRow

   _firstColorLabel = XtVaCreateManagedWidget("Color List",
					      xmLabelWidgetClass,
					      _firstSubRow,
					      NULL);

   _colorList = XmCreateScrolledList(_firstSubRow, "colorList", NULL, 0);
   XtManageChild(_colorList);

   _secondColorLabel = XtVaCreateManagedWidget("Color",
					       xmLabelWidgetClass,
					       _firstSubRow,
					       NULL);

   Widget frame = XtVaCreateManagedWidget("ColorFrame", xmFrameWidgetClass, _firstSubRow, NULL);
   _colorDisplay = XtVaCreateManagedWidget("colorDisplay",
					   xmDrawingAreaWidgetClass,
					   frame,
					   NULL);

   // Widgets in _second- and thirdSubRow

   _lineStyle = new RadioField(_secondSubRow, "lineStyleRadioBox", 3, line, 0,
			       XmVERTICAL, 1, True);
   _joinStyle = new RadioField(_secondSubRow, "joinStyleRadioBox", 3, join, 0,
			       XmVERTICAL, 1, True);
   _capStyle  = new RadioField(_thirdSubRow,  "capStyleRadioBox",  4, cap,  0,
			       XmVERTICAL, 1, True);

   _contextScale = XtVaCreateManagedWidget("contextScale",
					   xmScaleWidgetClass,
					   _thirdSubRow,
					   NULL);

   // Widgets in _forthSubRow

   _fontListLabel = XtVaCreateManagedWidget("Font List",
					    xmLabelWidgetClass,
					    _forthSubRow,
					    NULL);

   _fontList = XmCreateScrolledList(_forthSubRow, "fontList", NULL, 0);
   XtManageChild(_fontList);

   _fontTextLabel = XtVaCreateManagedWidget("Font",
					    xmLabelWidgetClass,
					    _forthSubRow,
					    NULL);


   frame = XtVaCreateManagedWidget("FontFrame", xmFrameWidgetClass, _forthSubRow, NULL);
   _fontDisplay = XtVaCreateManagedWidget("ABC abc",
					  xmLabelWidgetClass,
					  frame,
					  NULL);

   // end of chooser-menu

   XtVaSetValues(_colorList,
		 XmNitemCount,	 _colorContext->_colorListItems,
		 XmNitems,	 _colorContext->_colorList,
		 XmNvisibleItemCount, 7,
		 NULL);

   XtVaSetValues(_fontList,
		 XmNitemCount,	 _colorContext->_fontListItems,
		 XmNitems,	 _colorContext->_fontList,
		 XmNvisibleItemCount, 7,
		 NULL);

}

Status CCChanger::setCC(char *name)
{
   kill (_customName);
   _customName = strdup(name);

   if(!_w)
      getDialog();

   XmString str = XmStringCreateSimple(name);

   XtVaSetValues(_ccLabel,
		 XmNlabelString, str,
		 XmNalignment, XmALIGNMENT_CENTER,
		 NULL);

   XmStringFree(str);

   _pointerToContext = NULL;
   _pointerToColor   = NULL;
   
   ColorID colorID = theDrawArea->getColorID(name);

   if (!colorID)
   {
      ContextID contextID = theDrawArea->getContextID(name);
      if (!contextID)
	 return ERROR;

      _pointerToContext = theDrawArea->_contextList[contextID];

      // set Color

      kill (_colorName);
      _colorName = strdup(_pointerToContext->xName());
      setActualColor();

      if (_pointerToContext->fontOnly())
      {
	 XtUnmanageChild(_secondSubRow);
	 XtUnmanageChild(_thirdSubRow);
	 XtUnmanageChild(_sep1);
      }
      else
      {
	 XtManageChild(_secondSubRow);
	 XtManageChild(_thirdSubRow);
	 XtManageChild(_sep1);
	 // XtSetSensitive(_secondSubRow, True);
	 // XtSetSensitive(_thirdSubRow, True);
      }

      if(_pointerToContext->fontName())
      {
	 kill (_fontName);
	 _fontName = strdup(_pointerToContext->shortName());
	 setActualFont();

	 XtManageChild(_forthSubRow);
	 XtManageChild(_sep2);
	 // XtSetSensitive(_forthSubRow, True);
      }
      else
      {
	 XtUnmanageChild(_forthSubRow);
	 XtUnmanageChild(_sep2);
      }
	 // XtSetSensitive(_forthSubRow, False);

      _maxscaleValue = _pointerToContext->getMaxLineWidth();

      for (_decimalPoints = -5; _decimalPoints < 6; _decimalPoints++)
            if (1/(_decimalFaktor = pow(10, _decimalPoints)) <= _maxscaleValue/_lwsnov) break;

      XtVaSetValues(_contextScale,
		    XmNminimum,	    0,
		    XmNmaximum,	    round(_maxscaleValue*_decimalFaktor),
		    XmNdecimalPoints,_decimalPoints,
		    XmNorientation,  XmHORIZONTAL,
		    XmNshowValue,    True,
		    XtVaTypedArg,    XmNtitleString, XmRString,
		    "Line Width",    10,
		    XmNscaleMultiple,1,
		    NULL);

      int lw = round(_pointerToContext->lineWidth() * _decimalFaktor);
      XtVaSetValues(_contextScale, XmNvalue,
		    lw, NULL);

      int activeNumber;
      
      switch(_pointerToContext->lineStyle())
      {
	 case LineSolid:      activeNumber = 1;
			      break;
	 case LineOnOffDash:  activeNumber = 2;
			      break;
	 case LineDoubleDash: activeNumber = 3;
      }
      _lineStyle->setValue(activeNumber);

      switch(_pointerToContext->capStyle())
      {
	 case CapNotLast:     activeNumber = 1;
			      break;
	 case CapButt:	      activeNumber = 2;
			      break;
	 case CapRound:	      activeNumber = 3;
			      break;
	 case CapProjecting:  activeNumber = 4;
      }
      _capStyle->setValue(activeNumber);

      switch(_pointerToContext->joinStyle())
      {
	 case JoinMiter:      activeNumber = 1;
			      break;
	 case JoinRound:      activeNumber = 2;
			      break;
	 case JoinBevel:      activeNumber = 3;
      }
      _joinStyle->setValue(activeNumber);
      
			      
   }
   else
   {
      _pointerToColor = theDrawArea->_colorList[colorID];

      // set Color
      kill (_colorName);
      _colorName = strdup(_pointerToColor->xName());
      setActualColor();

      XtUnmanageChild(_secondSubRow);
      XtUnmanageChild(_thirdSubRow);
      XtUnmanageChild(_forthSubRow);
      XtUnmanageChild(_sep1);
      XtUnmanageChild(_sep2);
      // XtSetSensitive(_secondSubRow, False);
      // XtSetSensitive(_thirdSubRow, False);
      // XtSetSensitive(_forthSubRow, False);

   }

   XtVaSetValues(_colorDisplay,
		 XmNheight, 30,
		 NULL);

   return SUCCESS;
}

void CCChanger::apply()
{
   if (_pointerToContext)
   {
      int val;

      XtVaGetValues(_contextScale, XmNvalue, &val, NULL);
      _pointerToContext->setLineWidth(val/_decimalFaktor);
  
      int style;
   
      val = _lineStyle->getValue();
      switch(val)
      {
	 case 1:   style = LineSolid;
		   break;
	 case 2:   style = LineOnOffDash;
		   break;
	 case 3:   style = LineDoubleDash;
      }
      _pointerToContext->setLineStyle(style);
      
      val = _capStyle->getValue();
      switch(val)
      {
	 case 1:   style = CapNotLast;
		   break;
	 case 2:   style = CapButt;
		   break;
	 case 3:   style = CapRound;
		   break;
	 case 4:   style = CapProjecting;
      }
      _pointerToContext->setCapStyle(style);

      val = _joinStyle->getValue();
      switch(val)
      {
	 case 1:   style = JoinMiter;
	           break;
	 case 2:   style = JoinRound;
		   break;
	 case 3:   style = JoinBevel;
      }
      _pointerToContext->setJoinStyle(style);

      _pointerToContext->changeColor(_colorName);

      if (_fontName)
	 _pointerToContext->loadFontName(_fontName);

   }



   if (_pointerToColor)
   {
      ROColor *ptc = (ROColor *)_pointerToColor;
      ptc->changeColor(_colorName);
   }
   

	 

   theDrawArea->adjustToNewView(GENERAL_NEW_VIEW);
}

void CCChanger::ok()
{
   _colorContext->oneCleared(_thisNumber);

   apply();
}

void CCChanger::cancel()
{
   _colorContext->oneCleared(_thisNumber);
}


void CCChanger::colorSelectedCallback(Widget, XtPointer clientData, XtPointer)
{
   CCChanger *obj = (CCChanger *) clientData;

   obj->colorSelected();
}

void CCChanger::fontSelectedCallback(Widget, XtPointer clientData, XtPointer)
{
   CCChanger *obj = (CCChanger *) clientData;

   obj->fontSelected();
}

void CCChanger::enableCallbacks()
{
   PopupManager::enableCallbacks();

   XtAddCallback(_colorList, XmNbrowseSelectionCallback, &CCChanger::colorSelectedCallback, (XtPointer) this);
   XtAddCallback(_fontList, XmNbrowseSelectionCallback, &CCChanger::fontSelectedCallback, (XtPointer) this);
}

void CCChanger::disableCallbacks()
{
   PopupManager::disableCallbacks();

   XtRemoveCallback(_colorList, XmNbrowseSelectionCallback, &CCChanger::colorSelectedCallback, (XtPointer) this);
   XtRemoveCallback(_fontList, XmNbrowseSelectionCallback, &CCChanger::fontSelectedCallback, (XtPointer) this);
}

void CCChanger::colorSelected()
{
   XmString *h;
   int count;

   XtVaGetValues(_colorList,
		 XmNselectedItems, &h,
		 XmNselectedItemCount, &count,
		 NULL);

   kill (_colorName);
   XmStringGetLtoR(h[0], XmSTRING_DEFAULT_CHARSET, &_colorName);

   unsigned long pixel = theDrawArea->_colorInformation->allocateColor(_colorName, _firstNumber);

   XtVaSetValues(_colorDisplay, XmNbackground, pixel, NULL);
}

void CCChanger::fontSelected()
{
   XmString *h;
   int count;

   XtVaGetValues(_fontList,
		 XmNselectedItems, &h,
		 XmNselectedItemCount, &count,
		 NULL);

   kill (_fontName);
   XmStringGetLtoR(h[0], XmSTRING_DEFAULT_CHARSET, &_fontName);

   setActualFont();
}


void CCChanger::setActualColor()
{
      XmString xmstring[1];
      xmstring[0] = XmStringCreateSimple(_colorName);
      XtVaSetValues(_colorList,
		    XmNtopItemPosition,	  theDrawArea->_colorInformation->colorNamePosition(_colorName),
		    XmNselectedItems, xmstring,
		    XmNselectedItemCount, 1,
		    NULL);
      XmStringFree(xmstring[0]);

      unsigned long pixel = theDrawArea->_colorInformation->allocateColor(_colorName,
									_firstNumber);
      XtVaSetValues(_colorDisplay, XmNbackground, pixel, NULL);


}

void CCChanger::setActualFont()
{
      XmString xmstring[1];
      xmstring[0] = XmStringCreateSimple(_fontName);
      XtVaSetValues(_fontList,
		    XmNtopItemPosition,	  _colorContext->_fontInformation->fontNamePosition(_fontName),
		    XmNselectedItems, xmstring,
		    XmNselectedItemCount, 1,
		    NULL);
      XmStringFree(xmstring[0]);


      char hs[300];
      GFEResourceManager  rsc(FONTFILENAME);

      char *s = rsc.getstring(_fontName);
      strcpy(hs, s);
      strcat(hs, "*-18*");

      XmFontList oldFont = _font;
      XFontStruct *oldxFont = _xfont;
      
      _xfont = XLoadQueryFont(XtDisplay(_fontDisplay), hs);

      _font = XmFontListCreate(_xfont, XmSTRING_DEFAULT_CHARSET);

      XtVaSetValues(_fontDisplay,
		    XmNfontList, _font,
		    NULL);

      if (oldFont)
	 XmFontListFree(oldFont);

      if (oldxFont)
	 XFreeFont(XtDisplay(_fontDisplay), oldxFont);

}
