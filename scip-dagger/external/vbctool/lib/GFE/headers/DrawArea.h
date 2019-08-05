////////////////////////////////////////////////////////////////////////        
//
//      Filename :      DrawArea.h
//
//      Version  :      7.2.94
//
//      Author   :      Martin Diehl & Joachim Kupke
//
//      Language :      C++ 
//
//      Purpose  :      see .cc
//
////////////////////////////////////////////////////////////////////////        
#ifndef DRAWAREA_H
#define DRAWAREA_H


#include "GFE.inc"
#include "UIComponent.h"
#include "Color.h"
#include "Geometric.h"


class ClientInterface;
class Context;
class Color;
class MenuBar;
class ClearScreenCmd;
class OpenZoomRectCmd;
class SwitchToZoomingCmd;
class ZoomCmd;
class PrinterCmd;
class CmdList;
class Postscript;
class DraggingCmd;
class DisplayArea;
class ColorContextCmd;
class ColorContext;
class ColorInformation;
class PrintPopup;
class RepaintCmd;

class ZoomCmd;
class FitImageCmd;
class Postscript;
class DraggingCmd;
class ColorContext;
class CCChanger;


typedef void (*clientRepaintCallback) (RectItem<CoordType> *, XtPointer);

// constants
#define SCROLLING_NEW_VIEW    1
#define ZOOMING_NEW_VIEW      2
#define RESIZE_NEW_VIEW	      3
#define INIT_COORDINATE_RANGE 4
#define NEW_COORDINATE_RANGE  5
#define	GENERAL_NEW_VIEW      6
#define DRAGGING_NEW_VIEW     7

#define CLEARID		      0
#define XORID		      -1

class DrawArea : public UIComponent {

friend class ZoomCmd;
friend class FitImageCmd;
friend class Postscript;
friend class DraggingCmd;
friend class ColorContext;
friend class CCChanger;

	private:

// Private data members:
	 Pixmap      _pixmap;             // ID of buffering pixmap
	 Pixmap	     _draggingPixmap;	  // ID of the Pixmap-buffer used for dragging

	 GC          _copyGC;             // GC used for copying pixmap into DA.
	 GC          _clearGC;            // used to clear the pixmap.
	 GC          _drawGC;             // used for normal drawing.
	 GC          _XOrGC;

	 GC	     *_pointerToClearGC;  // used either for default Background or
					  // client-specified Background
	 GC	     *_pointerToGC;	  // points either to _drawGC, clearGC or _XOrGC

	 Boolean     _repainting;
	 
	 Rect<int>   _dAreaSize;         // Size of DA and pixmap.
	 Rect<int>   _virtualDrawAreaSize; // Hypothetical size of DA if it had
					   // to contain a zoomed image.
	 Rect<int>   _initialSize;       // Size of the DA at creation time
	 Rect<int>   _clipWindowSize;    // Size of the scrolled window's clip window.

	 Rect<CoordType> _coordinateRange;  // Holds the range in which the client can draw.
	 Rect<CoordType> _actualCoordRange; // Smallest rect that is >= _coordinateRange
					    // and has the same width to height ratio
                                            // as the drawin area widget.

	 Rect<CoordType> _actualVisibleRange; // Holds the part of the image that
                                              // can currently be seen.

	 Rect<CoordType> _demandedVisibleRange;
                    // The part of the _coordinateRange which the user
                    // demanded to see via a zooming or scrolling action.
							
	 Scaler<CoordType, int> *_scaler; // Scales the client coords into pixel coords.
	 Scaler<CoordType, int> *_SBScaler; // Scales between scrollbar values and
                                       // client coordinates.

	 Boolean _yInvert;

					  
	 Widget   _dArea;             // The actual DrawingArea widget.

	 Widget _horSB;	// Horizointal Scrollbars
	 Widget _verSB;	// Vertical Scrollbars
	 int _horSBSliderSize, _horSBValue;
	 int _verSBSliderSize, _verSBValue;
	 int _horSBMaximum, _verSBMaximum;
	 int _SBSizePageDiff;			   // Difference between the slider size of
						   // the scrollbars and the page increment.
						   // Should have a value between 1 and 10.

	 int _verSBOrientation;  // Signals if zero is at the top or bottom of
				 // the vertical scrollbar, depeneds on the wish
				 // of the client to have zero at the top or bottom
				 // in the picture.

