/*****************************************************************

            Filename    :  VisualizeAlgoCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.

******************************************************************/


#include "def.glb"
#include "VisualizeAlgoCmd.h"
#include "TreeInterface.h"


/*****************************************************************
                    VisualizeAlgoCmd
******************************************************************/


VisualizeAlgoCmd::VisualizeAlgoCmd(Graph* graph_ptr,char* name,
                                   int active, int time)
             : NoUndoCmd(name,active)

   // Constructor.
{
   _visual_graph = graph_ptr;
   _app = theApplication->appContext();
   _interval = time;                   // Time intervall time milliseconds.
   _node = 0;
   _passed = FALSE;
   
}



/*****************************************************************
                          doit
******************************************************************/


void VisualizeAlgoCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It starts a
   // visualization of the last algorithm, that built the tree.
{
   if (_visual_graph != NULL)
   {
      _passed = FALSE;
                                       // Get the number of nodes in the tree.
      int node_nb = _visual_graph->maxNodes();
      if (node_nb > 0)
         _node = 1;
	                               // First draw all nodes in
				       // Standardshade colour.
      for (int i = 1; i <= node_nb; i++)
      {
         _visual_graph->eraseNode(i);
         _visual_graph->drawNode(STANDARDSHADE,i);
	 theTreeInterface->Tree()->set_node_colour(i,STANDARDSHADE);
	 theTreeInterface->Tree()->set_under_colour(i,STANDARDSHADE);
      }

                                       // Start the timer with a time intervall
				       // of lenght _intervall. During this
				       // Time intervall, the TreeInterface
				       // recognises all other inputs as
				       // mouse button clicks and does the
				       // appropriate action. After the time
				       // intervall is finished, ths procedure
				       // paintCallback of is called.
      _timerID = XtAppAddTimeOut(_app,_interval,&VisualizeAlgoCmd::paintCallback,
                                 (XtPointer) this);

   }
}



/*****************************************************************
                    paintCallback
******************************************************************/


void VisualizeAlgoCmd::paintCallback(XtPointer clientData, XtIntervalId *)

   // This procedure is called after the time intervall is finished.
   // It calls the procedure nextPaint of this object.
{
   VisualizeAlgoCmd * object = (VisualizeAlgoCmd *) clientData;
   object->nextPaint();
}





/*****************************************************************
                      nextPaint
******************************************************************/


void VisualizeAlgoCmd::nextPaint()

   // The Procedure nextPaint is envoked after the finish of the time
   // intervall and called by the procedure paintCallback.
{
   if (_node > 1)
                                       // If there is a highlighted node,
				       // draw it in standard colour.
   {
      _visual_graph->eraseNode(_node-1);
      _visual_graph->drawNode(STANDARDCOLOUR,_node-1);
      theTreeInterface->Tree()->set_node_colour(_node-1,STANDARDCOLOUR);
      theTreeInterface->Tree()->set_under_colour(_node-1,STANDARDCOLOUR);
   }

   if (_node <= _visual_graph->maxNodes())
                                       // If there are still nodes to 
				       // draw, draw the next one in
				       // highlighted colour.
   {
      _visual_graph->drawNode(STANDARDHIGHLIGHT,_node);
      theTreeInterface->Tree()->set_node_colour(_node,STANDARDHIGHLIGHT);
      theTreeInterface->Tree()->set_under_colour(_node,STANDARDHIGHLIGHT);
      _node++;
   }

   if ( (_node-1) < _visual_graph->maxNodes())
                                       // If there are still nodes to draw,
				       // install the timer again by the 
				       // function call installTimer.
      installTimer();
      
   else if ( (_node-1) == _visual_graph->maxNodes() && !_passed)
                                       // We have passed all nodes. Install
				       // the timer one last time in order
				       // to draw the highlighted node in 
				       // standard colour.
   {
      _passed = TRUE;
      installTimer();
   }
   
}



/*****************************************************************
                      installTimer
******************************************************************/


void VisualizeAlgoCmd::installTimer()

   // The procedure is called by the procedure nextPaint in order
   // to install the timer again.
{

   _timerID = XtAppAddTimeOut(_app,_interval,&VisualizeAlgoCmd::paintCallback,
                              (XtPointer) this);

}

