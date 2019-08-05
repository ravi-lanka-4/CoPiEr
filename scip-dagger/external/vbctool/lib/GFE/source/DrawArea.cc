////////////////////////////////////////////////////////////////////////
//   Filename : DrawArea.cc
//
//   Version  : 16.03.94
//
//   Author   : Martin Diehl & Joachim Kupke
//
//   Language : C++
//
//   Purpose  : displays the DrawArea with all drawing-methods
//
////////////////////////////////////////////////////////////////////////        

// Motif includes
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>

#include <assert.h>

// Young includes
#include "Application.h"
#include "CmdList.h"
#include "Cmd.h"
#include "MenuBar.h"


// GFE includes
#include "GFE.inc"
#include "GFEWindow.h"
#include "ClientInterface.h"
#include "DrawArea.h"
#include "ROColor.h"
#include "Context.h"
#include "ClearScreenCmd.h"
#include "SwitchToZoomingCmd.h"
#include "OpenZoomRectCmd.h"
#include "ZoomCmd.h"
#include "FitImageCmd.h"
#include "PrinterCmd.h"
#include "Postscript.h"
#include "DraggingCmd.h"
#include "DisplayArea.h"
#include "ColorContextCmd.h"
#include "ColorContext.h"
#include "ColorInformation.h"
#include "PrintPopup.h"

////////////////////////////////////////////////////////////////////////
//
//
//          Private Member Functions
//
//                   STATIC
//
////////////////////////////////////////////////////////////////////////


//************************************************************************
//
//                     resizeCallback
//
//   Is called after a resizing of the scrolled window and calls the 
//   resize() member function of the according DrawArea instance.
//
//************************************************************************ 
void  DrawArea::resizeCallback(Widget,
				XtPointer clientData,
				XtPointer)
{ 
	DrawArea *obj = (DrawArea *) clientData;
	obj->resize();
} // resizeCallback


//************************************************************************
//
//                 exposeCallback
//
//************************************************************************ 
void DrawArea::exposeCallback(Widget,
				XtPointer clientData,
				XtPointer cbs)
{
	DrawArea *obj = (DrawArea *) clientData;
	obj->expose((XmDrawingAreaCallbackStruct*) cbs);
}

//************************************************************************
//
//                 verSBValueChangedCallback
//
//************************************************************************ 
void DrawArea::verSBValueChangedCallback(Widget,
				XtPointer clientData,
				XtPointer cbs)
{
	DrawArea *obj = (DrawArea *) clientData;
	obj->verSBValueChanged((XmScrollBarCallbackStruct*) cbs);
}


//************************************************************************
//
//                 horSBValueChangedCallback
//
//************************************************************************ 
void DrawArea::horSBValueChangedCallback(Widget,
				XtPointer clientData,
				XtPointer cbs)
{
	DrawArea *obj = (DrawArea *) clientData;
	obj->horSBValueChanged((XmScrollBarCallbackStruct*) cbs);
}


//************************************************************************
//
//             button1MotionEventHandler
//
//************************************************************************ 
void DrawArea::button1MotionEventHandler(Widget widget,
				  XtPointer clientData,
				  XEvent *event,
				  Boolean *continueToDiapatch)
{
	DrawArea *obj = (DrawArea*) clientData;
	obj->button1Motion(event);
}


//************************************************************************
//
//             button2MotionEventHandler
//
//************************************************************************ 
void DrawArea::button2MotionEventHandler(Widget widget,
				  XtPointer clientData,
				  XEvent *event,
				  Boolean *continueToDiapatch)
{
	DrawArea *obj = (DrawArea*) clientData;
	obj->button2Motion(event);
}


//************************************************************************
//
//                  buttonReleaseEventHandler
//
//************************************************************************ 
void DrawArea::buttonReleaseEventHandler(Widget widget,
					XtPointer clientData,
					XEvent *event,
					Boolean *continueToDispatch)
{
	DrawArea *obj = (DrawArea*) clientData;
	if (event->xbutton.button==Button1)
		obj->button1Release(event);
	if (event->xbutton.button==Button2)
		obj->button2Release(event);
}

//************************************************************************
//
//	       buttonPressEventHandler
//
//************************************************************************
void DrawArea::buttonPressEventHandler(Widget widget,
				       XtPointer clientData,
				       XEvent *event,
				       Boolean *continueToDispatch)
{
   DrawArea *obj = (DrawArea *) clientData;

   if (event->xbutton.button==Button3)
      obj->button3Press(event);
   if (event->xbutton.button==Button1)
      obj->button1Press(event);
}

////////////////////////////////////////////////////////////////////////
//
//
//          Private Member Functions
//
//                   NON STATIC
//
////////////////////////////////////////////////////////////////////////




//************************************************************************
//
//                           resize()
//
//   Called after each resizing of the clip window of the scrolled window.
//
//   I.e. each time the user changes the size of the scrolled window,   
//   but we can't attach such a callback function to the scrolled window
//   directly.
//
//************************************************************************ 
void DrawArea::resize()
{
   adjustToNewView(RESIZE_NEW_VIEW);
} // resize



//************************************************************************
//
//                  expose
//
//************************************************************************ 
void DrawArea::expose(XmDrawingAreaCallbackStruct *cbs)
{
   if (_pixmap)
	XCopyArea(XtDisplay(_dArea), _pixmap,
		  XtWindow(_dArea), _copyGC,
		  (cbs->event)->xexpose.x, 
		  (cbs->event)->xexpose.y,
		  (cbs->event)->xexpose.width,
		  (cbs->event)->xexpose.height,
		  (cbs->event)->xexpose.x,
		  (cbs->event)->xexpose.y
		  );
}


void DrawArea::button1Motion(XEvent *event)
{
   Point<int> point;

   point.newInit(event->xmotion.x, event->xmotion.y);
   _openZoomRectCmd->executeWithPoint(&point);
	                         // If zooming is currently allowed
	                         // and we are in zooming mode, the OpenZoomRectCmd
	                         // will do the right thing. Otherwise it will do
	                         // nothing.

   if (!_openZoomRectCmd->active()) {
      Point<CoordType> p(_scaler->xReConvert(point.x()), _scaler->yReConvert(point.y()));
      theClientInterface->button1Motion(&p);
   }

} // button1Motion


void DrawArea::button2Motion(XEvent *event)
{
   Point<int> *point;

   point = new Point<int>(event->xmotion.x, event->xmotion.y);
   _draggingCmd->executeWithPoint(point);
   
   delete point;

} // button2Motion



void DrawArea::button1Release(XEvent *event)
{
   _zoomCmd->executeWithOpenZoomRectCmd(_openZoomRectCmd);
   _switchToZoomingCmd->undoit();

}// button1Release


void DrawArea::button2Release(XEvent *event)
{
   _draggingCmd->endOfDragging();

}// button2Release

void DrawArea::button3Press(XEvent *event)
{
   Point<CoordType> point(_scaler->xReConvert(event->xmotion.x),
			  _scaler->yReConvert(event->xmotion.y));
   theClientInterface->button3Press(point);
}

void DrawArea::button1Press(XEvent *event)
{
   if (_openZoomRectCmd->active()) return;
   
   Point<CoordType> point(_scaler->xReConvert(event->xmotion.x),
			  _scaler->yReConvert(event->xmotion.y));
   theClientInterface->button1Press(point);
}

//************************************************************************
//
//                 verSBValueChanged
//
//************************************************************************
void DrawArea::verSBValueChanged(XmScrollBarCallbackStruct *cbs)
{
// Funktionierte nicht!
//   XtVaGetValues(_verSB,
//                 XmNvalue,	&_verSBValue,
//                 NULL);
   handleScrolling();
}


//************************************************************************
//
//                 horSBValueChanged
//
//************************************************************************
void DrawArea::horSBValueChanged(XmScrollBarCallbackStruct *cbs)
{
// Funktionierte nicht!
//   XtVaGetValues(_horSB,
//                 XmNvalue,	&_horSBValue,
//                 NULL);
   handleScrolling();
}


