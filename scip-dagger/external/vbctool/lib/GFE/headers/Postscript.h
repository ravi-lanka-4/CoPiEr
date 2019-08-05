/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	Postscript.h
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

#ifndef POSTSCRIPT_H
#define POSTSCRIPT_H

#include "GFE.inc"
#include "Filenames.h"
#include "DrawArea.h"

class Postscript
{
   private:
   
      FILE		      *_postscriptFile;
      Boolean		      _printFlag;
      Scaler<CoordType, int>  *_paperScaler;

      Rect<int>		      _paperRect;
      Rect<int>		      *_actualPaperSize;

      static Rect<int>	      *_DinAPortrait[];
      static Rect<int>	      *_DinALandscape[];

      static char	      *_landscape;
      static char	      *_portrait;

      static int	      _leftMargin;
      static int	      _rightMargin;
      static int	      _topMargin;
      static int	      _bottomMargin;

      char		      *_orientation;
      Bool		      _withFrame;

      int		      _bbminx;
      int		      _bbminy;
      int		      _bbmaxx;
      int		      _bbmaxy;

      int		      _standardContext;

      char		      *_backgroundColorName;

      static char	      *_prefix;

      void calculateBB(int xx1, int yy1, int xx2, int yy2)
      {
	 _bbmaxx = maximum(_bbmaxx, maximum(xx1, xx2));
	 _bbmaxy = maximum(_bbmaxy, maximum(yy1, yy2));
	 _bbminx = minimum(_bbminx, minimum(xx1, xx2));
	 _bbminy = minimum(_bbminy, minimum(yy1, yy2));
      }

      void getRange(Rect<int> *dinRect)
      {
	 _paperRect.newInit(dinRect->minX()+_leftMargin, dinRect->minY()+_bottomMargin,
			    dinRect->maxX()-_leftMargin-_rightMargin,
			    dinRect->maxY()-_topMargin-_bottomMargin);
	 _actualPaperSize = dinRect;
      }

   protected:
      void openPostscriptFile(char*);
      void closePostscriptFile();
      void createHeader();
      void createContextArrays();
      void testEnd();
      void rectangle(ContextID, Rect<CoordType> &rect);
      void circle(ContextID, CoordType, CoordType, CoordType);

   public:

      Postscript();
      ~Postscript();

      Boolean printFlag() { return _printFlag; }
      
      void setBackgroundColorName(char *n) { _backgroundColorName = strdup(n); }

      void printall(char*);
      void setValues(Bool withFrame, int orientation, int paper);
      void setMargins(char**);
      void getMargins(int &l, int &r, int &t, int &b)
      {
	 l = _leftMargin;
	 r = _rightMargin;
	 t = _topMargin;
	 b = _bottomMargin;
      }


      void setBackground(ColorID);

      void drawLine(ColorID, CoordType, CoordType, CoordType, CoordType);
      void drawLine(ContextID, CoordType, CoordType, CoordType, CoordType);
      
      void fillCircle(ColorID, CoordType, CoordType, CoordType);
      void fillCircle(ContextID, CoordType, CoordType, CoordType);

      void fillRectangle(ContextID, Rect<CoordType> &rect);
      void drawRectangle(ContextID, Rect<CoordType> &rect);
      
      void drawCircle(ColorID, CoordType, CoordType, CoordType);
      void drawCircle(ContextID, CoordType, CoordType, CoordType);

      void drawString(ContextID, const char*, Rect<CoordType> &rect, int, int, float);

      char *postscriptFilename();

};
#endif


   
