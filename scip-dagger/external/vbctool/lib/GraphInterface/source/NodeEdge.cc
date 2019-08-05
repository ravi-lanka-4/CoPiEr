/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      NodeEdge.cc
//
//      Version  :      08.02.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////

#include "DrawArea.h"
#include "Context.h"
#include "NodeInformation.h"
#include "NodeEdge.h"
#include "GraphInterface.h"
#include <assert.h>

EdgeCategorie::EdgeCategorie(char *customName, char *xName, char *outerCustomName,
			     CoordType width, Boolean dashed, Boolean locked)
{
   _locked = locked;
   
   _outer = theDrawArea->createNewContext(xName, outerCustomName);

   if (customName) _customName = strdup(customName);
   else _customName = NULL;
      
   theDrawArea->lockContext(_outer);
   
   Context *c = theDrawArea->getContext(_outer);
   c->setLineWidth(width);
   if (dashed)
      c->setLineStyle(LineOnOffDash);
}

NodeCategorie::NodeCategorie(char *customName,
			     char *outerXName, char *outerCustomName,
			     char *innerXName, char *innerCustomName,
			     char *font,
			     float r,
			     Boolean nr, Boolean fill,
			     Boolean circle, Boolean locked) :
			     EdgeCategorie(customName, outerXName, outerCustomName,
					   0.0, False, locked)
{
   if (font)
      _inner = theDrawArea->createNewContext(innerXName, innerCustomName, font);
   else
      _inner = theDrawArea->createNewContext(innerXName, innerCustomName);
      
   theDrawArea->lockContext(_inner);

   _ratio = r;
   _withNumbers = nr;
   _filled = fill;
   _circle = circle;

   _radius = 0.0;
}

void NodeCategorie::draw(int n, CoordType x, CoordType y, NodeCategorie *other)
{
   CoordType r2 = _radius * _ratio;

   if(_circle)
   {
      if(_filled) {
	 theDrawArea->fillCircle(_outer, x, y, _radius);
	 if (other)
	    theDrawArea->fillCircle(other->_outer, x, y, r2);
      }
      else {
	 theDrawArea->drawCircle(_outer, x, y, _radius);
	 if (other)
	    theDrawArea->drawCircle(other->_outer, x, y, r2);
      }
   }
   else
   {
      if(_filled) {
	 theDrawArea->fillRectangle(_outer, x-_radius, y-_radius, 2*_radius, 2*_radius);
	 if (other)
	    theDrawArea->fillRectangle(other->_outer, x-r2, y-r2, 2*r2, 2*r2);
      }
      else {
	 theDrawArea->drawRectangle(_outer, x-_radius, y-_radius, 2*_radius, 2*_radius);
	 if (other)
	    theDrawArea->drawRectangle(other->_outer, x-r2, y-r2, 2*r2, 2*r2);
      }
   }

   if(_withNumbers)
   {
      static char buf[16];

      sprintf(buf, "%d", theGraphInterface->graph()->maxNodes());
      int maxlen = strlen(buf);
	    
      sprintf(buf, "%d", n);

      Rect<CoordType> r(x - _radius, y - _radius, 2*_radius, 2*_radius);
      if (_circle) maxlen -= 2;
      theDrawArea->drawString(_inner, buf, maxlen, r, 5);
   }
}

void NodeCategorie::erase(int n, CoordType x, CoordType y)
{
   Context *c1 = theDrawArea->getContext(_outer);
   Context *c2 = theDrawArea->getContext(_inner);

   c1->backgroundToPixelValue();
   c2->backgroundToPixelValue();

   draw(n, x, y, NULL);

   c1->restoreOldPixelValue();
   c2->restoreOldPixelValue();
}

void NodeCategorie::mxor(int n, CoordType x, CoordType y)
{
   CoordType rad = _radius * 1.7;
   Rect<CoordType> h;
   theDrawArea->getActualCoordRange(h);
   CoordType rad2 = maximum(h.height(), h.width());
   rad2 = rad2/200;
   rad = maximum(rad2, rad);
   
   if (_circle)
      theDrawArea->fillCircle(XORID, x, y, rad);
   else
      theDrawArea->fillRectangle(XORID, x-rad, y-rad, 2*rad, 2*rad);
}




Node::Node(int nr, CoordType x, CoordType y, NodeCategorie *cat,
	   Boolean drawn, NodeCategorie* other)
{
   _other = other;
   _number = nr;
   _x = x;
   _y = y;
   _categorie = cat;
   _drawn = drawn;
}




Edge::Edge(int f, int t, EdgeCategorie *cat)
{
   _from = f;
   _to   = t;
   _categorie = cat;
}

void Edge::draw(NodeInformation *ni)
{
   assert(ni);

   CoordType x1, x2, y1, y2;

   x1 = ni->getNode(_from)->x();
   y1 = ni->getNode(_from)->y();
   x2 = ni->getNode(_to)->x();
   y2 = ni->getNode(_to)->y();
   
   theDrawArea->drawLine(_categorie->_outer, x1, y1, x2, y2);
}

void Edge::erase(NodeInformation *ni)
{
   assert(ni);
   
   Context *c1 = theDrawArea->getContext(_categorie->_outer);

   c1->backgroundToPixelValue();

   draw(ni);

   c1->restoreOldPixelValue();
}

void Edge::mxor(NodeInformation *ni)
{
   assert(ni);
}



Edge2::Edge2(CoordType x1, CoordType y1, CoordType x2, CoordType y2, EdgeCategorie *ec)
{
   _x1 = x1;
   _x2 = x2;
   _y1 = y1;
   _y2 = y2;
   _categorie = ec;
}

void Edge2::draw()
{
   theDrawArea->drawLine(_categorie->_outer, _x1, _y1, _x2, _y2);
}

void Edge2::erase()
{
   Context *c1 = theDrawArea->getContext(_categorie->_outer);

   c1->backgroundToPixelValue();

   draw();

   c1->restoreOldPixelValue();
}

void Edge2::mxor()
{
}