//************************************************************************
//
//                 handleScrolling
//
//************************************************************************
//inline
void DrawArea::handleScrolling()
{
// Funktionierte nicht! (alt: alles was auskommentiert ist, alles andere ist neu)
//   static Rect<int> viewPort(_horSBValue,
//			     _verSBValue,
//			     _horSBSliderSize,
//			     _verSBSliderSize);
//
//   _SBScaler   ->reConvertRect(&viewPort, &_demandedVisibleRange);

   int x, y;
   XtVaGetValues(_horSB, XmNvalue, &x, NULL);
   XtVaGetValues(_verSB, XmNvalue, &y, NULL);

   CoordType dx = _SBScaler->reConvert(x - _horSBValue);
   CoordType dy = _SBScaler->reConvert(y - _verSBValue);

   dy = _yInvert ? -dy : dy;

   _demandedVisibleRange.moveRect(dx, dy);

//   adjustToNewView(SCROLLING_NEW_VIEW);
   adjustToNewView(DRAGGING_NEW_VIEW);
} // handleScrolling


/////////////////////////////////////////////////////////////
//
//       adjustToNewView()
//
// Manages everything if the size or position of the visible
// area of the current image changes.
// This function is also invoked if the size of the window
// changes.
// It takes care about the settings of the scrollbars and
// calls the repaint callback functions of the client
// if there must be something repainted, because the scale
// changes or there is a new part visible, that was former
// invisible.
//
// Important for this functions are the following global
// informations:
//
//    _demadedVisibleRange    Rect that says which part of
//                            the whole coord range the user
//                            wants to see.
//
//    _dAreaSize         The current size of the clip
//                            window, which is the same as
//                            the size of the drawing area.
//
/////////////////////////////////////////////////////////////
void DrawArea::adjustToNewView(int reason)
{
   Rect<double>	     dummy;
   static Boolean first=True;


   static Scaler<CoordType, int> oldScaler;
   Rect<int> oldDAreaSize = _dAreaSize;
   Rect<CoordType>  oldVisibleRange = _actualVisibleRange;


   // force the next Color/Context to be explicitely applied

   _lastUsedColor = 0;
   _lastUsedContext = 0;

   if (reason !=SCROLLING_NEW_VIEW && reason !=DRAGGING_NEW_VIEW)
   {
   // Don't look beyond coordinate range, normaly.
       //Rect<CoordType> t(_demandedVisibleRange ^ _coordinateRange);
      _demandedVisibleRange = _demandedVisibleRange ^ _coordinateRange;

   // Get the current size of the clip window

      Dimension width, height;
      XtVaGetValues(_dArea,
                 XmNwidth,  &width,
                 XmNheight, &height,
                 NULL);

      _dAreaSize.newInit(0, 0, width-1, height-1);

      if (!(oldDAreaSize==_dAreaSize))
      {
	 // Create new pixmap because a pixmap cannot
	 // be resized.
	 if (_pixmap) XFreePixmap(XtDisplay(_dArea), _pixmap); 
	 _pixmap = XCreatePixmap(XtDisplay(_dArea), 
                              RootWindowOfScreen(XtScreen(_dArea)),
                              _dAreaSize.width()+1,
                              _dAreaSize.height()+1,
                              DefaultDepthOfScreen(XtScreen(_w)));

	 // Clear pixmap
	 unsigned long     color;
	 XtVaGetValues(_dArea,XmNbackground, &color,
                    NULL);
	 XSetForeground(XtDisplay(_dArea), *_pointerToClearGC, color);
	 XFillRectangle(XtDisplay(_dArea), _pixmap, *_pointerToClearGC,
                     0, 0,
                     _dAreaSize.width()+1,
                     _dAreaSize.height()+1);
      }
   }
   
   kill (_scaler);
   _scaler = new Scaler<CoordType, int>	  // Create new Scaler, perhaps 
                 ( _demandedVisibleRange, // this is the first one at all.
                   _dAreaSize,
                   _actualVisibleRange,
                   _yInvert,
		   ROUND_TARGET);

      // Determine the actual coordinate range which is the biggest rect
      // in client coordinates that can be displayed if the picture is not
      // zoomed. This is possibly larger than _coordinateRange because
      // the draw area might have another width to height ratio.

      _actualCoordRange = _coordinateRange;   
      _actualCoordRange.convertToRatio(_dAreaSize.ratio());

      
      // Determine the size the draw area would have if it could contain
      // the whole actualCoordRange in the current scale.
      Rect<int> t(0, 0, _scaler->scale(_actualCoordRange.width()),
                        _scaler->scale(_actualCoordRange.height()));
      _virtualDrawAreaSize = t;
                                           
      if (reason != SCROLLING_NEW_VIEW)
      {

	 // Fit scroll bars //////////////////////////

// Musste auskommentiert werden, da bei _yInvert == FALSE die rechte Scrollbar genau falsch
// herum lief
//
//	 if (_yInvert) 
	     _verSBOrientation = XmMAX_ON_BOTTOM;
//	 else _verSBOrientation = XmMAX_ON_TOP;

	 _horSBMaximum = _virtualDrawAreaSize.maxX();
	 _verSBMaximum = _virtualDrawAreaSize.maxY();

	 // Create a Scaler that scales between the scrollbar values and
	 // and the client coordinates.
	 if (_SBScaler) delete(_SBScaler);
	 _SBScaler = new Scaler<CoordType, int>(_actualCoordRange,
						_virtualDrawAreaSize,
						dummy,
						_yInvert,
						ROUND_TARGET);

	 // Determine where the sliders have to stand.      
	 _horSBValue = _SBScaler->xToTarget(_actualVisibleRange.minX());
	 _verSBValue = _yInvert ? _SBScaler->yToTarget(_actualVisibleRange.maxY()) :
				  _SBScaler->yToTarget(_actualVisibleRange.minY());

	 // Determin how big the sliders have to be.
	 _horSBSliderSize = _SBScaler->scale(_actualVisibleRange.width());
	 _verSBSliderSize = _SBScaler->scale(_actualVisibleRange.height());

	 if (_horSBValue> _horSBMaximum- _horSBSliderSize)
	       _horSBValue = _horSBMaximum- _horSBSliderSize;
	 if (_verSBValue> _verSBMaximum- _verSBSliderSize)
	       _verSBValue = _verSBMaximum- _verSBSliderSize;
      
	 if (_horSBValue < 0) _horSBValue = 0;
	 if (_verSBValue < 0) _verSBValue = 0;
	    
      
	 XtVaSetValues(_horSB,
		       XmNmaximum,       _horSBMaximum,
		       XmNminimum,       0,
		       XmNsliderSize,    _horSBSliderSize,
		       XmNpageIncrement, (_horSBSliderSize-_SBSizePageDiff)/2,
		       XmNincrement,	 (_horSBSliderSize-_SBSizePageDiff)/20,
		       XmNvalue,         _horSBValue,
		       NULL);

	 XtVaSetValues(_verSB,
		       XmNmaximum,       _verSBMaximum,
		       XmNminimum,       0,
		       XmNsliderSize,    _verSBSliderSize,
		       XmNpageIncrement, (_verSBSliderSize-_SBSizePageDiff)/2,
		       XmNincrement,	 (_verSBSliderSize-_SBSizePageDiff)/20,
		       XmNvalue,         _verSBValue,
		       XmNprocessingDirection,   _verSBOrientation,
		       NULL);
      }

      // If this is the first time this function was called
      // (which is then because of the first resize event after the window
	  // creation), we just store the scaler and return.
      if (first)
      {
	 first = False;
	 oldScaler = *_scaler;   // backup for next time.
	 return;
      }


   // The function ends here if it was envoked from the initCoordinateRange()
   // function, because in this case we don't need a repaint, because there
   // will be nothing to be repainted.
   if (reason == INIT_COORDINATE_RANGE)
      return;

   // Even if scale hasn't changed we have something to do.
   // Copy the part of the image that was visible before to its new
   // place and let the client repaint the rest. 
   if (reason == SCROLLING_NEW_VIEW || reason == DRAGGING_NEW_VIEW)
   {
      startDrawingInPixmap();
      //clearScreen();

      Rect<CoordType> copyRect;
      copyRect = _actualVisibleRange^oldVisibleRange; // Intersection.

      CoordType help = _yInvert ? copyRect.maxY() : copyRect.minY();
      
      int oldXPos = oldScaler.xToTarget(copyRect.minX());
      int oldYPos = oldScaler.yToTarget(help);
      int width   = oldScaler.scale(copyRect.width())+1;
      int height  = oldScaler.scale(copyRect.height())+1;
      int newXPos = _scaler->xToTarget(copyRect.minX());
      int newYPos = _scaler->yToTarget(help);


      // revisions necessary !!!
      Pixmap hp = XCreatePixmap(XtDisplay(_dArea),
				RootWindowOfScreen(XtScreen(_dArea)),
				width,
				height,
				DefaultDepthOfScreen(XtScreen(_w)));

      XCopyArea(XtDisplay(_dArea), _pixmap,
                hp, _copyGC,
                oldXPos, oldYPos,
                width,	 height,
                0,0);

      clearScreen();

      XCopyArea(XtDisplay(_dArea), hp,
                _pixmap, _copyGC,
                0,0,
                width,	 height,
                newXPos, newYPos);

/*      Rect<int> copyArea(newXPos, newYPos, width, height);
      Rect<int> *list = _dAreaSize - copyArea;
      for (int j = 0; j<=3; j++)
	 if (list[j].isEmpty())
	    break;
	 else
	    clearScreen(list[j]);
*/

      XFreePixmap(XtDisplay(_dArea), hp);
      
      // Let the client repaint the rest

	    _repainting = True;

      Rect<CoordType> *rectlist = _actualVisibleRange-oldVisibleRange;
      
      for (int i = _numberOfRepaintCallbacks; i >= 1; i--)
	 _repaintCallbacks[i]((rectlist), _repaintCallbackClientData[i]);

      delete[] rectlist;
      
	    _repainting = False;

      oldScaler = *_scaler;   // backup for next time.

      drawingFinished();
      return;
   }

// The scaling factor changed, so the client has to redraw the visible
// part of the image.

   if (reason == RESIZE_NEW_VIEW || reason == ZOOMING_NEW_VIEW)
      startDrawing();
      
   clearScreen();

   Rect<CoordType> rectList[3];

   rectList[0] = _actualVisibleRange;

   // Let the client repaint the rest

	 _repainting = True;
   
   for (int i = _numberOfRepaintCallbacks; i >= 1; i--)
      _repaintCallbacks[i](rectList, _repaintCallbackClientData[i]);

	 _repainting = False;
	 
   if (reason == RESIZE_NEW_VIEW || reason == ZOOMING_NEW_VIEW)
      drawingFinished();

   oldScaler = *_scaler;   // backup for next time.

   return;

} // adjustToNewView



