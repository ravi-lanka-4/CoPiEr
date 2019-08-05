/*****************************************************************

            Filename    :  TreeInterface.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Interface for drawing trees.
	                   A drived class from GraphInterface.

******************************************************************/


#ifndef TREEINTERFACE_H
#define TREEINTERFACE_H

#include "GFE.inc"
#include "GraphInterface.h"
#include "queue.h"
#include "tree.h"
#include "GFEWindow.h"
#include "TreeDiagArray.h"
#include "VisualizeAlgoCmd.h"
#include "TreeScalerCmd.h"
#include "TreeSearcherCmd.h"
#include "SwitchToSearcherCmd.h"
#include "NormalModeCmd.h"
#include "EmulationCmd.h"
#include "EmlSetupCmd.h"
#include "EmlInterruptCmd.h"
#include "EmlStopCmd.h"
#include "EmlContinueCmd.h"
#include "StandardInputCmd.h"
#include "CopyrightCmd.h"
#include "VBCCmd.h"
#include <X11/cursorfont.h>

class StdinInterruptCmd;   // implements the stdin-interrupt-function
class StdinContinueCmd;    // implements the stdin-continue-function

class TreeInterface : public GraphInterface {

public:

   TreeInterface();
   ~TreeInterface();



/*****************************************************************
      Virtual Functions from the GFE-ClientInterface.
******************************************************************/

   // Virtual functions of the GFE that have been overloaded by
   // the TreeInterface.

   virtual int    graphicInit();
                                    // Initializes all graphical fonts.

   virtual int    menuInit(MenuBar*);
                                    // Initializes the menubar.

   virtual int    load(FILE*,char*);
                                    // Manages the load command.

   virtual int    handleFirstEvents();
				    // Manages the first events.


/*****************************************************************
         Virtual Functions from TreeInterface.
******************************************************************/

   // Functions introduced for an easy handling of a derivation
   // of the TreeInterface. They can be overloaded by the customer
   // at will. No special purpose in the TreeInterface itself.
				    
   virtual int    checkFile(FILE* file_ptr,char* filename){ return FALSE; };
                                    // Checks a file for customer purposes. 
   virtual void   addToAlgorithmMenue(CmdList* List){};
                                    // Adds menue buttons into the
				    // menue "Algorithms".
   


/*****************************************************************
         Special Purpose Virtual Functions from TreeInterface.
******************************************************************/

   // Functions can be overloaded by the customer in a derivation
   // of the TreeInterface. Functions are needed in the TreeInterface.
   
   virtual void   DeactivationCall(){ _visualizeAlgo->deactivate();
                                      _branchAndCutCmd->deactivate(); };   
                                    // Deactivate command buttons
				    // in the menue bar.
   virtual void   ActivationCall(){   _treeScaler->activate();
				      _switchToSearcherCmd->activate(); };
                                    // Activate command buttons
				    // in the menue bar.



/*****************************************************************
       Customer function calls from TreeInterface.
******************************************************************/

   // Functions introduced for adapting customer subroutines to
   // the TreeInterface.

   int    SetNodeInfo(int node,char* information);
                                    // Sets the information of a node.
   int    AddNodeInfo(int node,char* information);
                                    // Adds information to already
				    // existing information of a node.
   void   PaintNode(int node,int colour);
                                    // Sets a colour of a node
   void   NewNode(int father, int newNode,int colour);
                                    // Introduces a new node. Includes
				    // repainting the tree.
   void   NewNode(int father, int newNode,int colour,int draw);
                                    // Introduces a new node. Repainting
				    // the tree is dependent on the
				    // boolean value of draw.
   void   UpperBound(double bound);
                                    // Prints out the upper bound.
   void   LowerBound(double bound);
                                    // Prints out the lower bound.
   void   RepaintTree();
                                    // Repaints a tree.
  


/*****************************************************************
      Virtual Customer function calls from TreeInterface.
******************************************************************/
   
   // Function introduced for adapting customer subroutines to
   // the TreeInterface. Can be overloaded by the client in a
   // derivation of the TreeInterface.

   virtual void   FinishAlgorithm(int boolean, int returnvalue);
                                    // After finishing a customers
				    // subroutine, this function
				    // activates command buttons
				    // and starts valid clean ups.



/*****************************************************************
                  Return Values.
******************************************************************/

