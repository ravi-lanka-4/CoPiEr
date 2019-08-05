/*****************************************************************

            Filename    :  VisualizeAlgoCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Visualizes a Brunch and Cut Algorithm.
	                   A derived Class from NoUndoCmd.
			   
******************************************************************/


#ifndef VISUALIZEALGOCMD_H
#define VISUALIZEALGOCMD_H

#include "Graph.h"
#include "NoUndoCmd.h"

class VisualizeAlgoCmd : public NoUndoCmd{

public:

   VisualizeAlgoCmd(Graph* = NULL,char* = NULL,int = 0,int = 1000);

protected:

   void doit();

private:

   Graph*        _visual_graph;
   XtIntervalId  _timerID;
   XtAppContext  _app;
   int           _interval;
   int           _node;
   int	         _passed;

   void nextPaint();
   void installTimer();

   static void paintCallback(XtPointer, XtIntervalId *);
   
   
};

#endif