	Boolean  _serverGrabbed;   // Indicates if server is grabbed. If it is,
					// releas it in the destructor.

	Boolean	 _pixmapOnly;	 // Draw only in the Pixmap, then copy Pixmap into DrawArea

	
	
	clientRepaintCallback *_repaintCallbacks;	 // Array of callbacks to trigger a redraw.
	XtPointer             *_repaintCallbackClientData;
	int		      _numberOfRepaintCallbacks;

	// Data members for the Color management 
		Color        **_colorList;       // Array of pointers to all registrated colors
		ColorID      _numberRegisteredColors;
		ColorID      _lastUsedColor;     // The color that was lastly used
													// stored to minimize changeColor calls.

	// Data members for the Context management
		Context	   **_contextList;
		ContextID  _numberRegisteredContexts;
		ContextID  _lastUsedContext;
		Context	   *_defaultContext;

	// class for Color-information and colornames-list

		ColorInformation *_colorInformation;
		

// Commands that support general draw area actions
	ClearScreenCmd *_clearScreenCmd;
	
// The commands that belong to the zooming
   SwitchToZoomingCmd   *_switchToZoomingCmd;   // Brings the mouse cursor to a magnifier
   OpenZoomRectCmd   *_openZoomRectCmd;   // Draws a rectangle on the screen when the user
						// drags over the screen opening a zoom rect.
   ZoomCmd     *_zoomCmd;     // Performs the zooming when the user releases the
						// mouse button, after opening a zoom rect.

   FitImageCmd *_fitImageCmd;

   RepaintCmd  *_repaintCmd;

// Command, that creates a Postscript-file and/or sends it to the printer

   PrinterCmd  *_printerCmd;

   ColorContextCmd  *_colorContextCmd;

   DraggingCmd *_draggingCmd;

// Pointer to the ClientInterface Object

// Private member functions:

// Private member function prototypes:

// Static member functions that are callback for Motif:

	 static void exposeCallback( Widget, XtPointer, XtPointer);
	 static void resizeCallback( Widget, XtPointer, XtPointer);
	 static void horSBValueChangedCallback( Widget, XtPointer, XtPointer);
	 static void verSBValueChangedCallback( Widget, XtPointer, XtPointer);
	 static void button1MotionEventHandler(Widget widget,
				  XtPointer clientData,
				  XEvent *event,
				  Boolean *continueToDiapatch);

	 static void button2MotionEventHandler(Widget widget,
				  XtPointer clientData,
				  XEvent *event,
				  Boolean *continueToDiapatch);
				  
	 static void buttonReleaseEventHandler(Widget widget,
				 XtPointer clientData,
				 XEvent *event,
				 Boolean *continueToDispatch);

	 static void buttonPressEventHandler(Widget widget,
				XtPointer clientData,
				XEvent *event,
				Boolean *continueToDispatch);

// Non static member functions

	 void expose(XmDrawingAreaCallbackStruct*);   // Called after an EXPOSE event
	 void resize();				      // Called after a RESIZE event
	 void horSBValueChanged(XmScrollBarCallbackStruct *cbs);
	 void verSBValueChanged(XmScrollBarCallbackStruct *cbs);
	 void handleScrolling();
	 void adjustToNewView(int);
	 void button1Motion(XEvent *event);              // Called from the button1MotionEventHandler
	 void button2Motion(XEvent *event);              // Called from the button2MotionEventHandler
	 void button1Release(XEvent *event);       // Called from the button1ReleaseEventHandler
	 void button2Release(XEvent *event);       // Called from the button2ReleaseEventHandler
	 void button3Press(XEvent *event);	   // Called from the buttonPressEventHandler
	 void button1Press(XEvent *event);	   // Called from the buttonPressEventHandler
	 
	 void sizeDrawAreaWidget(Rect<int> *); // Brings the drawing area widget to the
						// requested size.

	 void saveDrawArea();
	 void saveDrawArea(RectItem<int> &r);

    Status setColor(ColorID colorID);   // Bring color to effect.
    Status applieContextToGC(ContextID contextID); // Bring context
                                                   // to effect.

	// important Objects
	ColorContext	*_colorContextPopup;
	PrintPopup	*_printPopup;
	Postscript	*_postscript;

////////////////////////////////////////////////////////////////////////
//
//
//          Public member functios prototypes:
//
//
////////////////////////////////////////////////////////////////////////
	public:


