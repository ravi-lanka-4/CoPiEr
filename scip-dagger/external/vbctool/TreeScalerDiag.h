/*****************************************************************

            Filename    :  TreeScalerDiag.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages the PopUp Menue for scaling the
	                   Tree. Does make a redraw after choosing
			   the new scaler.
			   A derived Class from PopupManager.
			   
******************************************************************/


#ifndef TREESCALERDIAG_H
#define TREESCALERDIAG_H

#include "GFE.inc"
#include "PopupManager.h"
#include "def.glb"
#include "Graph.h"
#include "tree.h"


class TreeScalerDiag : public PopupManager {

public:

   TreeScalerDiag(char*,Widget);
   ~TreeScalerDiag();

   virtual void post();
   void _setMaxValues(int subtree,int sibling);

protected:


   virtual void cancel();
   virtual void ok();
   virtual void apply();

private:

   virtual void createControlArea();
   virtual void createActionArea();

   void setXtValueRadius(int value,int min,int max);

   Graph*          _visual_graph;
   tree*           _T;

   short _decimalFaktor;
   short _decimalPoints;

   int   subtree_max;
   int   sibling_max;
   int   radius_max;
   
   Widget    _scalerControl;
   Widget    _levelScale;
   Widget    _siblingScale;
   Widget    _subtreeScale;
   Widget    _radiusScale;

   
};

#endif

