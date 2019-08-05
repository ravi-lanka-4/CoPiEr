#ifndef GRAPHINTERFACE_H
#define GRAPHINTERFACE_H

#include "ClientInterface.h"
#include "NodeSelection.h"
#include "Graph.h"

class NodePopup;
class NodeSelection;
class EdgeCategorie;
class NodeCategorie;
class NodeListPopup;
class EdgeListPopup;
class Interrupt;
class SingleStep;

class GraphInterface : public ClientInterface
{
   private:

      NodeListPopup*	_nodeListPopup;

      EdgeListPopup*	_edgeListPopup;
      
      NodeSelection*	_nodeSelection;
      Graph*		_graph;

      NodeCategorie   **_nodeCategorie;
      int		_maxNodeCategories;
      EdgeCategorie   **_edgeCategorie;
      int		_maxEdgeCategories;


      Interrupt	       *_interrupt;
      SingleStep       *_singleStep;

      void initCategories();

   protected:

   public:

      GraphInterface(int nc = 1, int ec = 1);
      ~GraphInterface();

      NodeSelection   *nodeSelection() { return _nodeSelection; }
      Graph	      *graph() { return _graph; }
      void cmdInit();

      virtual char *edgeColor(int i) { return NULL; } // all returned strings have to be static!!
      virtual char *edgeCustomName(int i) { return NULL ;}
      virtual CoordType edgeWidth(int i) { return (CoordType) (-1);}

      virtual char *nodeColor(int i) { return NULL; }
      virtual char *nodeFontColor(int i) { return NULL; }
      virtual char *nodeFont(int i) { return NULL; }
      virtual char *nodeCustomName(int i) { return NULL; }

      int forkProcess(int (*func)(char*, int, int), char*, int, int);
      void killProcess(int pid);

      void button1Press(Point<CoordType> point)
	 { _nodeSelection->setValue(_graph->nextNode(&point)); }

      int maxNodeCategories() { return _maxNodeCategories; }
      int maxEdgeCategories() { return _maxEdgeCategories; }
      NodeCategorie **nodeCategorie() { return _nodeCategorie; }
      EdgeCategorie **edgeCategorie() { return _edgeCategorie; }

      void addToExistingMenu(CmdList*, CmdList*, CmdList*);

      char *version();
      char *prefix();
};

extern GraphInterface *theGraphInterface;

#endif
