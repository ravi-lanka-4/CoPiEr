/////////////////////////////////////////////////////////////////////// 
//
// Filename :  Geometric.h
//
// Version  :  07.02.94
//
// Author   :  Martin Diehl & Joachim Kupke
//
// Language :  C++ 
//
// Purpose  :  Point, Vector, Rect and Line class to simplify
//             geometrical computations.
//             And a Scaler class that can convert coordinate
//             regions into each other.
//
////////////////////////////////////////////////////////////////////////
#ifndef GEOMETRIC_H
#define GEOMETRIC_H

//typedef void* XtPointer;

#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>

#include <X11/Intrinsic.h>
#include "UsefulInlines.h"

#define ScalerYInvert   True
#define ScalerNoYInvert False
#define ROUND_TARGET 1
#define ROUND_SOURCE 2
#define NO_ROUNDING     0

class GeometricItem

// Abstract base class for all geometric items
{
   protected:
      Boolean _empty;   // signals if this Item is currently valid
                         // or still empty
   public:
      virtual void print() = 0;
      Boolean isEmpty() { return _empty;}
      void emptyIt() { _empty = True; }
};// GeometricItem


template<class T> class PointItem : public GeometricItem
// An abstract base class for all geometric items, that have the propertie
// of containing at least one point, which is given by x() and y().
// The type of the coordinates is the templates parameter.
{
   protected:
      T _x;  // X-Coordinate of the item's origine
      T _y;  // Y-Coordinate of the item's origine

   public:
      PointItem() // empty constructor
      {
             _x=_y=0;
             _empty = True;
      }

      PointItem(T x, T y)
      {
             _x = x;
             _y = y;
             _empty = False;
      }

      virtual ~PointItem()
      {
      }
      
      T x() { return _x;}

      T y() { return _y;}

      void setX(T x) { _x = x;}

      void setY(T y) { _y = y;}

      void moveToXY(T x, T y)
      {
          setX(x);
          setY(y);
      }

}; // PointItem

template<class T> class Point : public PointItem<T>
{
   public:
   
      Point()           : PointItem<T>() {}
      Point(T x, T y)   : PointItem<T>(x,y) {}
      Point(Point<T> &p): PointItem<T>(p.x(), p.y()) {}

      double distanceToZero()
      {
          return (double)sqrt((double) this->x()*this->x() +
                                    (double) this->y()*this->y());
      }

      void newInit(Point<T> *p)
      {
             this->setX(p->x());
             this->setY(p->y());
	     GeometricItem::_empty = False;
      }

      void newInit(T x, T y)
      {
             this->setX(x);
             this->setY(y);
	     GeometricItem::_empty = False;
      }

      virtual void print()
      {
	 double x = (double) PointItem<T>::_x;
	 double y = (double) PointItem<T>::_y;
	 if (!GeometricItem::_empty)
	   printf("x: %f y: %f\n",x,y);
	 else
	   printf("This Point is empty!\n");
      }
}; // Point


template<class T> class Vector : public PointItem<T>
{
   public:

      Vector()         : PointItem<T>() {}
      Vector(T x, T y) : PointItem<T>(x,y) {}
      Vector(Vector<T> &v) : PointItem<T>(v.x(), v.y()){}

      double length()
      {
          return (double) sqrt((double) this->x()*this->x() +
                               (double) this->y()*this->y());
      }

      virtual void print()
      {
	double x = (double) PointItem<T>::_x;
	double y = (double) PointItem<T>::_y;
	if (!GeometricItem::_empty)
	  printf("x: %f y: %f\n",x,y);
	else
	  printf("This Vector is empty!\n");
      }

      void newInit(T x, T y)
      {
         this->setX(x);
         this->setY(y);
	 GeometricItem::_empty = False;
      }

      void normalRight()
      {
         this->setX(this->y());
         this->setY(-this->x());
      }

      void normalLeft()
      {
         this->setX(-this->y());
         this->setY(this->x());
      }
      
}; // Vector