//*************************** INLINE *************************************
//        
//                  getColorID
//
//   Search for a given customName in the colorList and return its 
//   colorID, if the name was found return 0 otherwise.
//   
//************************************************************************
ColorID DrawArea::getColorID(char *customName)
{
	 for(ColorID i=1; i<=_numberRegisteredColors; i++)
	 {
		 if (strcmp(_colorList[i]->customName(), customName)==0)
		 return i;
	 }
	 return 0;
 } // getColorID

//*************************** INLINE *************************************
//        
//                  getContextID
//
//   Search for a given customName in the contextList and return its 
//   contextID, if the name was found return 0 otherwise.
//   
//************************************************************************
ContextID DrawArea::getContextID(char *customName)
{
	 for(ContextID i=1; i<=_numberRegisteredContexts; i++)
	 {
		 if (strcmp(_contextList[i]->customName(), customName)==0)
		 return i;
	 }
	 return 0;
 } // getContexID

 

//************************************************************************
//
//             setColor           
//
//   Sets the desired color for the next drawings referenced through 
//   the color Id number given from the registration function 
//   createNewROColor.
//
//************************************************************************
//inline
Status DrawArea::setColor(ColorID colorID)
{

		 if (colorID == XORID)
		 {
		     if (_repainting)
		     {
			fprintf(stderr, "WARNING: don't use XORID in Repainting\n");
			return ERROR;
		     }
			
		     _pointerToGC = &_XOrGC;
		     return SUCCESS;
		 }

		 if (colorID == CLEARID)
		 {
		     if (_repainting)
		     {
			fprintf(stderr, "WARNING: don't use CLEARID in Repainting\n");
			return ERROR;
		     }
		     
		     ColorID h;
		     
		     if ((h = _clearScreenCmd->colorID()) > 0)
			return setColor(h);
			
		     _pointerToGC = &_clearGC;
		     return SUCCESS;
		 }

	 _pointerToGC = &_drawGC;

	 if ((colorID != _lastUsedColor) || _colorList[colorID]->valuesChanged())
	 // don't make unnecessary Xlib calls!
	 {          
		 if (colorID > _numberRegisteredColors)
		     return ERROR;

		 _colorList[colorID]->applyToGC(_drawGC);
		  
		 _lastUsedColor = colorID;
		 _lastUsedContext = 0; // Forces the next Context to be explicitly
				       // applied.
		 return SUCCESS;

	 }
	 else
	    return SUCCESS;
		  
} // setColor


//************************************************************************
//
//              applieContextToGC
//
//   Sets the desired properties that are contained in the passed Context
//   in the _drawGC.
//
//************************************************************************
//inline
Status DrawArea::applieContextToGC(ContextID contextID)
{
      if (contextID == XORID)
      {
	 if (_repainting)
	 {
	    fprintf(stderr, "WARNING: don't use XORID in Repainting\n");
	    return ERROR;
	 }
	 
	 _pointerToGC = &_XOrGC;
	 return SUCCESS;
      }

      if (contextID == CLEARID)
      {
	 if (_repainting)
	 {
	    fprintf(stderr, "WARNING: don't use CLEARID in Repainting\n");
	    return ERROR;
	 }
	 
	 ContextID h;

	 if ((h = _clearScreenCmd->colorID()) > 0)
	    return applieContextToGC(h);
	    
	 _pointerToGC = &_clearGC;
	 return SUCCESS;
      }

   _pointerToGC = &_drawGC;

   if ((contextID != _lastUsedContext) || _contextList[contextID]->valuesChanged())
   // don't make unnecessary Xlib calls!
   {
      if (contextID > _numberRegisteredContexts) // Don't reference an unregistered
	 return ERROR;			         // Context

      _contextList[contextID]->applyToGC(_drawGC);
      _lastUsedContext = contextID;
      _lastUsedColor = 0;   // Forces the next Color to be explicitly applied.
      return SUCCESS;
   }
   else return SUCCESS;
   
}// applieContextToGC


//************************************************************************
//
//             saveDrawArea
//
//   Copies the whole drawing area to the pixmap.
//   This function is used by the drawingFinished function.
//
//************************************************************************
//inline
void DrawArea::saveDrawArea()
	 {
	 XCopyArea(XtDisplay(_dArea), XtWindow(_dArea), _pixmap, _copyGC, 0, 0,
		_dAreaSize.width()+1, _dAreaSize.height()+1, 0, 0);
	 }

