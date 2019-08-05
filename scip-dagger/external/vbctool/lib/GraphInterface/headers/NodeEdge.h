/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      NodeEdge.h
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
#ifndef NODEEDGE_H
#define NODEEDGE_H

#include "GFE.inc"

class NodeInformation;
class NodeCategorie;

class EdgeCategorie
{
   friend class Edge;
   friend class Edge2;
   friend class Node;

   protected:

      Boolean	    _locked;
      ContextID	    _outer;
      char	   *_customName;

   public:

      EdgeCategorie(char*, char*, char*, CoordType width = 0.0,
		    Boolean dashed = False, Boolean locked = False);
      ~EdgeCategorie() { delete _customName; }

      ContextID outer() { return _outer; }
      char *customName() { return _customName; }
      Boolean locked() { return _locked; }
};

class NodeCategorie : public EdgeCategorie
{
   protected:

      ContextID	     _inner;

   public:

      float	    _ratio;
      CoordType	    _radius;
      Boolean	    _withNumbers;
      Boolean	    _filled;
      Boolean	    _circle;

      NodeCategorie(char*, char*, char*, char*, char*, char*, float r = 50.0,
		    Boolean nr = False,
		    Boolean fill = False, Boolean circle = True, Boolean locked = False);

      ContextID inner() { return _inner; }

      void draw(int, CoordType, CoordType, NodeCategorie* = NULL);
      void erase(int, CoordType, CoordType);
      void mxor(int, CoordType, CoordType);
};

class Node
{
   friend class NodeInformation;
   friend class NodeQueue;

   private:

      int	     _number;
      CoordType	     _x;
      CoordType	     _y;

      NodeCategorie *_categorie;
      NodeCategorie *_other;
      Boolean	     _drawn;

   public:

      Node(int, CoordType, CoordType, NodeCategorie*,
	   Boolean drawn = True, NodeCategorie *other = NULL);

      void draw()  { if (_drawn) _categorie->draw (_number, _x, _y, _other); }
      void erase() { if (_drawn) _categorie->erase(_number, _x, _y);        }
      void mxor()   { if (_drawn) _categorie->mxor  (_number, _x, _y);        }

      int number()  { return _number; }
      CoordType x() { return _x; }
      CoordType y() { return _y; }

};

class Edge
{
   friend class EdgeInformation;
   
   private:

      int	     _from;
      int	     _to;

      EdgeCategorie *_categorie;
      
   public:

      Edge(int, int, EdgeCategorie*);

      void draw(NodeInformation*);
      void erase(NodeInformation*);
      void mxor(NodeInformation*);

      int from() { return _from; }
      int to()   { return _to  ; }
   
};

class Edge2
{
   friend class EdgeInformation;
   friend class EdgeQueue;
   
   private:

      CoordType	     _x1, _y1;
      CoordType	     _x2, _y2;

      EdgeCategorie *_categorie;

   public:

      Edge2(CoordType, CoordType, CoordType, CoordType, EdgeCategorie*);

      void draw();
      void erase();
      void mxor();

      CoordType x1() { return _x1; }
      CoordType x2() { return _x2; }
      CoordType y1() { return _y1; }
      CoordType y2() { return _y2; }
};
      

#endif