template<class T> class RectItem : public GeometricItem
// An abstract base class for all descendants of GeometricItem,
// which have a stretch in the plane and are discribable by two
// points.
{
   protected:
   
      Point<T> *_firstPoint;
      Point<T> *_secondPoint;


      // Make sure that firstPoint.x is <= secondPoint.x and
      // firstPoint.y <= secondPoint.y
      void normalizeRect()
      {
             T help;
             if ( _firstPoint->x() > _secondPoint->x() )
             {
                  help = _secondPoint->x();
                  _secondPoint->setX(_firstPoint->x());
                  _firstPoint->setX(help);
             }
             if ( _firstPoint->y() > _secondPoint->y() )
             {
                  help = _secondPoint->y();
                  _secondPoint->setY(_firstPoint->y());
                  _firstPoint->setY(help);
             }
      }
   

   public:

      RectItem()
      {
             _firstPoint = new Point<T>();
             _secondPoint = new Point<T>();
             _empty = True;
      }

      RectItem(T x1, T y1, T x2, T y2)
      {
             _firstPoint   = new Point<T>(x1,y1);
             _secondPoint  = new Point<T>(x2, y2);
        if ((x1==x2) || (y1==y2))
           _empty = True;
        else
                 _empty = False;
             normalizeRect();
      }

      Point<T> &upperRightPoint()
      {
             return *_secondPoint;
      }

      Point<T> &lowerLeftPoint()
      {
             return *_firstPoint;
      }
      
      T width() const { return (_secondPoint->x() - _firstPoint->x());}
      T height() const { return (_secondPoint->y() - _firstPoint->y());}
      T maxX() const { return (_secondPoint->x());}
      T maxY() const { return (_secondPoint->y());}
      T minX() const { return (_firstPoint->x());}
      T minY() const { return (_firstPoint->y());}

      virtual ~RectItem()
      {
             delete (_firstPoint);
             delete (_secondPoint);
      }
}; // RectItem

template<class T> class Rect: public RectItem<T>
{   
   public:

      Rect()                                  : RectItem<T>() {}

      Rect(T x, T y,
           T width, T height) : RectItem<T>(x, y,
                                            x+width, y+height){}
      Rect(Point<T> &p1,
           Point<T> &p2)      : RectItem<T>(p1.x(), p1.y(),
                                            p2.x(), p2.y()){}

      Rect(const Rect<T> &r)        : RectItem<T>(r.minX(), r.minY(),
                                            r.maxX(), r.maxY()){}

      void newInit(Rect<T> *r)   // Make this to a copy of r
      {
         Point<T> lp(r->lowerLeftPoint());
         Point<T> rp(r->upperRightPoint());
	 RectItem<T>::_firstPoint->newInit(&lp);
	 RectItem<T>::_secondPoint->newInit(&rp);
	 GeometricItem::_empty = False;
         this->normalizeRect();
      }

      void newInit(T x, T y, T width, T height)
      {
	RectItem<T>::_firstPoint->newInit(x,y);
	RectItem<T>::_secondPoint->newInit(x+width, y+height);
	GeometricItem::_empty = False;
	this->normalizeRect();
      }

      void setLowerLeftPoint(T x, T y)
      {
	RectItem<T>::_firstPoint->newInit(x,y);
	this->normalizeRect();
      }
      void setUpperRightPoint(T x, T y)
      {
	RectItem<T>::_secondPoint->newInit(x,y);
	this->normalizeRect();
      }

      void newInit(Point<T> *p1, Point<T> *p2)
      {
	RectItem<T>::_firstPoint->newInit(p1);
	RectItem<T>::_secondPoint->newInit(p2);
	GeometricItem::_empty = False;
	this->normalizeRect();
      }

      void moveRect(T dx, T dy)   // moves the Coordinates of the Rect
      {
	RectItem<T>::_firstPoint->newInit(RectItem<T>::_firstPoint->x()+dx, RectItem<T>::_firstPoint->y()+dy);
	RectItem<T>::_secondPoint->newInit(RectItem<T>::_secondPoint->x()+dx, RectItem<T>::_secondPoint->y()+dy);
      }
      
      virtual void print()
      {
             if (GeometricItem::_empty)
                printf("This Rect is empty!\n");
             else
             {
                  printf("The two max points of the Rect are:\n");
		  RectItem<T>::_firstPoint->print();
		  RectItem<T>::_secondPoint->print();
             }
      }

      void growPercent(double p)
      {
         // makes the Rect growing in size
         // the Rect is p% wider an p% higher after
         // this operation.
         // It is centered over the old position.
         
             double percentFromWidth;
             double percentFromHeight;

             percentFromWidth  = ( (double) this->width()* (p/2.0))/100.0;
             percentFromHeight = ( (double) this->height()*(p/2.0))/100.0;

             T newX1 = (T) (RectItem<T>::_firstPoint->x()-percentFromWidth);
             T newX2 = (T) (RectItem<T>::_secondPoint->x()+percentFromWidth);
             T newY1 = (T) (RectItem<T>::_firstPoint->y()-percentFromHeight);
             T newY2 = (T) (RectItem<T>::_secondPoint->y()+percentFromHeight);
         
             RectItem<T>::_firstPoint->newInit(newX1, newY1);
             RectItem<T>::_secondPoint->newInit(newX2, newY2);
      }
         
         
         
      double ratio()
      {
          if (!GeometricItem::_empty) return (double) (double)this->width()/(double)this->height();
          return -1;
      }
      
      void convertToRatio(double givenRatio)
      // Change my width and, or height such that I'll have the
      // given width to height ratio.
      // I'm greater or equal then now after this function.
      //
      ///////////////////////////////////////////////////////////////
      //
      //       CAUTION !!
      //
      // When this is a Rect<int> the converted Rect<int> may not be
      // correct, because it may be that the correct one would
      // have fractional width, height, x or y values !!
      //
      ///////////////////////////////////////////////////////////////
      {
             if (ratio() == givenRatio)
             return;
             if (ratio() < givenRatio)
             {
                  T newWidth    = (T) (givenRatio * (double) this->height());
                  T widthDiff   = newWidth - this->width();
                  T newX1       = this->minX() - (T)(widthDiff/2.0);
                  T newX2       = this->maxX() + (T)(widthDiff/2.0);
                  RectItem<T>::_firstPoint->setX(newX1);
                  RectItem<T>::_secondPoint->setX(newX2);
              }
              else
              {    
                   T newHeight  = (T) ((double) this->width() / (double) givenRatio);
                   T heightDiff = newHeight - this->height();
                   T newY1     = this->minY() - (T)(heightDiff/2.0);
                   T newY2     = this->maxY() + (T)(heightDiff/2.0);
                   RectItem<T>::_firstPoint->setY(newY1);
                   RectItem<T>::_secondPoint->setY(newY2);
              }
      }

      Rect<T> &operator = (Rect<T> r)
      {
         if (this != &r)   // don't assign myself
         {
            if (!r.isEmpty())
               this->newInit(&r);
         }
         return *this;
      }

      
}; // Rect