//***********************************************************************
//
//             saveDrawArea Overlaoded
//
//   Copies the given part of the drawing area to the pixmap.
//   This function is used by the drawingFinished function.
//
//************************************************************************
//inline
void DrawArea::saveDrawArea(RectItem<int> &r)
	 {
	XCopyArea(XtDisplay(_dArea), XtWindow(_dArea), _pixmap, _copyGC, r.minX(), r.minY(),
		r.width(), r.height() , r.minX(), r.minY());
	 } 






////////////////////////////////////////////////////////////////////////
//
//
//             Public Member Functions
//
//                  General Function
//
////////////////////////////////////////////////////////////////////////



//************************************************************************
//
//                       DrawArea()  Constructor
//
//************************************************************************ 
DrawArea::DrawArea(Widget parent, char *name, Rect<CoordType> &cRange,
			Boolean yInvert, DisplayArea *displayArea)
		  : UIComponent(name)
{
   theDrawArea = this;
	
// initialize private data

   _pixmap  = 0;

   _copyGC  = 0;
   _drawGC  = NULL;
   _clearGC = NULL;

   _repainting = False;

   _numberRegisteredColors  = 0;
   _lastUsedColor           = 0;    // Forces the first color to be applied.
   _colorList               = NULL;
	
   _numberRegisteredContexts= 0;
   _lastUsedContext         = 0;    // Forces the first Context to be applied.
   _contextList             = NULL;

   _numberOfRepaintCallbacks = 0;
   _repaintCallbacks = NULL;
   _repaintCallbackClientData = NULL;

   _serverGrabbed = FALSE;
   _pixmapOnly = False;

   _scaler   = NULL;   // Holds a pointer to the current scaler object.
   _SBScaler = NULL;

   _yInvert = yInvert;


   _coordinateRange       = cRange;  // DrawArea can show at least this range.

   _demandedVisibleRange  = cRange;  // User wants to see everything at the
                                     // beginning.
	
	_SBSizePageDiff = 5;  // Difference between the slider size of
			      // the scrollbars and the page increment.
			      // Should have a value between 1 and 10.

// initialize the Pointer to the ClientInterface-Object
	
// Create the Postscript-Object
   _postscript = new Postscript();

// Create the ColorInformation-Object
   _colorInformation = new ColorInformation(XtDisplay(theApplication->baseWidget()),
					    XtWindow (theApplication->baseWidget()));

// Create the scrolled window as the widget tree's root
   _w = XtVaCreateWidget(name, xmScrolledWindowWidgetClass,
			 parent,
			 XmNscrollingPolicy, XmAPPLICATION_DEFINED,
			 XmNvisualPolicy,    XmVARIABLE,
			 XmNpaneMinimum,     100,
			 NULL);


// Create all needed GC's     
	_clearGC = XCreateGC(XtDisplay(_w),
			RootWindowOfScreen(XtScreen(_w)),
			0, NULL);
	_copyGC  = XCreateGC(XtDisplay(_w),
			RootWindowOfScreen(XtScreen(_w)),
			0, NULL);
	_drawGC  = XCreateGC(XtDisplay(_w),
			RootWindowOfScreen(XtScreen(_w)),
			0, NULL);
	_XOrGC   = XCreateGC( XtDisplay(_w),
			RootWindowOfScreen(XtScreen(_w)),
			0, NULL);

	_pointerToClearGC = &_clearGC;	  // sets _pointerToClearGC to default, _clearGC
					  // never changed

	_pointerToGC = &_drawGC;	  // set to default


#ifdef LINUX  // otherwise you would see nothing under Linux
	XSetForeground(XtDisplay(_w), _XOrGC, WhitePixelOfScreen(XtScreen(_w)));
#else
	XSetForeground(XtDisplay(_w), _XOrGC, BlackPixelOfScreen(XtScreen(_w)));
#endif


	XSetFunction(XtDisplay(_w), _XOrGC, GXxor);	

   // Create the actual drawing area widget.     
	_dArea = XtVaCreateManagedWidget("dArea",
				  xmDrawingAreaWidgetClass, _w,
//				  XmNwidth,  100,   // This are arbitrary values that will
//				  XmNheight, 100,   // be directly overwritten by the scrolled window
				  NULL);            // which will scale the drawing area to its
                                // desired size.

   // Set geometry if not in resource
   Dimension width, height;
   XtVaGetValues(_dArea, XmNwidth, &width, XmNheight, &height, NULL);
   if (width <= 1 && height <= 1)
      XtVaSetValues(_dArea, XmNwidth, 100, XmNheight, 100, NULL);
      
   // Register Callbacks
	XtAddCallback (_dArea, XmNexposeCallback,
		       &DrawArea::exposeCallback,
		       (XtPointer) this);

	XtAddCallback (_dArea, XmNresizeCallback,
		       &DrawArea::resizeCallback,
		       (XtPointer) this);
								  
   // Add new event handlers for mouse button input.
	XtAddEventHandler(_dArea, Button1MotionMask, FALSE,
			  button1MotionEventHandler,
			  (XtPointer) this);

	XtAddEventHandler(_dArea, Button2MotionMask, FALSE,
			  button2MotionEventHandler,
			  (XtPointer) this);

	XtAddEventHandler(_dArea, ButtonReleaseMask, FALSE,
			  buttonReleaseEventHandler,
			  (XtPointer) this);

	XtAddEventHandler(_dArea, ButtonPressMask, FALSE,
			  buttonPressEventHandler,
			  (XtPointer) this);

   // Create scrollbars
	_horSB = XtVaCreateManagedWidget("sbh",xmScrollBarWidgetClass,
	                           _w,
				  XmNorientation,  XmHORIZONTAL,
				  XmNmaximum,	   100,   // This are arbitrary values
				  XmNminimum,	   0,     // that will be directly overwritten
				  XmNsliderSize,   100,  // in the first call to adjustToNewView().
				  NULL);

	_verSB = XtVaCreateManagedWidget("sbv", xmScrollBarWidgetClass,
				  _w,
				  XmNorientation, XmVERTICAL,
				  XmNmaximum,	   100,
				  XmNminimum,	   0,
				  XmNsliderSize,   100,				  
				  NULL);

   // Register callbacks.
	XtAddCallback(_horSB, XmNvalueChangedCallback,
		      &DrawArea::horSBValueChangedCallback,
		      XtPointer(this));
		      
	XtAddCallback(_verSB, XmNvalueChangedCallback,
		      &DrawArea::verSBValueChangedCallback,
		      XtPointer(this));
				  
				  
   XmScrolledWindowSetAreas(_w, _horSB, _verSB, _dArea);	

   // Create a DEFAULT CONTEXT a context object that serves as a storage
   // for the default values of the later created context instances.
	_defaultContext = new Context (XtDisplay(_dArea), NULL, NULL, NULL);
							
		  

	installDestroyHandler();      // This is a inhereted member function.

// initializes the used commands

   _printerCmd = new PrinterCmd("Print", False);
   _colorContextCmd = new ColorContextCmd("Color/Context-Chooser", True);
   _draggingCmd = new DraggingCmd("unvisible DraggingCmd", False);
   _clearScreenCmd   = new ClearScreenCmd("unvisible ClearScreenCmd", TRUE);
							  // The clear screen is a command that is not present
							  // in the menu bar, because there is no reason to
							  // allow the user to clear the screen. At this
							  // level. The client program can give the user
							  // a 'Clear Screen' button if it wants.
							  // The clearScreenCmd is created though because
							  // it is convenient to work with commands any way.
   _switchToZoomingCmd   = new SwitchToZoomingCmd("Area Zoom",False);
					// This is the only one of the three commands that belongs
					// to zooming that is visible in the menu bar.
   _openZoomRectCmd  = new OpenZoomRectCmd("unvisible OpenZoomRectCmd", False);
   _zoomCmd      = new ZoomCmd("unvisible ZoomCmd", False);
   _fitImageCmd = new FitImageCmd("Fit To Window", False);
   _repaintCmd = new RepaintCmd("Repaint", False);

   _colorContextPopup = new ColorContext("Color/Context",
					 theApplication->baseWidget());

   _printPopup = new PrintPopup("Print",
				theGFEWindow->baseWidget());

} // DrawArea Constructor


