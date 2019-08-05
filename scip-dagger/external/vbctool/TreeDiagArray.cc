/*****************************************************************

            Filename    :  TreeDiagArray.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
			   
******************************************************************/


#include "TreeDiagArray.h"
#include "TreeInterface.h"



/*****************************************************************
                  TreeDiagArray
******************************************************************/


TreeDiagArray::TreeDiagArray(int number)

   // Constructor
{
   _maxWinNum = number;
   _openWinNum = 0;

                                      // Initialize the Array of PopUp Menues
   _treeDialog = new TreeDiagManager*[number];
                                      // Allocate the PopUpMenues
   for (int i = 0; i < number; i++)
   _treeDialog[i] = new TreeDiagManager(i,"Node Information Window",
				       theGFEWindow->baseWidget());

				   // Changed after using gcc 2.7.0
   for (int j = 0; j < number; j++)
      _Q.enqueue(j);
   
}



/*****************************************************************
                  TreeDiagArray
******************************************************************/


TreeDiagArray::~TreeDiagArray()

   // Destructor
{
   for (int i = 0; i < _maxWinNum; i++)
      delete _treeDialog[i];
   delete[] _treeDialog;
}




/*****************************************************************
                     callDialog
******************************************************************/


void TreeDiagArray::callDialog(int node)


   // Procedure which is called by button3Press() in TreeInterface
   // after the /user has pressed the right mouse button.
{

                                 // Check if the are still unused PopUpMenues
   if (_openWinNum < _maxWinNum)
   {

      char*   buflong;
      char*   bufshort;
      char    bufshort2[SHORTSIZE];
      tree*   _T = theTreeInterface->Tree();
                                 // Get an unused PopUpMenue.
      int     window = _Q.dequeue();
      
	                         // Get the nodes information.
      buflong = _T->get_node_info(node);
      if (_T->is_standard())
         bufshort = _T->get_node_short(node);
      else
      {
         sprintf(bufshort2,"node: %d\n",node);
         bufshort = bufshort2;
      }

      if (_T->is_displayed(node))
                                // If there is already an open PopUp, which
				// displays the nodes information, the
				// information shown by the PopUp is only
				// updated. The call _T->is_displayed(node)
				// gives the number of the window plus 1.
      {
         _treeDialog[_T->is_displayed(node)-1]->changeWindow(buflong,bufshort);
      }
      else
                                // There is no open PopUp, so an unused
				// PopUp is opend an initialized with the
				// information of the node.
      {
         _treeDialog[window]->post(buflong,bufshort,node);
         _T->display(node,window+1);
         _openWinNum++;
      }

   }

                                // There is no unused PopUpMenue left.
				// Remind the user to close one before
				// using the right mouse button again.
   else
   {
      printf("For further displaying of Node Information:\n");
      printf("Please close a Dialog Window first.\n"); 
   }
}






/*****************************************************************
                  decreaseOpenWin
******************************************************************/


void TreeDiagArray::decreaseOpenWin(int number)

   // Procedure which is called after a PopUpMenue is closed
   // via the OK or Cancel button. Makes shure that the clodes
   // window can be used again. 
{
   _Q.enqueue(number);
   _openWinNum--;
}



/*****************************************************************
                  cleanup
******************************************************************/


void TreeDiagArray::cleanup()

   // If a new Graph or Problem is loaded, open PopUpMenues have to be
   // closed. This is done by this procedure called by load() in
   // TreeInterface.
{

   if (_openWinNum > 0)
                                // Close all open PopUpMenues
   {
      for (int i = 0; i < DIALOGNUM; i++)
         _treeDialog[i]->unpost();
   }

   _openWinNum = 0;
   _Q.empty_queue();
   for (int i = 0; i < _maxWinNum; i++)
      _Q.enqueue(i);
   
}