template<class T> class LineItem : public GeometricItem
// An abstract base class for all descendants of GeometricItem,
// which have a stretch in the plane and are discribable by two
// points.
{
   protected:
   
      Point<T> *_firstPoint;
      Point<T> *_secondPoint;

      void normalizeLine()
      {
         if (x1() > x2())
         {
            Point<T> *h = _firstPoint;
            _firstPoint = _secondPoint;
            _secondPoint = h;
         }
      }

   public:

      LineItem()
      {
             _firstPoint = new Point<T>();
             _secondPoint = new Point<T>();
             GeometricItem::_empty = True;
      }

      LineItem(T x1, T y1, T x2, T y2)
      {
             _firstPoint   = new Point<T>(x1,y1);
             _secondPoint  = new Point<T>(x2, y2);
             GeometricItem::_empty = False;
             normalizeLine();
      }

      Point<T> &endPoint()
      {
             return *_secondPoint;
      }

      Point<T> &startPoint()
      {
             return *_firstPoint;
      }
      
      T dx() { return (_secondPoint->x() - _firstPoint->x());}
      T dy() { return (_secondPoint->y() - _firstPoint->y());}
      T x2() { return (_secondPoint->x());}
      T y2() { return (_secondPoint->y());}
      T x1() { return (_firstPoint->x());}
      T y1() { return (_firstPoint->y());}

      virtual ~LineItem()
      {
             delete (_firstPoint);
             delete (_secondPoint);
      }
}; // LineItem