//************************************************************************
//
//                       ~DrawArea()  Destructor
//
//************************************************************************ 
  DrawArea::~DrawArea()
{
   deleteRepaintCallbacks();
	
   delete _colorContextPopup;
   delete _printPopup;

// Free the Colors and Context Objects

   for (int i=1; i<=_numberRegisteredColors; i++)
      delete _colorList[i];
   delete _colorList;

   for (int i = 1; i<=_numberRegisteredContexts; i++)
      delete _contextList[i];
   delete _contextList;

   delete _defaultContext;

   kill(_scaler);
   kill(_SBScaler);
   
// Free the pixmap if they still exist
	if(_pixmap)
		XFreePixmap(XtDisplay(_dArea), _pixmap);

// Free the allocated GCs

        XFreeGC(XtDisplay(_dArea), _copyGC);
	XFreeGC(XtDisplay(_dArea), _clearGC);
	XFreeGC(XtDisplay(_dArea), _XOrGC);
	XFreeGC(XtDisplay(_dArea), _drawGC);
	
// Ungrab the server if it accidentaly is still grabbed.
	if (_serverGrabbed)
		 XUngrabServer(XtDisplay(_dArea));

// Delete Postscript-Object
	delete _postscript;

// Delete ColorInformation
	delete _colorInformation;

// Delete commands

   delete _clearScreenCmd;

   delete _switchToZoomingCmd;
   delete _openZoomRectCmd;
   delete _zoomCmd;

   delete _fitImageCmd;
   delete _repaintCmd;

   delete _draggingCmd;

   delete _printerCmd;

   delete _colorContextCmd;

} // ~DrawArea Destructor



//************************************************************************
//
//                       createMenuEntries
//
//   Creates the menu entries specific for the DrawArea class,
//   which are:     FitImageCmd
//
//   And perhaps later some others!
//
//************************************************************************ 
  void  DrawArea::createMenuEntries(CmdList *cmdListFile, CmdList *cmdListEdit, CmdList *cmdListView)
{
// create menu entries that belong to this draw area
   
   cmdListFile->add(_printerCmd);

// _openZoomRectCmd should be active after switching to zooming.
   _switchToZoomingCmd->addToActivationList(_openZoomRectCmd);

// After performing the zooming the _openZoomRectCmd must be inactive again.
   _zoomCmd->addToDeactivationList(_openZoomRectCmd);   

// After switching to zooming the _switchToZoomCmd must be inactive
   _switchToZoomingCmd->addToDeactivationList(_switchToZoomingCmd);

// _zoomCmd should be active after opening the zoom rect.
   _openZoomRectCmd->addToActivationList(_zoomCmd);

// After performing the zooming the _switchToZoomingCmd can be active again.
   _zoomCmd->addToActivationList(_switchToZoomingCmd);

   _zoomCmd->addToActivationList(_fitImageCmd);
   _fitImageCmd->addToDeactivationList(_fitImageCmd);
	
   cmdListView->add(_switchToZoomingCmd);
   cmdListView->add(_fitImageCmd);
   cmdListView->add(_repaintCmd);
   cmdListView->add(_colorContextCmd);

}// createMenuEntries


//************************************************************************
//
//                       clearScreen
//
//   Just calls the execute() member function of the registered 
//   clearScreenCmd object.
//
//************************************************************************ 
  void  DrawArea::clearScreen()
{
	_clearScreenCmd->execute();
} // clearScreen

void DrawArea::clearScreen(Rect<int> &r)
{
      fillCanvas(_clearScreenCmd->colorID(), r);
}

//************************************************************************
//
//        fillCanvas
//
//   Fills the entire canvas with the given color used by ClearScreenCmd.
//
//************************************************************************
void DrawArea::fillCanvas(ColorID colorID)
{
   if (!_postscript->printFlag())
   {
	 if (colorID)
	 {
	    setColor(colorID);
	    _pointerToClearGC = &_drawGC;
	 }
	 else
	    _pointerToClearGC = &_clearGC;

	 if (!_pixmapOnly)
	    XFillRectangle(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToClearGC,
			   0, 0,
			   _dAreaSize.width()+1,
			   _dAreaSize.height()+1);
			
	 if (!_serverGrabbed)
		 XFillRectangle(XtDisplay(_dArea), _pixmap, *_pointerToClearGC,
				0, 0,
				_dAreaSize.width()+1,
				_dAreaSize.height()+1);
   }
   else _postscript->setBackground(colorID);
}// fillCanvas

void DrawArea::fillCanvas(ColorID colorID, Rect<int> &rect)
{
   //if (!_postscript->printFlag())
   {
	 if (colorID)
	 {
	    setColor(colorID);
	    _pointerToClearGC = &_drawGC;
	 }
	 else
	    _pointerToClearGC = &_clearGC;

	 int y = rect.minY();

	 if (!_pixmapOnly)
	    XFillRectangle(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToClearGC,
			   rect.minX(),
			   y,
			   rect.width(),
			   rect.height());
			
	 if (!_serverGrabbed)
		 XFillRectangle(XtDisplay(_dArea), _pixmap, *_pointerToClearGC,
				rect.minX(),
				y,
				rect.width(),
				rect.height());
   }
   //else _postscript->setBackground(colorID);
}

//************************************************************************
//
//                 addRepaintCallback
//
//************************************************************************
void DrawArea::addRepaintCallback(clientRepaintCallback repaintCallback,
				  XtPointer            clientData)
{
   _switchToZoomingCmd->activate();
   _printerCmd->activate();
   _draggingCmd->activate();
   _repaintCmd->activate();

   // check if the Repaint-Callback is already added
   for (int i = 1; i<=_numberOfRepaintCallbacks; i++)
      if ((_repaintCallbacks[i]==repaintCallback) && (_repaintCallbackClientData[i]==clientData)) return;
   

   // The list of callbacks starts with 1 (we leave the 0 element unused!).
   clientRepaintCallback *newList  = new clientRepaintCallback[_numberOfRepaintCallbacks+2];
   XtPointer *ClientDataList	    = new XtPointer[_numberOfRepaintCallbacks+2];

   for (int i = 1; i<=_numberOfRepaintCallbacks; i++)
   {
      newList[i]	= _repaintCallbacks[i];
      ClientDataList[i] = _repaintCallbackClientData[i];
   }

   _numberOfRepaintCallbacks++;
	
   newList[_numberOfRepaintCallbacks]	     = repaintCallback;
   ClientDataList[_numberOfRepaintCallbacks] = clientData;

   if (_repaintCallbacks)
   {
      delete[] _repaintCallbacks;
      delete[] _repaintCallbackClientData;
   }
   
   _repaintCallbacks	      = newList;
   _repaintCallbackClientData = ClientDataList;

} // addRepaintCallback

//**********************************************************************
//
//             removeRepaintCallback
//
//**********************************************************************
void DrawArea::removeRepaintCallback(clientRepaintCallback repaintCallback,
				     XtPointer clientData)
{
   if (_numberOfRepaintCallbacks <= 0)
      return;

   clientRepaintCallback *newList = new clientRepaintCallback[_numberOfRepaintCallbacks];
   XtPointer *ClientDataList      = new XtPointer[_numberOfRepaintCallbacks];

   register int j = 0;
   
   for (int i = 1; i<=_numberOfRepaintCallbacks; i++)
   {
      if ((_repaintCallbacks[i]==repaintCallback) && (_repaintCallbackClientData[i]==clientData))
	 j = 1;

      if (i == _numberOfRepaintCallbacks) break;

      newList[i]        = _repaintCallbacks[i+j];
      ClientDataList[i] = _repaintCallbackClientData[i+j];
   }

   if (!j)
   {
      delete[] newList;
      delete[] ClientDataList;
   }
   else
   {
      if (--_numberOfRepaintCallbacks == 0) {
	 prohibitRepaint();
	 delete[] newList;
	 delete[] ClientDataList;
      }
      else {
	 assert(_repaintCallbacks && _repaintCallbackClientData);

	 delete[] _repaintCallbacks;
	 delete[] _repaintCallbackClientData;

	 _repaintCallbacks          = newList;
	 _repaintCallbackClientData = ClientDataList;
      }
   }
}

