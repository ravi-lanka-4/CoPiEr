/*****************************************************************

            Filename    :  TreeSearcherCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

******************************************************************/


#ifndef TREESEARCHERCMD_H
#define TREESEARCHERCMD_H

#include "Graph.h"
#include "NoUndoCmd.h"
#include "tree.h"
#include "def.glb"


class TreeSearcherCmd : public NoUndoCmd{

public:

   TreeSearcherCmd(char* = NULL,int = 0,DrawArea* = NULL,
                   tree* = NULL,Graph* = NULL);

   void executeWithPoint(Point<CoordType>*);
   void cleanup();
   
protected:

   void doit();

private:

   DrawArea*         _drawArea;
   Graph*            _visual_graph;
   tree*             _T;
   Point<CoordType>  _point;
   int               _currentNode;
   int               _nextNode;
   int               _highlighted;
   
   
};

#endif