template<class T> class Line : public LineItem<T>
{
   public:

      Line()   : LineItem<T>() {}
      Line(T x1, T y1, T x2, T y2)      : LineItem<T>(x1, y1, x2, y2){}
      Line(Point<T> &p1, Point<T> &p2)  : LineItem<T>(p1.x(), p1.y(), p2.x(), p2.y()){}
      Line(Line<T> &l) : LineItem<T>(l.x1(), l.y1(), l.x2(), l.y2()) {}

      double length()
      {
             double l;
             l = (double)sqrt( fabs( LineItem<T>::_firstPoint->x() - LineItem<T>::_secondPoint->x()) +
                                     fabs( LineItem<T>::_firstPoint->y() - LineItem<T>::_secondPoint->y()));
             return l;
      }
      
      void newInit(Line<T> *l)   // Make this to a copy of l
      {
             LineItem<T>::_firstPoint->newInit(&l->startPoint());
             LineItem<T>::_secondPoint->newInit(&l->endPoint());
             GeometricItem::_empty = False;
             this->normalizeLine();
      }

      void newInit(T x1, T y1, T x2, T y2)   // Make this to a copy of l
      {
	     LineItem<T>::_firstPoint->newInit(x1, y1);
             LineItem<T>::_secondPoint->newInit(x2, y2);
             GeometricItem::_empty = False;
             this->normalizeLine();
      }


      virtual void print()
      {
             if (GeometricItem::_empty)
             printf("This Line is empty!\n");
             else
             {
                  printf("The two end points of the Line are:\n");
                  LineItem<T>::_firstPoint->print();
                  LineItem<T>::_secondPoint->print();
             }
      }

      Line<T>& operator = (Line<T>& r)
      {
         if (this != &r)   // don't assign myself
         {
            if (!r.isEmpty())
               this->newInit(&r);
         }
         return *this;
      }

}; // Line



template<class T1, class T2> class Scaler
// A class that scales the coordinate range of one Rect
// to the coord range of another one.
{
   private:
      double   _factor, _yFactor;
      double   _xOffset, _yOffset;
      Boolean  _yInvert;               // Convert the y-coordinate
      Boolean  _empty;
      double   _roundingT1;
      double   _roundingT2;

      double round(double x, double r)
      {
         if (x >= 0.0) return (x + r);
         else          return (x - r);
      }

   public:

      Scaler()
      {
         _empty   = True;
         _factor  = 0;
         _yFactor = 0;
         _xOffset = 0;
         _yOffset = 0;
         _yInvert = False;
         _roundingT1 = _roundingT2 = 0.0;
      }

      Scaler(Rect<T1>   &sourceRect,
             Rect<T2>   &targetRect,
             Rect<T1>   &actualSourceRect,
             Boolean    yInvert,
             Boolean    rounding)
      {
         _roundingT1 = _roundingT2 = 0.0;
         switch (rounding)
         {
            case ROUND_TARGET : _roundingT2 = 0.5;
                                break;

            case ROUND_SOURCE : _roundingT1 = 0.5;
         }
         
         if (sourceRect.isEmpty() || targetRect.isEmpty())
         {
            _empty = True;
            _factor  = 0;
            _yFactor = 0;
            _xOffset = 0;
            _yOffset = 0;
            _yInvert = False;
         }
         else
         {
            _empty = False;
            _yInvert = yInvert;
            _factor  = 0;
            _yFactor = 0;
            _xOffset = 0;
            _yOffset = 0;
         
             actualSourceRect = sourceRect;
             actualSourceRect.convertToRatio(targetRect.ratio());
         
             _factor  = (double)((double)targetRect.width() /
                                 (double)actualSourceRect.width());

             _xOffset = (double)((double) -actualSourceRect.minX() *_factor
                                         +(double) targetRect.minX());

             if (_yInvert)
             {
                  _yOffset = (double)((double) targetRect.maxY() +
                                      (double) actualSourceRect.minY() * _factor);
                  _yFactor = -_factor;
             }
             else
             {
                  _yOffset = (double)((double) -actualSourceRect.minY() * _factor
                                              +(double) targetRect.minY());
                  _yFactor = _factor;
             }
         }
      }

      Boolean isEmpty() {return _empty;}

      double factor() {return _factor;}

      void print()
      {
         printf("yInvert : %d\n",_yInvert);
         printf("X Factor: %f\n",_factor);
         printf("Y Factor: %f\n",_yFactor);
         printf("X Offset: %f\n",_xOffset);
         printf("Y Offset: %f\n",_yOffset);
     }
         
      double scalingFactor()
      {
         return _factor;
      }

      T2 xToTarget(T1 x)
      {
         return (T2) round(x * _factor + _xOffset, _roundingT2);
      }
         
      T2 yToTarget(T1 y)
      {
             return (T2) round(y * _yFactor + _yOffset, _roundingT2);
      }

      void convertPoint(Point<T1> *sourcePoint, Point<T2> *targetPoint)
      {
           targetPoint->newInit( xToTarget(sourcePoint->x()),
                                 yToTarget(sourcePoint->y()));
      }

      void convertRect(Rect<T1> *sourceRect, Rect<T2> *targetRect)
      {
         Point<T2> p1, p2;
         Point<T1> sp1(sourceRect->lowerLeftPoint());
         Point<T1> sp2(sourceRect->upperRightPoint());

          convertPoint( &sp1, &p1);
          convertPoint( &sp2, &p2);

          targetRect->newInit(&p1, &p2);
      }

    T2 scale(T1 s)
         // scales the value s to the Target value S
         // different from xToTarget or yToTarget
         // because scale only use the scaling
         // factor and NOT any of the offsets.
         // This is function can be used e.g for the scaling
         // of the radius of a circle but NOT for scaling a
         // x or y value
    {
              return (T2) round(s*_factor, _roundingT2);
    }

         
      T1 xReConvert(T2 x)
      {
         return (T1) round( (double)(x-_xOffset) / double(_factor), _roundingT1);
      }


      T1 yReConvert(T2 y)
      {
         return (T1) round( (double)(y-_yOffset) / (double)_yFactor, _roundingT1);
      }

      T1 reConvert(T2 s)
      {
              return (T1) round(s/_factor, _roundingT1);
      }
                                                                              
      void reConvertPoint(Point<T2> *sourcePoint, Point<T1> *targetPoint)
      {
         targetPoint->newInit(xReConvert(sourcePoint->x()),
                              yReConvert(sourcePoint->y()));
      }

      
      void reConvertRect(Rect<T2> *sourceRect, Rect<T1> *targetRect)
      {
              Point<T1> tp1,tp2;
              Point<T2> sp1(sourceRect->lowerLeftPoint());
              Point<T2> sp2(sourceRect->upperRightPoint());

              reConvertPoint( &sp1, &tp1);
              reConvertPoint( &sp2, &tp2);
         
              targetRect->newInit(&tp1, &tp2);
      }

      Boolean yInvert() { return _yInvert; }
      int     yInvertFaktor() { return (_yInvert ? 1 : 0); }
      
}; // Scaler