   // Public functions returning values.
   
   double ActualRadius(){ return _actualRadius; };
                                    // Returns the actual radius of
				    // the nodes. 
   tree*  Tree(){ return &_T; };
                                    // Returns a pointer to the tree.



/*****************************************************************
         Public commands for the Emulation Process. 
******************************************************************/

   // Public commands used only for the emulation process.
   // They only supply pointers to the different Cmd classes,
   // which supports the Cmd classes to interact each other.

   EmulationCmd*     emulationCmd(){ return _emulationCmd; };
                                    // Returns the pointer to the start
				    // command of the emulation process.
   EmlInterruptCmd*  emlInterruptCmd(){ return _emlInterruptCmd; };
                                    // Returns the pointer to the
				    // interrupt command. 
   EmlStopCmd*       emlStopCmd(){ return _emlStopCmd; };
                                    // Returns the pointer to the
				    // stop command.
   EmlContinueCmd*   emlContinueCmd(){ return _emlContinueCmd; };
                                    // Returns the pointer to the
				    // continue command. 
   EmlSetupCmd*      emlSetupCmd(){ return _emlSetupCmd; };
                                    // Returns the pointer to the
				    // setup command.  

   
/*****************************************************************
         Public commands for the standard-in Process. 
******************************************************************/

   StandardInputCmd* standardInputCmd() { return _standardInputCmd; };
				    // Returns the pointer to the
				    // standard input command.

/*****************************************************************
           Further public commands of TreeInterface
******************************************************************/

   // Public commands used in different classes that are included
   // by the TreeInterface.

   void   NewActualRadius(double rad){ _actualRadius = rad; };
                                    // Sets a new radius for the nodes
   void   MaxCoordValues(double *x,double *y){ (*x) = _T.get_max_X_coord();
                                               (*y) = _T.get_max_Y_coord(); };
                                    // Gets the maximal coordinates
				    // of the tree. Used for estimating
				    // the size of coordinate systems
   void   decreaseOpenWin(int number){ _treedialog->decreaseOpenWin(number); };
                                    // Decreases the number of open
				    // Node Information Window.




private:

   TreeDiagArray*           _treedialog;

   tree                     _T;

   int                      _node_nb;
   int                      _branchAndCutActive;
   int                      _firstVisualize;
   
   Cursor                   _cursor;

   VisualizeAlgoCmd*        _visualizeAlgo;
   TreeScalerCmd*           _treeScaler;
   SwitchToSearcherCmd*     _switchToSearcherCmd;
   TreeSearcherCmd*         _treeSearcherCmd;
   NormalModeCmd*           _normalModeCmd;
   CopyrightCmd*	    _copyrightCmd;
   
   VBCCmd*                  _branchAndCutCmd;

   EmulationCmd*            _emulationCmd;
   EmlSetupCmd*             _emlSetupCmd;
   EmlInterruptCmd*         _emlInterruptCmd;
   EmlStopCmd*              _emlStopCmd;
   EmlContinueCmd*          _emlContinueCmd;

   StandardInputCmd*        _standardInputCmd;
   StdinInterruptCmd*       _stdinInterruptCmd;
   StdinContinueCmd*        _stdinContinueCmd;
   
   double                   _actualRadius;
   double		    _upperBound;
   double		    _lowerBound;
   

   void   Mybutton1Motion(XEvent *event);
   void   Mybutton1Release(XEvent *event);
   void   button3Press(Point<CoordType>);
   void   SpaceBarPressEvent(XEvent *event);
   

   void   readCompleteTree(char*);
   void   NewTreePaint(int);
   
   static void button1MotionEventHandler(Widget widget,
			     XtPointer clientData,
			     XEvent *event,
			     Boolean *continueToDiapatch);

   static void buttonReleaseEventHandler(Widget widget,
                             XtPointer clientData,
			     XEvent *event,
			     Boolean *continueToDiapatch);

   static void keyPressSpaceBarEventHandler(Widget widget,
                             XtPointer clientData,
			     XEvent *event,
			     Boolean *continueToDiapatch);

};


extern TreeInterface* theTreeInterface;

#endif

