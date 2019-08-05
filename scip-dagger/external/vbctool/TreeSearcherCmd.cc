/*****************************************************************

            Filename    :  TreeSearcherCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File
	    
******************************************************************/

#include "TreeSearcherCmd.h"
#include "GFEWindow.h"
#include "DisplayArea.h"


/*****************************************************************
                    TreeSearcherCmd
******************************************************************/


TreeSearcherCmd::TreeSearcherCmd(char* name,int active,DrawArea* da,
                                 tree* Tree,Graph* graph_ptr)
             : NoUndoCmd(name,active)

    // Constructor
{
 
   _drawArea = da;
   _visual_graph = graph_ptr;
   _T = Tree;
   

   _currentNode = 0;
   _nextNode = 0;

   _highlighted = FALSE;
}



/*****************************************************************
                   executeWithPoint
******************************************************************/


void TreeSearcherCmd::executeWithPoint(Point<CoordType> *point)

   // This Procedure is called by Mybutton1Motion in the TreeInterface.
   // That means, if the user presses the left mouse button and moves
   // it around the draw Area, this is going to be registered by the
   // button1MotionEventHandler (TreeInterface), which calls Mybutton1Motion
   // which then calls executeWithPoint.
{
   _point.newInit(point);

                                       // The execute command is a function
				       // by D.A. Young, which first checks
				       // whether this Command is active.
				       // That means it checks whether the
				       // user has switched to Browser Mode.
				       // If it is active the doit function of
				       // this Command is called. If not,
				       // nothind happens.
   execute();
}



/*****************************************************************
                       doit
******************************************************************/


void TreeSearcherCmd::doit()

   // The doit function executes the Command.
{
   if (_visual_graph != NULL && _T != NULL && !_T->tree_empty())
   {
                                       // Get the node, which is next to
				       // the cursor.
      _nextNode = _visual_graph->nextNode(&_point);
      if (_nextNode != _currentNode)
                                       // If the _nextNode is differnet from
				       // the last found node (_currentNode)
				       // the important information of the
				       // _nextNode. Has to be printed
				       // This is not necessary if the
				       // _nextNode is the last found node, 
				       // since its message is already printed.
      {
         char* message;
         char  message2[SHORTSIZE];
	 if (_T->is_standard())
	                               // The nextNode contains standard
				       // information. Print it. 
	 {
	    message = _T->get_node_short(_nextNode);
  	    lprintf(message);
	 }
	 else
	                               // The nextNode contains no information.
				       // Print its number.
	 {
	    sprintf(message2,"node: %d\n",_nextNode);
	    lprintf(message2);
	 }

	                               // Draw the nextNode in a highlighted
				       // colour. Draw the currentNode in
				       // its underlying colour. Observe, that 
				       // this is not necessary the colour the
				       // node had, before is was highlighted.
				       // Its colour might have changed during
				       // some Algorithm by the call PaintNode
				       // in the TreeInterface. Since this
				       // call has no effectiveness on nodes,
				       // which are highlighted, the new
				       // colour was remebered as underlying
				       // colour.
				       // In case that the node has been
				       // highlighted by a nodeInformation
				       // window, the node is not redrawn
				       // in its underlying colour.
	 if (_currentNode > 0 && _currentNode <= _T->get_node_nb())
	 {
	    if (!_T->is_displayed(_currentNode) && !_highlighted)
	    {
	       _visual_graph->eraseNode(_currentNode);
	       _visual_graph->drawNode(_T->get_under_colour(_currentNode),
	                               _currentNode);
	       _T->set_node_colour(_currentNode,
	                           _T->get_under_colour(_currentNode));
	       _T->highlight(_currentNode,FALSE);
	    }
	 }
	 if (!_T->is_highlighted(_nextNode))
	 {
	    _visual_graph->eraseNode(_nextNode);
	    _visual_graph->drawNode(STANDARDHIGHLIGHT,_nextNode);
	    _T->set_node_colour(_nextNode,STANDARDHIGHLIGHT);
	    _T->highlight(_nextNode,TRUE);
	    _currentNode = _nextNode;
	    _highlighted = FALSE;
	 }
	 else
	    _highlighted = TRUE;
      }
   }

}




/*****************************************************************
                       cleanup
******************************************************************/


void TreeSearcherCmd::cleanup()

   // If the user releases the left mouse button, one node is still
   // highlighted. It will be drawn in its underlying colour by this
   // procedure. This procedure is called bu Mybutton1Release in the
   // TreeInterface.
{
   if (_currentNode > 0 && _currentNode <= _T->get_node_nb())
                                       // If the Browser Mode is active, no
				       // node was highlighted by this
				       // procedure and currentNode will be
				       // zero.
   {
      if (!_highlighted)
      {
         _visual_graph->eraseNode(_currentNode);
         _visual_graph->drawNode(_T->get_under_colour(_currentNode),_currentNode);
         _T->set_node_colour(_currentNode,_T->get_under_colour(_currentNode));
         _T->highlight(_currentNode,FALSE);
      }
   }
   _highlighted = FALSE;
   _currentNode = 0;
}