////////////////////////////////////////////////////////////////////////
//
//          Operator overloading
//
////////////////////////////////////////////////////////////////////////
      
template<class T1, class T2> inline Boolean operator ==(Point<T1> &p1, Point<T2> &p2)
{
   if ( (p1.x() == (T1) p2.x() ) && ( p1.y() == (T1) p2.y() ) )
      return True;
   else
      return False;
}

template<class T> inline Point<T> &operator +(Point<T> &p1, Point<T> &p2)
{
   Point<T> p(p1.x()+p2.x(), p1.y()+p2.y());
   return p;
}

template<class T> inline Vector<T> &operator -(Point<T> &p1, Point<T> &p2)
{
   Vector<T> v( p1.x()-p2.x(), p1.y()-p2.y());
   return v;
}

template<class T> inline Vector<T> &operator +(Vector<T> &v1, Vector<T> &v2)
{
   Vector<T> v( v1.x()+v2.x(), v1.y()+v2.y() );
   return v;
}

template<class T> inline T operator *(Vector<T> &v1, Vector<T> &v2)
// Scalarproduct
{
   return (v1.x()*v2.x() + v1.y()*v2.y());
}

template<class T1, class T2> inline Boolean operator <(Point<T1> &p, Rect<T2> &r)
// returns True if p lies properly in r
{
   if (  ( ((T2) p.x())>r.minX() ) &&
         ( ((T2) p.x())<r.maxX() ) &&
         ( ((T2) p.y())>r.minY() ) &&
         ( ((T2) p.y())<r.maxY() )
       )
      return True;
   else
      return False;
}

template<class T1, class T2> inline Boolean operator >(Rect<T1> &r, Point<T2> &p)
// returns True if r contains p properly
{
   return (p<r);
}

template<class T1, class T2> inline Boolean operator <=(Point<T1> &p, Rect<T2> &r)
// returns True if p lies  in r
{
   if (  ( ((T2) p.x())>=r.minX() ) &&
         ( ((T2) p.x())<=r.maxX() ) &&
         ( ((T2) p.y())>=r.minY() ) &&
         ( ((T2) p.y())<=r.maxY() )
       )
      return True;
   else
      return False;
}

template<class T1, class T2> inline Boolean operator >=(Rect<T1> &r, Point<T2> &p)
// returns True if r contains p
{
   return (p<=r);
}

