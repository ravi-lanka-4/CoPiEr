/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      Postscript.cc
//
//      Version  :      18.08.94
//
//      Author   :      Joachim Kupke
//
//      Language :      C++ 
//
//      Purpose  :      methods for writing Postscript-Files
// 
//
////////////////////////////////////////////////////////////////////////        

#include "ClientInterface.h"
#include "GFEWindow.h"
#include "Postscript.h"
#include "DrawArea.h"
#include "Context.h"
#include "GFEResourceManager.h"

#define PERCENT   -5.0

#define DINA4X    592
#define DINA4Y    840

#define DINA5X    DINA4Y/2
#define DINA5Y    DINA4X
#define DINA6X    DINA5Y/2
#define DINA6Y    DINA5X

#define DINA3X    DINA4Y
#define DINA3Y    DINA4X*2
#define DINA2X    DINA3Y
#define DINA2Y    DINA3X*2


Postscript::Postscript()
{
   _postscriptFile = NULL;
   _printFlag = FALSE;
   _paperScaler = NULL;
   
   getRange(_DinAPortrait[2]);
   _orientation = _portrait;
   _withFrame = False;

   _backgroundColorName = NULL;
   _paperScaler = NULL;
}

Postscript::~Postscript()
{
   kill (_backgroundColorName);
}

void Postscript::openPostscriptFile(char* filename)
{
   if (_postscriptFile) closePostscriptFile();

   _postscriptFile = fopen(filename, "w");
}

void Postscript::closePostscriptFile()
{
   int error = fclose(_postscriptFile);
   _postscriptFile = NULL;

   if (error == EOF) printf("IO-ERROR in Postscript::closePostscriptFile()\n");
}

void Postscript::createHeader()
{
   fprintf(_postscriptFile, "%%!PS-Adobe-3.0\n%%%%Creator: %s\n", "set the Creator");
   fprintf(_postscriptFile, "%%%%Title: %s\n", "set the Title");
   fprintf(_postscriptFile, "%%%%CreationDate: %s\n", "set the Date");
   fprintf(_postscriptFile, "%%%%BoundingBox: (atend)\n");
   fprintf(_postscriptFile, "%%%%Orientation: %s\n", _orientation);
   fprintf(_postscriptFile, "%%%%DocumentFonts: %s\n", "Fonts");
   fprintf(_postscriptFile, "%%%%Pages: 1\n%%%%EndComments\n\n\n");
   fprintf(_postscriptFile, "%%%%Page: 1\n\n");

   if (_orientation == _landscape)
      fprintf(_postscriptFile, "90 rotate\n0 -%d translate\n\n", _actualPaperSize->maxY());

   GFEResourceManager  psSource(HEADERFILENAME);

   psSource.copy(_postscriptFile);

   // initialize Bounding-Box
   _bbmaxx = _bbmaxy = 0;
   _bbminx = _bbminy = INT_MAX;
}