	DrawArea(Widget, char*, Rect<CoordType> &, Boolean, DisplayArea*);
	~DrawArea();

	void sizeDrawAreaWidget();    // Brings the drawing area to the size of the
						// clip window.
	void createMenuEntries(CmdList*, CmdList*, CmdList*);

	void changeColor(ColorID, char *);
							// Change a color to the given color in the 
							// X database.


	void initCoordinateRange(Rect<CoordType> *,
				 Boolean);	     // Initializes the coordinate range, which are the
						     // coordinates which the client refere to in all
						     // drawing requests.
							

	void setCoordinateRange(Rect<CoordType> *,
				Boolean);
							// Changes the coordinate range, which are the
							// coordinates which the client refere to in all
							// drawing requests.
							

	ColorID createNewROColor(char *, char*);
							// Creates a new color and returns its
							// ID.

	void lockColor(ColorID);      // Lock a color against change by the user.
	void unlockColor(ColorID);    // Unlocks a color.

	ContextID createNewContext(char *, char*);
							// Creates a new context and returns its
							// ID.
	ContextID createNewContext(char*, char*, char*); //context overload

	void lockContext(ContextID);      // Lock a context against change by the user.
	void unlockContext(ContextID);    // Unlocks a context.


	void addRepaintCallback(clientRepaintCallback, XtPointer);
						      // Call this function to allow the DrawArea
						      // zooming and printing a picture.
						      // For both actions the DrawArea has to have
						      // the possibility to trigger a repaint of
						      // the image.

	void removeRepaintCallback(clientRepaintCallback, XtPointer);

	void prohibitRepaint();	    // Call this function to prohibit repainting again.
				    // By default, after creating a DrawArea 
				    // instance repainting is prohibited.

      void deleteRepaintCallbacks();   // Deletes all callbacks from the array of repaint
				       // callbacks. That is the same as prohibitRepaint().

	void clearScreen();
	void clearScreen(Rect<int> &r);
	void fillCanvas(ColorID colorID);
	void fillCanvas(ColorID colorID, Rect<int> &r);

	ContextID getContextID(char *customName);
	ColorID   getColorID(char *customName);


	Context *getContext(ContextID);
	Color   *getColor(ColorID);
	void setMaxLineWidth(ContextID cID, CoordType mlw);


// Simple inline return functions:

	 virtual const char *const className()
	 {
		 return ("DrawArea");
	 }
	
	 const ColorID numberRegisteredColors()
	 {
		return (_numberRegisteredColors);
	 }
	
	 void getActualCoordRange(Rect<CoordType> &r)
	 {
	   r.newInit(&_actualVisibleRange);
	 }

	 void getCoordinateRange(Rect<CoordType> &r)
	 {
	    r.newInit(&_coordinateRange);
	 }

	 Display *display() { return XtDisplay(_dArea); }
	 Window window()   { return XtWindow(_dArea); }
	 PrinterCmd *printerCmd() { return _printerCmd; }
	 Rect<CoordType> *coordinateRange() { return &_coordinateRange; }
	 Rect<CoordType> *actualCoordRange() { return &_actualCoordRange; }
	 Rect<int> *dAreaSize() { return &_dAreaSize; }

	 ColorContext *colorContextPopup() { return _colorContextPopup; }
	 PrintPopup   *printPopup()	   { return _printPopup; }
	 Postscript   *postscript()        { return _postscript; }
	 ColorInformation *colorInformation() { return _colorInformation; }

	 Widget dArea() { return _dArea; }

// Scaling services for the client program

	int scaleX(CoordType x) { return _scaler->xToTarget(x);}
	int scaleY(CoordType y) { return _scaler->yToTarget(y);}
	int scaleLength(CoordType l) {return _scaler->scale(l);}
	Scaler<CoordType, int> *scaler() { return _scaler; }
	 