template<class T1, class T2> inline Boolean operator <=(Rect<T1> &r1, Rect<T2> &r2)
// returns True if r1 lies in r2
{
   if (  ( ((T2) r1.minX()) >= r2.minX() ) &&
         ( ((T2) r1.maxX()) <= r2.maxX() ) &&
         ( ((T2) r1.minY()) >= r2.minY() ) &&
         ( ((T2) r1.maxY()) <= r2.maxY() )
       )
      return True;
   else
      return False;
}

template<class T1, class T2> inline Boolean operator <(Rect<T1> &r1, Rect<T2> &r2)
// returns True if r1 lies properly in r2
{
   if (  (r1 <= r2) &&                 // to lie properly inside another rect means
         (                             // to be contained by it and to have at least 
          ( ((T2) r1.minX()) > r2.minX() ) || // one border tighter than the other one.
          ( ((T2) r1.maxX()) < r2.maxX() ) ||
          ( ((T2) r1.minY()) > r2.minY() ) ||
          ( ((T2) r1.maxY()) < r2.maxY() )
         )
       )
      return True;
   else
      return False;
}

template<class T1, class T2> inline Boolean operator >(Rect<T1> &r1, Rect<T2> &r2)
// returns True if r1 contains r2 properly
{
   return (r2<r1);
}

template<class T1, class T2> inline Boolean operator >=(Rect<T1> &r1, Rect<T2> &r2)
// returns True if r1 contains r2
{
   return (r2<=r1);
}

template<class T1, class T2> inline Boolean operator /(Rect<T1> &r1, Rect<T2> &r2)
// returns True if r1 intersects r2
{
   if (  ( ((T2) r1.maxX()) > r2.minX() ) &&
         ( ((T2) r1.minX()) < r2.maxX() ) &&
         ( ((T2) r1.maxY()) > r2.minY() ) &&
         ( ((T2) r1.minY()) < r2.maxY() )
       )
      return True;
   else
      return False;
}

template<class T1, class T2> inline Boolean operator /(Line<T1> &l, Rect<T2> &r)
// returns True if l intersects r
{
   if ((l.startPoint() <= r) || (l.endPoint() <= r)) return True;

   if ((((T2) l.x2()) < r.minX()) || (((T2) l.x1()) > r.maxX()) ||
       (((T2) minimum(l.y1(), l.y2())) > r.maxY()) ||
       (((T2) maximum(l.y1(), l.y2())) < r.minY()))
      return False;

   const double min = 1/sqrt(DBL_MAX);

   double dx = l.dx();
   double dy = l.dy();

   if (fabs(dx) > min)
   {
      double a = dy/dx;
      double t = l.y1() - a*l.x1();
      double y1 = a*r.minX() + t;
      double y2 = a*r.maxX() + t;

      if (((T2) minimum(y1, y2)) < r.maxY() && ((T2) maximum(y1, y2)) > r.minY())
         return True;
      else
         return False;
   }

   Rect<T2> rect(l.startPoint(), l.endPoint());
   return (rect/r);
}

template<class T> inline Line<T> &operator ^(Line<T> &l, Rect<T> &r)
// return the intersection Line or l and r
{
   Point<T> sp(l.startPoint());
   Point<T> ep(l.endPoint());

   static Line<T> ret;

   if ((sp <= r) && (ep <= r)) return l;

   if ((( l.x2()) < r.minX()) || (( l.x1()) > r.maxX()) ||
       (( minimum(l.y1(), l.y2())) > r.maxY()) ||
       (( maximum(l.y1(), l.y2())) < r.minY())) {
         return ret;
   }

   double dx = (double) l.dx();
   double dy = (double) l.dy();

   ret.newInit(&l);

   if (fabs(dx) >= fabs(dy)) {
      double a = dy / dx;
      double t = l.y1() - a*l.x1();

      if (ret.x1() < r.minX())
         ret.newInit(r.minX(), a * r.minX() + t, ret.x2(), ret.y2());

      if (ret.x2() > r.maxX())
         ret.newInit(ret.x1(), ret.y1(), r.maxX(), a * r.maxX() + t);

      if (ret.y1() <= ret.y2()) {
         if (ret.y1() < r.minY())
            ret.newInit((r.minY() - t) / a, r.minY(), ret.x2(), ret.y2());
         if (ret.y2() > r.maxY())
            ret.newInit(ret.x1(), ret.y1(), (r.maxY() - t) / a, r.maxY());
      }
      else {
         if (ret.y1() > r.maxY())
            ret.newInit((r.maxY() - t) / a, r.maxY(), ret.x2(), ret.y2());
         if (ret.y2() < r.minY())
            ret.newInit(ret.x1(), ret.y1(), (r.minY() - t) / a, r.minY());
      }
   }
   else {
      double a = dx / dy;
      double t = l.x1() - a*l.y1();

      if (ret.y1() <= ret.y2()) {
         if (ret.y1() < r.minY())
            ret.newInit(r.minY() * a + t, r.minY(), ret.x2(), ret.y2());
         if (ret.y2() > r.maxY())
            ret.newInit(ret.x1(), ret.y1(), r.maxY() * a + t, r.maxY());
      }
      else {
         if (ret.y1() > r.maxY())
            ret.newInit(r.maxY() * a + t, r.maxY(), ret.x2(), ret.y2());
         if (ret.y2() < r.minY())
            ret.newInit(ret.x1(), ret.y1(), r.minY() * a + t, r.minY());
      }

      if (ret.x1() < r.minX())
         ret.newInit(r.minX(), (r.minX() - t) / a, ret.x2(), ret.y2());

      if (ret.x2() > r.maxX())
         ret.newInit(ret.x1(), ret.y1(), r.maxX(), (r.maxX() - t) / a);
   }

   return ret;
}