//************************************************************************
//
//               deleteRepaintCallbacks
//               
//************************************************************************ 
void DrawArea::deleteRepaintCallbacks()
{
   prohibitRepaint();
}

//************************************************************************
//
//               prohibitRepaint
//               
//************************************************************************ 
void DrawArea::prohibitRepaint()
{
	_switchToZoomingCmd->deactivate();
	_printerCmd->deactivate();
	_draggingCmd->deactivate();
	_repaintCmd->deactivate();
	
	_numberOfRepaintCallbacks = 0;

      if (_repaintCallbacks)
      {
	 delete[] _repaintCallbacks;
	 delete[] _repaintCallbackClientData;
	 _repaintCallbacks = NULL;
	 _repaintCallbackClientData = NULL;
      }
} // prohibitRepaint




//************************************************************************
//
//                     initCoordinateRange
//
// The client has to call this function to init a coordSystem other than
// (0,0) - (1,1).
//
// This function must be called before any drawing call.
//
//************************************************************************ 
  void DrawArea::initCoordinateRange(Rect<CoordType> *rect,
					 Boolean  yInvert)
{
	_coordinateRange      = *rect;
	_demandedVisibleRange = *rect;
	_yInvert	      = yInvert;

	adjustToNewView(INIT_COORDINATE_RANGE);
}

//************************************************************************
//
//                     setCoordinateRange
//
// This function is the same as initCoordinateRange() except the fact
// that setCoordinateRange() possibly triggers a call to the client's
// repaint procedure.
//
//
//************************************************************************ 
  void DrawArea::setCoordinateRange(Rect<CoordType> *rect,
					 Boolean  yInvert)
{
	_coordinateRange      = *rect;
	_demandedVisibleRange = *rect;
	_yInvert	      = yInvert;

	adjustToNewView(NEW_COORDINATE_RANGE);
}


//************************************************************************
//
//                  createNewROColor
//
//   Creates a new Read-Only color that holds the color represented by
//   color name NAME. This new color will hold the custom name customName.
//
//   Use this function if you only want to specify the color in all your
//   drawings.
//   To specify any other propertie you have to use the Context struct
//   which determines the color but also line widhts, fill style, etc.
//
//************************************************************************ 
  ColorID  DrawArea::createNewROColor(char *name, char *customName)
{
	// test if name is in the ColorInformatonClass
        if (!_colorInformation->colorNamePosition(name))
	{
	    fprintf(stderr, "%s not in your rgb.txt-File\n",name);
	    return 0;
	}
	
	// First test if this custom name is not allready present
	// in a color.
	ColorID  number;
	if (number = getColorID(customName))
		return number;
		
	// Create new Color Data Structur
	ROColor *newColor = new ROColor(name, customName);

	// Allocate a new list large enough to hold
	// one more color.
	// colorList starts with 1 (we leave the 0 element
	// unused!).
	Color **newList = new Color*[_numberRegisteredColors+2];

	// Copy the old list to the new
	// colorList starts with 1 (we leave the 0 element
	// unused!).
	for (ColorID i = 1; i<=_numberRegisteredColors; i++)
		newList[i] = _colorList[i];

	_numberRegisteredColors++;
	
	// Add new color object to the end of the list
	newList[_numberRegisteredColors] = newColor;

	// Delete the old list and make _colorList point
	// to the new list
	delete _colorList;
	_colorList = newList;
		
	return _numberRegisteredColors;
} // createNewROColor

//************************************************************************
//
//             lockColor
//
//************************************************************************ 
  void DrawArea::lockColor(ColorID colorID)
{
	if (colorID <= _numberRegisteredColors)
	{
		_colorList[colorID]->lock();
	}
} // lockColor

//************************************************************************
//
//            unlockColor
//
//************************************************************************ 
  void DrawArea::unlockColor(ColorID colorID)
{
	if (colorID <= _numberRegisteredColors)
	{
		_colorList[colorID]->unlock();
	}
} // unlockColor



//************************************************************************
//
//                  createNewContext
//
//   Creates a Context object, that holds the default values for all
//   graphic propertires, like they are stored in the _defaultContext.
//   Also a new ROColor will be created that holds the color described
//   by name an that holds the custom name given by customName.
//   The whole Context will hold this custom name too.
//
//************************************************************************ 
  ContextID  DrawArea::createNewContext(char *name, char *customName)
{          
	// test if name is in the ColorInformatonClass
        if (!_colorInformation->colorNamePosition(name))
	{
	    fprintf(stderr, "%s not in your rgb.txt-File\n", name);
	    return 0;
	}
	
	// First test if this custom name is not allready present
	// in a color.
	ContextID  number;
	if (number = getContextID(customName))
		return number;

/*	// Create the Contexts Color element.
	ROColor *color = new ROColor(name, this, customName);
*/
	// Create new Context Data Structur
	Context *newContext = new Context (XtDisplay(_dArea),
					   _defaultContext,
					   name,
					   customName);

	// Allocate a new list large enough to hold
	// one more context.
	// contextList starts with 1 (we leave the 0 element unused!).
	Context **newList = new Context *[_numberRegisteredContexts+2];

	// Copy the old list to the new contextList
	// starts with 1 (we leave the 0 element unused!).
	for (ContextID i = 1; i<=_numberRegisteredContexts; i++)
		newList[i] = _contextList[i];

	_numberRegisteredContexts++;
	
	// Add new Context object to the end of the list
	newList[_numberRegisteredContexts] = newContext;

	// Delete the old list and make _contextList point
	// to the new list
	delete(_contextList);
	_contextList = newList;
		
	return _numberRegisteredContexts;
} // createNewContext



// Context Overload
ContextID  DrawArea::createNewContext(char *name, char *customName, char* shortName)
{          
	ContextID nr = createNewContext(name, customName);

	_contextList[nr]->loadFontName(shortName);

	return nr;
} // createNewContext


   
//************************************************************************
//
//             lockContext
//
//************************************************************************ 
  void DrawArea::lockContext(ContextID ID)
{
	if (ID <= _numberRegisteredContexts)
	{
		_contextList[ID]->lock();
	}
} // lockContext

//************************************************************************
//
//            unlockContext
//
//************************************************************************ 
  void DrawArea::unlockContext(ContextID ID)
{
	if (ID <= _numberRegisteredContexts)
	{
		_contextList[ID]->unlock();
	}
} // unlockContext




//************************************************************************
//
//             startDrawing             
//
//   Use this before starting a bigger bunch of drawing commands.
//   This function will grab the X-server.
//   Then make all the drawings and immediatly after that call
//   drawingFinished().
//
//************************************************************************
void DrawArea::startDrawing()
{
	if (!_serverGrabbed)
	{
		XGrabServer(XtDisplay(_dArea));
		_serverGrabbed = True;
	}
} // startDrawing

void DrawArea::startDrawingInPixmap()
{
     _pixmapOnly = True;
}

//************************************************************************
//
//             drawingFinished
//
//   Call this function after all drawings of a bunch are completed.
//   This function will save the picture and release the X-server again.
//************************************************************************
void DrawArea::drawingFinished()
{
	if (_serverGrabbed)
	{
	        saveDrawArea();
		XUngrabServer(XtDisplay(_dArea));
		_serverGrabbed = False;
	}

	if (_pixmapOnly)
	{
	        _pixmapOnly = False;
		XCopyArea(XtDisplay(_dArea), _pixmap, XtWindow(_dArea), _copyGC, 0, 0,
		                _dAreaSize.width()+1, _dAreaSize.height()+1, 0, 0);
	}
} // drawingFinished

