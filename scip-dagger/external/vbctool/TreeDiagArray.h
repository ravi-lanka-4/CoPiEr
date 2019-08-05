/*****************************************************************

            Filename    :  TreeDiagArray.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages an Array of PopUp Menues,
	                   dependent on the class of TreeDiagManager.
			   
******************************************************************/

#ifndef TREEDIAGARRAY_H
#define TREEDIAGARRAY_H

#include "TreeDiagManager.h"
#include "queue.h"

class TreeDiagArray {

public:

   TreeDiagArray(int = 1);
   ~TreeDiagArray();

   void callDialog(int);
   void decreaseOpenWin(int);
   void cleanup();

private:


   TreeDiagManager**     _treeDialog;    // Array of PopUp Menues
   queue<int>            _Q;             // A queue for keeping track of which
                                         // PopUp Memue is used
   int                   _maxWinNum;     // Maximum number of PopUp Menues
   int                   _openWinNum;    // Aktuall number od open PopUp Menues

};


#endif