void Postscript::createContextArrays()
{
   fprintf(_postscriptFile, "\n%% create Context Arrays\n\n");

   char *contextValue[] = {"redarray",
                           "greenarray",
                           "bluearray",
                           "linewidtharray",
                           "linestylearray",
                           "capstylearray",
                           "joinstylearray",
                           "fontarray" };

   Context *pointerToContext;
   Color *pointerToColor;

   for (int k = 0; k <= 7; k++)
   {
      fprintf(_postscriptFile, "/%s[\n", contextValue[k]);

      ContextID i=1;
      for (i=1; i<=theDrawArea->_numberRegisteredContexts; i++)
      {
         pointerToContext = theDrawArea->_contextList[i];
         switch (k)
         {
            case 0:  fprintf(_postscriptFile, "%9.6f \t", (double) pointerToContext->RGBred()/65535);
                     break;
            case 1:  fprintf(_postscriptFile, "%9.6f \t", (double) pointerToContext->RGBgreen()/65535);
                     break;
            case 2:  fprintf(_postscriptFile, "%9.6f \t", (double) pointerToContext->RGBblue()/65535);
                     break;
            case 3:  fprintf(_postscriptFile, " %d \t", _paperScaler->scale(pointerToContext->lineWidth()));
                     break;
            case 4:  fprintf(_postscriptFile, " %d \t", pointerToContext->lineStyle());
                     break;
            case 5:  fprintf(_postscriptFile, " %d \t", pointerToContext->capStyle());
                     break;
            case 6:  fprintf(_postscriptFile, " %d \t", pointerToContext->joinStyle());
                     break;
            case 7:  if (pointerToContext->psName())
                        fprintf(_postscriptFile, " /%s findfont \t", pointerToContext->psName());
                     else
                        fprintf(_postscriptFile, " /Courier findfont \t");
                     break;
         }

         fprintf(_postscriptFile, "%% %d\tColor: %s\tName: %s\n", i-1, pointerToContext->xName(), pointerToContext->customName());
      }

      ColorID j=1;
      for (j=1; j<=theDrawArea->_numberRegisteredColors; j++)
      {
         pointerToColor = theDrawArea->_colorList[j];
         switch (k)
         {
            case 0:  fprintf(_postscriptFile, "%9.6f \t", (double) pointerToColor->red()/65535);
                     break;
            case 1:  fprintf(_postscriptFile, "%9.6f \t", (double) pointerToColor->green()/65535);
                     break;
            case 2:  fprintf(_postscriptFile, "%9.6f \t", (double) pointerToColor->blue()/65535);
                     break;
            case 3:  fprintf(_postscriptFile, " 0 \t");
                     break;
            case 4:  fprintf(_postscriptFile, " 0 \t");
                     break;
            case 5:  fprintf(_postscriptFile, " 1 \t");
                     break;
            case 6:  fprintf(_postscriptFile, " 0 \t");
                     break;
            case 7:  fprintf(_postscriptFile, " /Courier findfont \t");
                     break;
         }
         fprintf(_postscriptFile, "%% %d\tColor: %s\tName: %s\n", i+j-2, pointerToColor->xName(), pointerToColor->customName());
      }

      // create an instance with black color and regular context
      switch(k)
      {
         case 0:
         case 1:
         case 2:  fprintf(_postscriptFile, "%9.6f \t", 0.0);
                  break;
         case 3:  fprintf(_postscriptFile, " 0 \t");
                  break;
         case 4:  fprintf(_postscriptFile, " 0 \t");
                  break;
         case 5:  fprintf(_postscriptFile, " 1 \t");
                  break;
         case 6:  fprintf(_postscriptFile, " 0 \t");
                  break;
         case 7:  fprintf(_postscriptFile, " /Courier findfont \t");
                  break;
      }
      fprintf(_postscriptFile, "%% %d\tStandard Context\n", i+j-2);
      _standardContext = i+j-2;
                  
   fprintf(_postscriptFile, "] def\n\n");
   }
}


void Postscript::testEnd()
{
   _paperRect.growPercent(5.2631579);
   Rect<int> bbox1(_bbminx, _bbminy, _bbmaxx - _bbminx, _bbmaxy - _bbminy);
   Rect<int> bbox;
   bbox = (bbox1 ^ _paperRect);

   fprintf(_postscriptFile, "\nshowpage\n");
   fprintf(_postscriptFile, "%%%%BoundingBox: %d %d %d %d\n\n",
                              bbox.minX(),
                              bbox.minY(),
                              bbox.maxX(),
                              bbox.maxY());
}


