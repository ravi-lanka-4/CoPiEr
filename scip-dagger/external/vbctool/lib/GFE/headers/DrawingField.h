/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      DrawingField.h
//
//      Version  :      19.09.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////
#ifndef DRAWINGFIELD_H
#define DRAWINGFIELD_H

#include <X11/X.h>
#include <Xm/DrawingA.h>

class DrawingField
{
   private:

      Widget	     _da;
      unsigned long  _pv;
      Colormap	     _cm;
      GC	     _gc;

   protected:

   public:

      DrawingField(Widget, char*, int, int, char*);
      ~DrawingField();

      void drawCircle(int, int, int);
      void fillCircle(int, int, int);

};

#endif