//************************************************************************
//
//             drawingFinished
//
//                Overloaded
//             
//   Same as above but saves only the passed area.
//
//************************************************************************
void DrawArea::drawingFinished(const XRectangle& area)
{
	XCopyArea(XtDisplay(_dArea), XtWindow(_dArea), _pixmap, _copyGC, 
		area.x, area.y, area.width, area.height,
		area.x, area.y);

	if (_serverGrabbed)
	{
		XUngrabServer(XtDisplay(_dArea));
		_serverGrabbed = False;
	}
} // drawingFinished

//************************************************************************
//
//              getContext
//
//
//************************************************************************
Context *DrawArea::getContext(ContextID contextID)
{
	  if (contextID > _numberRegisteredContexts)  // Don't reference a unregistered 
		  return NULL;                        // Context.

	  return _contextList[contextID];
} // getContext

//************************************************************************
//
//              getROColor
//
//
//************************************************************************
Color *DrawArea::getColor(ColorID colorID)
{
	  if (colorID > _numberRegisteredColors)  // Don't reference a unregistered 
		  return NULL;                        // Context.

	  return _colorList[colorID];
} // getContext





///////////////////////////////////////////////////////////////////
//
//
//             Public Drawing Functions
//
//
//
///////////////////////////////////////////////////////////////////


//************************************************************************
//
//        drawLine            
//
//************************************************************************
void DrawArea::drawLine(ColorID colorID,
					CoordType x1, CoordType y1,  
					CoordType x2, CoordType y2)
{
   if (!_postscript->printFlag())
   {
	 Line<CoordType> l2(x1, y1, x2, y2);
         Line<CoordType> l(l2 ^ _actualVisibleRange);

	 x1 = l.x1();
	 x2 = l.x2();
	 y1 = l.y1();
	 y2 = l.y2();

	 if (setColor(colorID) == ERROR) return;

	 if (!_pixmapOnly)
	    XDrawLine(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
		      _scaler->xToTarget(x1),
		      _scaler->yToTarget(y1),
		      _scaler->xToTarget(x2),
		      _scaler->yToTarget(y2));
			
	 if (!_serverGrabbed)
	    XDrawLine(XtDisplay(_dArea), _pixmap, *_pointerToGC,
		      _scaler->xToTarget(x1),
		      _scaler->yToTarget(y1),
		      _scaler->xToTarget(x2),
		      _scaler->yToTarget(y2));
   }
   else if (colorID != XORID && colorID != CLEARID)
      _postscript->drawLine(colorID, x1, y1, x2, y2);

}// DrawLine

//************************************************************************
//
//        drawLine  Overloaded here with ContextID as an argument            
//
//************************************************************************
void DrawArea::drawLine(ContextID contextID,
			CoordType x1, CoordType y1,  
			CoordType x2, CoordType y2)
{
   if (!_postscript->printFlag())
   {
	 Line<CoordType> l2(x1, y1, x2, y2);
         Line<CoordType> l(l2 ^ _actualVisibleRange);

	 x1 = l.x1();
	 x2 = l.x2();
	 y1 = l.y1();
	 y2 = l.y2();

	 if (applieContextToGC(contextID) == ERROR) return;// Bring the information in the
							   // Context to effect.

	 if (!_pixmapOnly)
	    XDrawLine(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
		      _scaler->xToTarget(x1),
		      _scaler->yToTarget(y1),
		      _scaler->xToTarget(x2),
		      _scaler->yToTarget(y2));
			
	 if (!_serverGrabbed)
	    XDrawLine(XtDisplay(_dArea), _pixmap, *_pointerToGC,
		      _scaler->xToTarget(x1),
		      _scaler->yToTarget(y1),
		      _scaler->xToTarget(x2),
		      _scaler->yToTarget(y2));
   }
   else if (contextID != XORID && contextID != CLEARID)
      _postscript->drawLine(contextID, x1, y1, x2, y2);
   
}// DrawLine, overloaded with Context argument


//************************************************************************
//
//        fillCircle         
//
//************************************************************************
void DrawArea::fillCircle(ColorID colorID,
						CoordType x, CoordType y, 
						CoordType radius)
{
   if (!_postscript->printFlag())
   {
	 if (setColor(colorID) == ERROR) return;
	 int rad = _scaler->scale(radius);
	 int width = _scaler->scale(radius*2.0);
	    if (!width) width = 1;

	 if (!_pixmapOnly)
	    XFillArc(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
		     _scaler->xToTarget(x)-rad,
		     _scaler->yToTarget(y)-rad,
		     width, width,
		     0, 23040 );
			
	 if (!_serverGrabbed)
	    XFillArc(XtDisplay(_dArea), _pixmap, *_pointerToGC,
		     _scaler->xToTarget(x)-rad,
		     _scaler->yToTarget(y)-rad,
		     width, width,
		     0, 23040 );

   }
   else if (colorID != XORID && colorID != CLEARID)
      _postscript->fillCircle(colorID, x, y, radius);
   
}// fillCircle


//************************************************************************
//
//        fillCircle  Overloaded, this is the version with a Context
//
//************************************************************************
void DrawArea::fillCircle(ContextID contextID,
					 CoordType x, CoordType y, 
		CoordType radius)
{
   if (!_postscript->printFlag())
   {
	 if (applieContextToGC(contextID) == ERROR) return;
	 int rad = _scaler->scale(radius);
	 int width = _scaler->scale(radius*2.0);
	    if (!width) width = 1;

	 if (!_pixmapOnly)
	    XFillArc(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
		     _scaler->xToTarget(x)-rad,
		     _scaler->yToTarget(y)-rad,
		     width, width,
		     0, 23040 );
			
	 if (!_serverGrabbed)
	    XFillArc(XtDisplay(_dArea), _pixmap, *_pointerToGC,
		     _scaler->xToTarget(x)-rad,
		     _scaler->yToTarget(y)-rad,
		     width, width,
		     0, 23040 );
   }
   else if (contextID != XORID && contextID != CLEARID)
      _postscript->fillCircle(contextID, x, y, radius);
   
 }// fillCircle, overloaded


//************************************************************************
//
//		  fillRectangle 
//
//************************************************************************
void DrawArea::fillRectangle(ContextID contextID,
			     CoordType xpos,
			     CoordType ypos,
			     CoordType width,
			     CoordType height)
{
   Rect<CoordType> rect(xpos, ypos, width, height);
   fillRectangle(contextID, rect);

}// fillRectangle



//************************************************************************
//
//		  fillRectangle OVERLOAD to avoid roundingerrors
//
//************************************************************************
void DrawArea::fillRectangle(ContextID contextID,
			     Rect<CoordType> &rect)
{
   if (!_postscript->printFlag())
   {
      int x = _scaler->xToTarget(rect.minX());
      int y = _scaler->yToTarget(rect.minY());
      int w = _scaler->xToTarget(rect.maxX()) - x;
      int h = _scaler->yToTarget(rect.maxY()) - y;

      if (_yInvert)
      {
	 h = -h;
	 y = y - h;
      }
   
	 if (applieContextToGC(contextID) == ERROR) return;

	 if (!_pixmapOnly)
	    XFillRectangle(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
			   x,
			   y,
			   w+1,
			   h+1);
			
	 if (!_serverGrabbed)
	    XFillRectangle(XtDisplay(_dArea), _pixmap, *_pointerToGC,
			   x,
			   y,
			   w+1,
			   h+1);
   }
   else if (contextID != XORID && contextID != CLEARID)
      _postscript->fillRectangle (contextID, rect);

}// fillRectangle