void Postscript::printall(char *postscriptfilename)
{
   Rect<CoordType>   dummy;

   openPostscriptFile(postscriptfilename);
   if (!_postscriptFile)
   {
      printf("Can't open %s\n", postscriptfilename);
      return;
   }
   
   _printFlag = TRUE;

   Rect<CoordType> tempRect1;
   tempRect1 = (theDrawArea->_actualVisibleRange ^ theDrawArea->_coordinateRange);   
   _paperScaler = new Scaler<CoordType, int>
                     (tempRect1,
                      _paperRect,
                      dummy,
                      !theDrawArea->_yInvert,
                      ROUND_TARGET);
   
   createHeader();
   createContextArrays();

   fprintf(_postscriptFile,"%% Start Painting\n\n");

   fprintf(_postscriptFile, "\n%% Clipping\n");
   fprintf(_postscriptFile, "%d %d %d %d rectclip\n\n", _paperRect.minX(),
                                                        _paperRect.minY(),
                                                        _paperRect.width(),
                                                        _paperRect.height());


   theDrawArea->clearScreen();  // set Background-Color

   if (_withFrame)
   {
      // print Frame if desired
      fprintf(_postscriptFile, "\n%% Frame\n");
      fprintf(_postscriptFile, "%d %d %d %d %d rectangle\n\n", _paperRect.minX(),
                                                               _paperRect.minY(),
                                                               _paperRect.width(),
                                                               _paperRect.height(),
                                                               _standardContext);      

      //if (!_bbmaxx) // BoundingBox is not created yet so _bbmaxx is set to 0
      {
         _bbminx = _paperRect.minX();
         _bbminy = _paperRect.minY();
         _bbmaxx = _paperRect.maxX();
         _bbmaxy = _paperRect.maxY();
      }
      
      _paperRect.growPercent(-5.0);

      kill (_paperScaler);

      Rect<CoordType> tempRect2;
      tempRect2 = (theDrawArea->_actualVisibleRange ^ theDrawArea->_coordinateRange);
      _paperScaler = new Scaler<CoordType, int>
                        (tempRect2,
                         _paperRect,
                         dummy,
                         !theDrawArea->_yInvert,
                         ROUND_TARGET);
   }

   Rect<CoordType> rectList[2];
   rectList[0] = theDrawArea->_coordinateRange;

   for (int i = theDrawArea->_numberOfRepaintCallbacks; i>= 1; i--)
      theDrawArea->_repaintCallbacks[i] (rectList,
                                       theDrawArea->_repaintCallbackClientData[i]);
                                      
   testEnd();
   closePostscriptFile();

   delete _paperScaler;
   
   _printFlag = FALSE;
}

void Postscript::setBackground(ColorID colorID)
{
   if (!colorID) return;

   if (strcmp(theDrawArea->_colorList[colorID]->xName(), _backgroundColorName))
   {
      fprintf(_postscriptFile, "%d %d %d %d %d fillrectangle\t%% set the Background-Color\n\n",
               (_bbminx = _paperRect.minX()) - 1,
               (_bbminy = _paperRect.minY()) - 1,
               (_paperRect.maxX())-_paperRect.minX() + 1,
               (_paperRect.maxY())-_paperRect.minY() + 1,
               colorID + theDrawArea->_numberRegisteredContexts-1);
      _bbmaxx = _paperRect.maxX();
      _bbmaxy = _paperRect.maxY();

      if (!_withFrame) {
         _paperRect.growPercent(-5.0);

         kill (_paperScaler);
         Rect<CoordType> dummy;

         Rect<CoordType> tempRect;
         tempRect = (theDrawArea->_actualVisibleRange ^ theDrawArea->_coordinateRange);
         _paperScaler = new Scaler<CoordType, int>
                           (tempRect,
                            _paperRect,
                            dummy,
                            !theDrawArea->_yInvert,
                            ROUND_TARGET);
      }
   }
}

void Postscript::drawLine (ColorID colorID,
                           CoordType x1,
                           CoordType y1,
                           CoordType x2,
                           CoordType y2)

{
   drawLine ((ContextID)colorID+theDrawArea->_numberRegisteredContexts, x1, y1, x2, y2);
}

void Postscript::drawLine (ContextID contextID,
                           CoordType x1,
                           CoordType y1,
                           CoordType x2,
                           CoordType y2)

{
   int xx1, xx2, yy1, yy2;
   
   fprintf(_postscriptFile, "%d %d %d %d %d line\n", (xx1 = _paperScaler->xToTarget(x1)),
                                                     (yy1 = _paperScaler->yToTarget(y1)),
                                                     (xx2 = _paperScaler->xToTarget(x2)),
                                                     (yy2 = _paperScaler->yToTarget(y2)),
                                                     contextID-1);
   calculateBB (xx1, yy1, xx2, yy2);                                                 
}

void Postscript::fillCircle(ColorID colorID,
                            CoordType x,
                            CoordType y,
                            CoordType radius)
{
   fillCircle ((ContextID) colorID+theDrawArea->_numberRegisteredContexts, x, y, radius);
}

void Postscript::fillCircle(ContextID contextID,
                            CoordType x,
                            CoordType y,
                            CoordType radius)
{
   circle(contextID, x, y, radius);
   fprintf(_postscriptFile, "fillcircle\n");
}