template<class T> inline Rect<T> operator ^(Rect<T> &r1, Rect<T> &r2)
// returns the intersection Rect of r1 and r2
{
   if (r1/r2)
   {
      Point<T> llp( maximum( r1.minX(),  r2.minX()),
                     maximum( r1.minY(), r2.minY()));
      Point<T> urp( minimum( r1.maxX(), r2.maxX()),
                     minimum( r1.maxY(), r2.maxY()));
      Rect<T> r(llp, urp);
      return r;
   }
   else
   {
      Rect<T> r;
      return r;
   }
}

template<class T1, class T2> inline Boolean operator ==(Rect<T1> &r1, Rect<T2> &r2)
// returns True if r1=r2
{
   Point<T1> llp1(r1.lowerLeftPoint());
   Point<T2> llp2(r2.lowerLeftPoint());
   Point<T1> urp1(r1.upperRightPoint());
   Point<T2> urp2(r2.upperRightPoint());

   if
   (
    (  llp1 == llp2 ) &&
    (  urp1 == urp2 )
   )
       return True;
   else
       return False;
}

template<class T1, class T2> inline Boolean operator ==(Line<T1> &l1, Line<T2> &l2)
// returns True if l1=l2
{
   if
   (
    (  l1.startPoint() == l2.startPoint() ) &&
    (  l1.endPoint()   == l2.endPoint()   )
   )
      return True;
   else
      return False;
}

template<class T> inline Rect<T>* operator -(Rect<T> &R, Rect<T> &r)
// returns an array of pointers to Rect<T>'s. The end
// of the array is marked by an NULL-pointer.
// The array holds the pointer to a disjoint set of
// Rect<T>'s which union builds the set R-r.

{
   Rect<T> *list = new Rect<T>[4]; // create four empty rects.
   
   if (r>=R)
   {
      return list;          // (R-r) is the empty set.
   }

   if (!(r/R))  // There is no intersection.
   {
     list[0]=R;
     return list;
   }

   Rect<T> intersectionRect;
   intersectionRect = R ^ r;

   // intersectionRect lies properly in R, so determine (R - intersectionRect) = R - r

   int i = 0; // start with Rect 0
   T   helpVar;

   if ((helpVar = intersectionRect.minX() - R.minX()) != (T) 0)
      list[i++].newInit(R.minX(),
                        R.minY(),
                        helpVar,
                        R.height());

   if ((helpVar = R.maxY() - intersectionRect.maxY()) != (T) 0)
      list[i++].newInit(intersectionRect.minX(),
                        intersectionRect.maxY(),
                        intersectionRect.width(),
                        helpVar);

   if ((helpVar = intersectionRect.minY() - R.minY()) != (T) 0)
      list[i++].newInit(intersectionRect.minX(),
                        R.minY(),
                        intersectionRect.width(),
                        helpVar);
      
   if ((helpVar = R.maxX() - intersectionRect.maxX()) != (T) 0)
      list[i].newInit(intersectionRect.maxX(),
                      R.minY(),
                      helpVar,
                      R.height());

   return list;

} // operator -

#endif