	void repaint() { adjustToNewView(GENERAL_NEW_VIEW); }

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
void startDrawing();
void startDrawingInPixmap();

//************************************************************************
//
//             drawingFinished
//
//   Call this function after all drawings of a bunch are completed.
//   This function will save the picture and release the X-server again.
//************************************************************************
void drawingFinished();

//************************************************************************
//
//             drawingFinished
//
//                Overloaded
//             
//   Same as above but saves only the passed area.
//
//************************************************************************
void drawingFinished(const XRectangle& area);


/////////////////////////////////////////////////////////////////////////
//
//             Public Drawing Functions
//
//
//
/////////////////////////////////////////////////////////////////////////


//***********************************************************************
//
//        drawLine            
//
//************************************************************************
void drawLine(ColorID colorID, 
					CoordType x1, CoordType y1,  
					CoordType x2, CoordType y2);

//************************************************************************
//
//        drawLine  Overloaded here with ContextID as an argument            
//
//************************************************************************
void drawLine(ContextID Context,
			CoordType x1, CoordType y1,  
			CoordType x2, CoordType y2);


//************************************************************************
//
//        fillCircle         
//
//************************************************************************
void fillCircle(ColorID colorID, 
						CoordType x, CoordType y, 
						CoordType radius);


//************************************************************************
//
//        fillCircle  Overloaded, this is the version with a Context
//
//************************************************************************
void fillCircle(ContextID contextID, 
					 CoordType x, CoordType y, 
		CoordType radius);


//************************************************************************
//
//		  fillRectangle  
//
//************************************************************************
void fillRectangle(ContextID contextID, 
		   CoordType, CoordType, 
		   CoordType, CoordType);


//************************************************************************
//
//		  fillRectangle OVERLOAD
//
//************************************************************************
void fillRectangle(ContextID contextID, 
		   Rect<CoordType> &rect);


//************************************************************************
//
//		  fillRectangle with ColorID
//
//************************************************************************
void fillRectangle(ColorID colorID, 
		   CoordType, CoordType, 
		   CoordType, CoordType);


//************************************************************************
//
//		  fillRectangle OVERLOAD with ColorID
//
//************************************************************************
void fillRectangle(ColorID colorID, 
		   Rect<CoordType> &rect);


//************************************************************************
//
//		  drawRectangle  
//
//************************************************************************
void drawRectangle(ContextID contextID, 
		   CoordType x, CoordType y, 
		   CoordType w, CoordType h);

//************************************************************************
//
//		  drawRectangle OVERLOAD
//
//************************************************************************
void drawRectangle(ContextID contextID, 
		   Rect<CoordType> &rect);


//************************************************************************
//
//        drawCircle            
//
//************************************************************************
void drawCircle(ColorID colorID, 
		CoordType x, CoordType y,
		CoordType radius);


//************************************************************************
//
//        drawCircle, overloaded this is the version with a
//        ContextID argument.
//
//************************************************************************
void drawCircle(ContextID contextID,
		CoordType x, CoordType y,
		CoordType radius);

//************************************************************************
//
//		  drawString OVERLOAD
//
//************************************************************************
void drawString(ContextID contextID, char *text,
		CoordType xpos, CoordType ypos,
		CoordType width, CoordType height,
		int boxpos)
{
   Rect<CoordType> rect(xpos, ypos, width, height);
   drawString(contextID, text, strlen(text), rect, boxpos, 0.0);

} // drawString

void drawString(ContextID contextID, char *text,
		int textlen, Rect<CoordType> &rect, int boxpos)
{
   drawString(contextID, text, textlen, rect, boxpos, 0.0);
}

void drawString(ContextID contextID, char *text,
		Rect<CoordType> &rect, int boxpos, float tolerance)
{
   drawString(contextID, text, strlen(text), rect, boxpos, tolerance);
}

void drawString(ContextID contextID, char *text,
		Rect<CoordType> &rect, int boxpos)
{
   drawString(contextID, text, strlen(text), rect, boxpos, 0.0);
}
		

//************************************************************************
//
//		  drawString
//
//************************************************************************
void drawString(ContextID contextID, char *text, int textlen,
		Rect<CoordType> &rect,
		int boxpos, float);

//************************************************************************
//
//	  drawBoxWithXOr
//
//************************************************************************

void drawBoxWithXOr(Rect<int> *box);

//************************************************************************
//
//                flush
//
//************************************************************************
void flush();


//************************************************************************
//
//                sync
//
//************************************************************************
void sync();


//************************************************************************
//
//	    updateDisplay
//
//************************************************************************
void updateDisplay();

};// DrawArea

extern DrawArea *theDrawArea;

#endif