//inline
void Postscript::rectangle(ContextID contextID,
                           Rect<CoordType> &rect)
{
   int x = _paperScaler->xToTarget(rect.minX());
   int y = _paperScaler->yToTarget(rect.minY());
   int w = _paperScaler->scale(rect.maxX()-rect.minX());// - x;
   int h = _paperScaler->scale(rect.maxY()-rect.minY());// - y;

   if (_paperScaler->yInvert())
   {
      h = -h;
//      y = y - h;
   }

   fprintf(_postscriptFile, "%d %d %d %d %d ", x,
                                               y,
                                               w,
                                               h,
                                               contextID-1);
   calculateBB(x, y, x+w, y+h);
}

void Postscript::drawRectangle(ContextID contextID,
                               Rect<CoordType> &rect)
{
      rectangle(contextID, rect);
      fprintf(_postscriptFile, "rectangle\n");
}

void Postscript::fillRectangle(ContextID contextID,
                               Rect<CoordType> &rect)
{
      rectangle(contextID, rect);
      fprintf(_postscriptFile, "fillrectangle\n");
}

void Postscript::drawCircle(ColorID colorID,
                            CoordType x,
                            CoordType y,
                            CoordType radius)
{
   drawCircle((ContextID) colorID + theDrawArea->_numberRegisteredContexts, x, y, radius);
}

void Postscript::drawCircle(ContextID contextID,
                            CoordType x,
                            CoordType y,
                            CoordType radius)
{
         circle(contextID, x, y, radius);
         fprintf(_postscriptFile, "circle\n");
}

//inline
void Postscript::circle(ContextID contextID,
                        CoordType x,
                        CoordType y,
                        CoordType radius)
{
   int xx, yy, rr;
   
      fprintf(_postscriptFile, "%d %d %d %d ", (xx = _paperScaler->xToTarget(x)),
                                               (yy = _paperScaler->yToTarget(y)),
                                               (rr = _paperScaler->scale(radius)),
                                               contextID-1);
   calculateBB(xx+rr, yy+rr, xx-rr, yy-rr);
}


void Postscript::drawString(ContextID contextID, const char* text,
                            Rect<CoordType> &rect,
                            int pos, int len, float tolerance)
{
   int x = _paperScaler->xToTarget(rect.minX());
   int y = _paperScaler->yToTarget(rect.minY());
   int w = _paperScaler->xToTarget(rect.maxX()) - x;
   int h = _paperScaler->yToTarget(rect.maxY()) - y;

   if (_paperScaler->yInvert())
   {
      h = -h;
      y = y - h;
   }

   calculateBB(x, y, x+w, y+h);

   Status st = theDrawArea->_contextList[contextID]->calculateFontSize(text, x, y, w, h, pos, -1,
                                                                     len, tolerance);
   if (st == ERROR)
   {
      fillRectangle(contextID, rect);
      return;
   }

   int dah = theDrawArea->_contextList[contextID]->fontheight();
   CoordType hhh = theDrawArea->_scaler->reConvert(dah);
   int size = _paperScaler->scale(hhh);
   fprintf(_postscriptFile, "(%s) %d %d %d %d %d %d %d drawstring\n", text, x, y, w, h, pos, size, contextID-1);
}

void Postscript::setValues(Bool withFrame, int orientation, int paper)
{
   if (paper == -1) paper = 2; // set Value for DinA4
   
   _withFrame = withFrame;
   if (!orientation)
   {
      _orientation = _portrait;
      getRange(_DinAPortrait[paper]);
   }
   else
   {
      _orientation = _landscape;
      getRange(_DinALandscape[paper]);
   }

}

void Postscript::setMargins(char **text)
{
   float val[4];
   for (int i = 0; i<=3;i++)
      if (sscanf(text[i], "%f", &val[i])<=0) val[i] = 1.0;

   _leftMargin = (int) (val[0]*72.0/2.54 + 0.5);
   _rightMargin = (int) (val[1]*72.0/2.54 + 0.5);
   _topMargin = (int) (val[2]*72.0/2.54 + 0.5);
   _bottomMargin = (int) (val[3]*72.0/2.54 + 0.5);
}

char *Postscript::postscriptFilename()
{
   if (!_prefix)
      _prefix = strdup(theClientInterface->prefix());

   char buf[200];

   strcpy(buf, _prefix);
   strcat(buf, POSTSCRIPTFILENAME);

   return strdup(buf);
}