//************************************************************************
//
//		  fillRectangle with ColorID
//
//************************************************************************
void DrawArea::fillRectangle(ColorID colorID,
			     CoordType xpos,
			     CoordType ypos,
			     CoordType width,
			     CoordType height)
{
   Rect<CoordType> rect(xpos, ypos, width, height);
   fillRectangle(colorID, rect);

}// fillRectangle



//************************************************************************
//
//       fillRectangle OVERLOAD to avoid roundingerrors (with ColorID)
//
//************************************************************************
void DrawArea::fillRectangle(ColorID colorID,
			     Rect<CoordType> &rect)
{
   if (!_postscript->printFlag())
   {
      int x = _scaler->xToTarget(rect.minX());
      int y = _scaler->yToTarget(rect.minY());
      int w = _scaler->xToTarget(rect.maxX()) - x;
      int h = _scaler->yToTarget(rect.maxY()) - y;

      if (_yInvert)
      {
	 h = -h;
	 y = y - h;
      }
   
	 if (setColor(colorID) == ERROR) return;

	 if (!_pixmapOnly)
	    XFillRectangle(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
			   x,
			   y,
			   w+1,
			   h+1);
			
	 if (!_serverGrabbed)
	    XFillRectangle(XtDisplay(_dArea), _pixmap, *_pointerToGC,
			   x,
			   y,
			   w+1,
			   h+1);
   }
   else if (colorID != XORID && colorID != CLEARID)
      _postscript->fillRectangle (colorID, rect);

}// fillRectangle



//************************************************************************
//
//		  drawRectangle 
//
//************************************************************************
void DrawArea::drawRectangle(ContextID contextID,
			     CoordType xpos,
			     CoordType ypos, 
			     CoordType width,
			     CoordType height)
{
   Rect<CoordType> rect(xpos, ypos, width, height);
   drawRectangle(contextID, rect);

}// drawRectangle


//************************************************************************
//
//		  drawRectangle OVERLOAD
//
//************************************************************************
void DrawArea::drawRectangle(ContextID contextID,
			     Rect<CoordType> &rect)
{
   if (!_postscript->printFlag())
   {
      int x = _scaler->xToTarget(rect.minX());
      int y = _scaler->yToTarget(rect.minY());
      int w = _scaler->xToTarget(rect.maxX()) - x;
      int h = _scaler->yToTarget(rect.maxY()) - y;

      if (_yInvert)
      {
	 h = -h;
	 y = y - h;
      }
   
	 if (applieContextToGC(contextID) == ERROR) return;

	 if (!_pixmapOnly)
	    XDrawRectangle(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
			   x,
			   y,
			   w,
			   h);
			
	 if (!_serverGrabbed)
	    XDrawRectangle(XtDisplay(_dArea), _pixmap, *_pointerToGC,
			   x,
			   y,
			   w,
			   h);
   }
   else if (contextID != XORID && contextID != CLEARID)
      _postscript->drawRectangle(contextID, rect);
   
}// drawRectangle


//************************************************************************
//
//        drawCircle            
//
//************************************************************************
void DrawArea::drawCircle(ColorID colorID,
		CoordType x, CoordType y,
		CoordType radius)
{
   if (!_postscript->printFlag())
   {
	 if (setColor(colorID) == ERROR) return;
	 int rad = _scaler->scale(radius);
	 int width = 2 * rad;
	    if (!width) width = 1;
	 int xx = _scaler->xToTarget(x)-rad;
	 int yy = _scaler->yToTarget(y)-rad;
      
	 if (!_pixmapOnly)
	    XDrawArc(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
		     xx,
		     yy,
		     width, width,
		     0, 23040 );

	 if (!_serverGrabbed)
	    XDrawArc(XtDisplay(_dArea), _pixmap, *_pointerToGC,
		     xx,
		     yy,
		     width, width,
		     0, 23040 );
   }
   else if (colorID != XORID && colorID != CLEARID)
      _postscript->drawCircle(colorID, x, y, radius);
}// drawCircle


//************************************************************************
//
//        drawCircle, overloaded this is the version with a
//        ContextID argument.
//
//************************************************************************
void DrawArea::drawCircle(ContextID contextID,
					 CoordType x, CoordType y,
					 CoordType radius)
{
   if (!_postscript->printFlag())
   {
	 if (applieContextToGC(contextID) == ERROR) return;
	 int rad = _scaler->scale(radius);
	 int width = 2 * rad;
	    if (!width) width = 1;

	 if (!_pixmapOnly)
	    XDrawArc(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
		     _scaler->xToTarget(x)-rad,
		     _scaler->yToTarget(y)-rad,
		     width, width,
		     0, 23040 );
			
	 if (!_serverGrabbed)
	    XDrawArc(XtDisplay(_dArea), _pixmap, *_pointerToGC,
		     _scaler->xToTarget(x)-rad,
		     _scaler->yToTarget(y)-rad,
		     width, width,
		     0, 23040 );
   }
   else if (contextID != XORID && contextID != CLEARID)
      _postscript->drawCircle(contextID, x, y, radius);
   
} // drawCircle this was the version with a ContextID argument.

//************************************************************************
//
//		  drawString
//
//************************************************************************
void DrawArea::drawString(ContextID contextID,
			  char *text, int textlen,
			  Rect<CoordType> &rect,
			  int boxpos, float tolerance)
{
      int x = _scaler->xToTarget(rect.minX());
      int y = _scaler->yToTarget(rect.minY());
      int w = _scaler->xToTarget(rect.maxX()) - x;
      int h = _scaler->yToTarget(rect.maxY()) - y;

      if (_yInvert)
	 h = -h;
      else
	 y +=h;
   
	 if (!_postscript->printFlag())
	 {
	    if (applieContextToGC(contextID) == ERROR) return;

	    Status st = _contextList[contextID]->applyFontToGC(*_pointerToGC, text,
							       x, y, w, h, boxpos,
							       1, textlen, tolerance);

	    if (st==ERROR)
	       ;//fillRectangle (contextID, rect);
	    else
	    {


	       if (!_pixmapOnly)
		  XDrawString(XtDisplay(_dArea), XtWindow(_dArea), *_pointerToGC,
			      x,
			      y,
			      text,
			      strlen(text));
		     
			
	       if (!_serverGrabbed)
		  XDrawString(XtDisplay(_dArea), _pixmap, *_pointerToGC,
			      x,
			      y,
			      text,
			      strlen(text));
	    }
	 }
	 else if (contextID != XORID && contextID != CLEARID)
	    {
	       int oldlen = _contextList[contextID]->oldLen();
	       _postscript->drawString(contextID, text, rect, boxpos, textlen, tolerance);
	       _contextList[contextID]->calculateFontSize(text, x, y, w, h, boxpos, 1,
							  oldlen, 0.0); // set old defaults back
	    }

} // drawString


//************************** INLINE **************************************
//
//                  drawBoxWithXOr
//
//    for displaying the Zoom-Box
//
//************************************************************************
//inline
void DrawArea::drawBoxWithXOr(Rect<int> *box)
{
	XDrawRectangle(XtDisplay(_dArea), XtWindow(_dArea), _XOrGC,
		  box->minX(), box->minY(),
		  box->width(), box->height());
}

//************************************************************************
//
//                flush
//
//************************************************************************
void DrawArea::flush()
{
        XFlush(XtDisplay(_dArea));
}// flush


//************************************************************************
//
//                sync
//
//************************************************************************
void DrawArea::sync()
{
        XSync(XtDisplay(_dArea), False);
}// sync


void DrawArea::setMaxLineWidth(ContextID cID, CoordType mlw)
{
   getContext(cID)->setMaxLineWidth(mlw);
}

//************************************************************************
//
//          updateDisplay
//
//************************************************************************
void DrawArea::updateDisplay()
{
   XmUpdateDisplay(baseWidget());
   XmUpdateDisplay(_dArea);
   XmUpdateDisplay(_horSB);
   XmUpdateDisplay(_verSB);
}
