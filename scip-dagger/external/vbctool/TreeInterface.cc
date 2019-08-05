/*****************************************************************

            Filename    :  TreeInterface.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Interface for drawing trees.

******************************************************************/


#include <X11/keysym.h>
#include "tree.h"
#include "node.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cstdlib>
#include "TreeInterface.h"
#include "TreeDiagManager.h"
#include "GFEWindow.h"
#include "GFEResourceManager.h"
#include "MenuBar.h"
#include "ClientInterface.h"
#include "StdinInterruptCmd.h"
#include "StdinContinueCmd.h"

#ifdef LINUX

#include <unistd.h>

#endif




   TreeInterface* theTreeInterface = NULL; 

/*****************************************************************
                    TreeInterface
******************************************************************/


TreeInterface::TreeInterface()
      :GraphInterface(COLOURCOUNT,1)

   // Constructor
{
   _treedialog = NULL;
   _node_nb = 0;
   _lowerBound = 0;
   _upperBound = 0;
   _branchAndCutActive = FALSE;
   _firstVisualize = TRUE;

   theTreeInterface = this;   

}



/*****************************************************************
                    ~TreeInterface
******************************************************************/


TreeInterface::~TreeInterface()

   // Destructor
{
   delete _treedialog;
   delete _visualizeAlgo;
   delete _treeScaler;
   delete _switchToSearcherCmd;
   delete _treeSearcherCmd;
   delete _normalModeCmd;
   delete _branchAndCutCmd; 
   delete _emulationCmd;
   delete _emlInterruptCmd;
   delete _emlStopCmd;
   delete _emlContinueCmd;
   delete _emlSetupCmd;
   delete _standardInputCmd;
   delete _copyrightCmd;
   delete _stdinInterruptCmd;
   delete _stdinContinueCmd;

   
/*
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    A new command has to be deleted here.
    
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
}
   




/*****************************************************************
                    graphicInit
******************************************************************/


int TreeInterface::graphicInit()

   // Initializes all graphical fonts.
{
                                       // The button in the menue bar
				       // for loading a problem has to
				       // be activated.  
   loadCmd()->activate();

   _treedialog = new TreeDiagArray(DIALOGNUM);

                                       // Add the Event Handlers for mouse-
				       // button input.

   XtAddEventHandler(theDrawArea->dArea(),
                     Button1MotionMask,FALSE,
		     button1MotionEventHandler,(XtPointer) this);


   XtAddEventHandler(theDrawArea->dArea(),
                     ButtonReleaseMask,FALSE,
                     buttonReleaseEventHandler,(XtPointer) this);


                                       // Add the Event Handlers for
				       // keybord input.

   XtAddEventHandler(theDrawArea->dArea(),
                     KeyPressMask, FALSE,
	             keyPressSpaceBarEventHandler,
	             (XtPointer) this);
							    
                                       // Get important values out of the file
				       // GRAPHStandardResource.rsc.
   char* vbcptr;
   GFEResourceManager rsc(STANDARDRESOURCE);
   vbcptr = rsc.getstring("UserDefinedCommandName","User Algorithm");

   int   level;
   int   sibling;
   int   subtree;
   int   radius;
   char* sepptr;
   char  sepval[MAXSEPARATION];

   std::stringstream is; 

                                        // Get the level separation value
					// from GRAPHStandardResource.rsc.
   sepptr = rsc.getstring("TreeLevelSeparationValue","2");
   is << sepptr << std::endl; 
   is >> level;
   if (level < 0)
      level = 0;
   else if (level> MAXSEPARATION)
      level = MAXSEPARATION;
   
   
                                        // Get the subtree separation value
					// from GRAPHStandardResource.rsc.
   sepptr = rsc.getstring("TreeSubtreeSeparationValue","4");
   is << sepptr << std::endl;
   is >> subtree;
   if (subtree < 0)
      subtree = 0;
   else if (subtree > MAXSEPARATION)
      subtree = MAXSEPARATION;


                                        // Get the sibling separation value
					// from GRAPHStandardResource.rsc.
   sepptr = rsc.getstring("TreeSiblingSeparationValue","4");
   is << sepptr << std::endl;
   is >> sibling;
   if (sibling < 0)
      sibling = 0;
   else if (sibling > MAXSEPARATION)
      sibling = MAXSEPARATION;

   _T.set_separation_values(sibling,level,subtree);
   

                                        // Get the radius from
					// GRAPHStandardResource.rsc.
   int _decimalFactor = 10;
   sepptr = rsc.getstring("TreeNodeRadiusValue","10");
   is << sepptr << std::endl;
   is >> radius;

                                        // Make sure the radius is not to
					// small and not to large. The latter
					// means, that the radius of the
					// nodes may not be so large, that
					// they start intersecting each
					// other when using the current
					// separation values.
   int max = (sibling+2)/2;
   if (max > ((level+2)/2))
      max = (level+2)/2;
   if (max > ((subtree+2)/2))
      max = (subtree+2)/2;
   if (radius < 5)
      _actualRadius = 1.0;
   else if (radius > (max*_decimalFactor))
      _actualRadius = max;
   else
      _actualRadius = double (radius) /_decimalFactor;


   int intervall = 0;
                                        // Get the time intervall for
					// the VisualizeAlgoCmd 
					// from GRAPHStandardResource.rsc.
   sepptr = rsc.getstring("VisualizeAlgorithmTimeIntervall","1000");
   //strncpy(sepval,sepptr,sizeof(sepval));
   is << sepptr << std::endl;
   is >> intervall;
   if (intervall < 1)
      intervall = 1000;

                                       // Initialize the commands, which are
				       // added to the GraphInterface by the
				       // TreeInterface.
				       
   _visualizeAlgo = new VisualizeAlgoCmd(graph(),"Visualize Algorithm",
                                         FALSE,intervall);
   _treeScaler = new TreeScalerCmd("Scaler...",FALSE);
                                       // switchToSearcherCmd is the only
				       // visible command in the menue-bar
				       // that belongs to the brouser mode.
   _switchToSearcherCmd = new SwitchToSearcherCmd("Browser Mode",FALSE,theDrawArea);
   _treeSearcherCmd = new TreeSearcherCmd("unvisible Searcher",FALSE,
                                          theDrawArea,&_T,graph());
   _normalModeCmd = new NormalModeCmd("Normal Mode",FALSE,theDrawArea);
   _branchAndCutCmd = new VBCCmd(vbcptr,FALSE);

   _copyrightCmd = new CopyrightCmd("Copyright",TRUE);

   _emulationCmd = new EmulationCmd("Start Emulation",FALSE);
   _emlSetupCmd = new EmlSetupCmd("Setup...",FALSE);
   _emlStopCmd = new EmlStopCmd("Stop Emulation",FALSE);
   _emlInterruptCmd = new EmlInterruptCmd("Interrupt",FALSE);
   _emlContinueCmd = new EmlContinueCmd("Continue",FALSE);
   
   _standardInputCmd = new StandardInputCmd("Standard In",TRUE);
   _stdinInterruptCmd = new StdinInterruptCmd("Interrupt2",FALSE);
   _stdinContinueCmd  = new StdinContinueCmd("Continue2",FALSE);
   
				       // Create the cursor for the
				       // brouser mode 
   int cursorshape = XC_crosshair;
   _cursor = XCreateFontCursor(theDrawArea->display(),cursorshape);

   _treeScaler->set_separation_values(sibling,subtree);

/*
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    A new command has to be initialized here.
    
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
   
   return SUCCESS;



}




/*****************************************************************
                    handleFirstEvents
******************************************************************/


int TreeInterface::handleFirstEvents()
{
    if (!isatty(fileno(stdin))) {
	_treeScaler->activate();
	_stdinInterruptCmd->activate();
	_standardInputCmd->standardIn();
	_stdinInterruptCmd->deactivate();
	_stdinContinueCmd->deactivate();
    }
}




/*****************************************************************
                    menuInit
******************************************************************/


int TreeInterface::menuInit(MenuBar* mb)

   // Initializes the menubar.
{
   CmdList *myAppList = new CmdList();
   CmdList *AlgoList = new CmdList();
   CmdList *EmlList = new CmdList();
   CmdList *StdInList = new CmdList();
   CmdList *CopyrightList = new CmdList();

   
                                        // TreeSearcherCmd should be active
					// after switching to Searching
   _switchToSearcherCmd->addToActivationList(_treeSearcherCmd);
                                        // After switching to Searching
					// _switchToSearcherCmd must
					// be inactive.
   _switchToSearcherCmd->addToDeactivationList(_switchToSearcherCmd);
                                        // After switching to Searching
					// _normalModeCmd must be active;
   _switchToSearcherCmd->addToActivationList(_normalModeCmd);
                                        // After switching to NormalMode
					// _normalModeCmd must be inactive;
   _normalModeCmd->addToDeactivationList(_normalModeCmd);
                                        // After switching to NormalMode
					// _switchToSearcherCmd has to
					// be active again.
   _normalModeCmd->addToActivationList(_switchToSearcherCmd);
                                        // After switching to NormalMode
					// _treeSearcherCmd has to
					// be inactive again.
   _normalModeCmd->addToDeactivationList(_treeSearcherCmd);
   _emulationCmd->addToDeactivationList(_emulationCmd);
   _emulationCmd->addToActivationList(_emlInterruptCmd);
   _emulationCmd->addToActivationList(_emlStopCmd);

   _emlInterruptCmd->addToDeactivationList(_emlInterruptCmd);
   _emlInterruptCmd->addToActivationList(_emlContinueCmd);

   _emlContinueCmd->addToDeactivationList(_emlContinueCmd);
   _emlContinueCmd->addToActivationList(_emlInterruptCmd);

   _emlStopCmd->addToDeactivationList(_emlStopCmd);
   _emlStopCmd->addToDeactivationList(_emlInterruptCmd);
   _emlStopCmd->addToDeactivationList(_emlContinueCmd);
   _emlStopCmd->addToActivationList(_emulationCmd);


//   _standardInputCmd->addToActivationList(_switchToSearcherCmd);
//   _standardInputCmd->addToActivationList(_treeScaler);

   _stdinInterruptCmd->addToActivationList(_stdinContinueCmd);
   _stdinInterruptCmd->addToDeactivationList(_stdinInterruptCmd);
   _stdinContinueCmd->addToActivationList(_stdinInterruptCmd);
   _stdinContinueCmd->addToDeactivationList(_stdinContinueCmd);


   myAppList->add(_visualizeAlgo);
   myAppList->add(_treeScaler);
   myAppList->add(_switchToSearcherCmd);
   myAppList->add(_normalModeCmd);
   
   AlgoList->add(_branchAndCutCmd);
   addToAlgorithmMenue(AlgoList);

   EmlList->add(_emulationCmd);
   EmlList->add(_emlSetupCmd);
   EmlList->add(_emlInterruptCmd);
   EmlList->add(_emlContinueCmd);
   EmlList->add(_emlStopCmd);

//   StdInList->add(_standardInputCmd);
   StdInList->add(_stdinInterruptCmd);
   StdInList->add(_stdinContinueCmd);

   CopyrightList->add(_copyrightCmd);
   
/*
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    A new command has to be added to the AlgoList here.
    
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

   mb->addCommands(myAppList,"Applications");
   mb->addCommands(AlgoList,"Algorithm");
   mb->addCommands(EmlList,"Emulation");
   mb->addCommands(StdInList,"Online");
   mb->addCommands(CopyrightList,"Tree Interface");
   delete(myAppList);
   delete(AlgoList);
   delete(EmlList);
   delete(StdInList);
   delete(CopyrightList);
   
   loadCmd()->activate();

   return SUCCESS;
}





/*****************************************************************
                       load
******************************************************************/


int TreeInterface::load(FILE *file_ptr, char *filename)

   // Loads a Tree, computes the cordinates and initializes the
   // graphical class graph. 
{




                                        // If a graph has been loaded before
					// some dialog-menues may be still
					// posted. Remove them.

   _treedialog->cleanup();

   if (_treeScaler != NULL)
      _treeScaler->cleanup();
   _emulationCmd->Stop();
                                        // Deactivate some commands.
   _branchAndCutCmd->deactivate();
   _branchAndCutActive = FALSE;
   _emulationCmd->deactivate();
   _emlSetupCmd->deactivate();
  
   char inputline_1[INFOSIZE];
   char inputline_2[INFOSIZE];
   char *inputptr_1;
   char *inputptr_2;
   char infochar;
   int  j = 0;
   int  k = 0;

   DIR *dp;
   if ((dp = opendir(filename)) == NULL)
   {
      std::ifstream inClientFile(filename,std::ios::in);

                                         // Get the first line of the loaded
					 // problem in order to check what
					 // kind of problem it is.
      while ( (infochar = inClientFile.get()) != '\n' && j < FIRSTLINECOUNT)
         inputline_1[j++] = infochar;
      inputline_1[j] = '\0';
      inputptr_1 = inputline_1;
                                         // Get the second line in order to
					 // check which kind of complete tree
					 // this is .
      while ( (infochar = inClientFile.get()) != '\n' && k < SECONDLINECOUNT)
         inputline_2[k++] = infochar;
      inputline_2[k] = '\0';
      inputptr_2 = inputline_2;

      while (--k >= 0 )
         inClientFile.putback(inputline_2[k]);
      while (--j >= 0 )
         inClientFile.putback(inputline_1[j]);

                                         // Activate necessary buttons.
      _treeScaler->activate();
      if (!_treeSearcherCmd->active())
         _switchToSearcherCmd->activate();
      else
         _normalModeCmd->activate();


      theDisplayArea->clearRight();
      theDisplayArea->clearLeft();
      printf("File ");
      printf(filename);
      printf(" has been loaded.\n\n");

					 // Now check what kind of problem
					 // it is.
      if (!strcmp(inputptr_1,FIRSTLINE))
      {
         if (!strcmp(inputptr_2,SECONDLINE_1))
	 {
	    _emulationCmd->activate();
	    _emlSetupCmd->activate();
            _emulationCmd->setFileName(filename);
            _T.set_standard(TRUE);
	 }
	 else
            readCompleteTree(filename);
      }
      
      else if (!checkFile(file_ptr,filename))      
                                         // A Branch & Cut Problem is loaded.
      {
                                         // Activate the Branch & Cut command
					 // button in the menue bar.
         _branchAndCutCmd->activate();
	 _branchAndCutActive = TRUE;
         _branchAndCutCmd->setFileName(file_ptr,filename);
	 _T.set_standard(TRUE);
      }
/*
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    New commands probably have to be activated here.
    If different Problems have to be loaded, make
    appropriate changes in the if else struckture.
    
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
   }

   else
      closedir(dp);

}





/*****************************************************************
                    SetNodeInfo
******************************************************************/


int TreeInterface::SetNodeInfo(int node,char *information)

   // Set the Information of a node.
   // If the node has already an information, this
   // information is deleted and replaced by the new one.

   // THIS PROCEDURE IS ONLY USED BY EXTERNAL PROGRAMMS
{
   int boolean = set_node_info(node,information,_T);
   if (boolean)
   {
      if (_T.is_displayed(node))
                                        // If the information of the node
					// was displayed in a PopUpMenue,
					// the PopUpMenue has to be updated.
         _treedialog->callDialog(node);
   }
   return boolean;
}
	   



/*****************************************************************
                    AddNodeInfo
******************************************************************/


int TreeInterface::AddNodeInfo(int node,char *information)

   // Add new information to the existing information of a node.
   // If the node has already an information, this
   // information is kept.

   // THIS PROCEDURE IS ONLY USED BY EXTERNAL PROGRAMMS
{
   int boolean = add_node_info(node,information,_T);
   if (boolean)
   {
      if (_T.is_displayed(node))
                                        // If the information of the node
					// was displayed in a PopUpMenue,
					// the PopUpMenue has to be updated.
         _treedialog->callDialog(node);
   }
   return boolean;
}
	   


/*****************************************************************
                    PaintNode
******************************************************************/


void TreeInterface::PaintNode(int _node,int _colour)

   // Paint a node in a new Colour.

   // THIS PROCEDURE IS ONLY USED BY EXTERNAL PROGRAMMS

{
   if (_colour >0 && _colour <= COLOURCOUNT)    
   {
       if (_T.is_highlighted(_node))
                                        // The node is highlighted at the
					// moment, so we cannot draw it in
					// new colour. Just keep the colour
					// of the node in mind.
       {
	  _T.set_under_colour(_node,_colour);
       }
       else
       {
          graph()->eraseNode(_node);
	  graph()->drawNode(_colour,_node);
	  _T.set_node_colour(_node,_colour);
	  _T.set_under_colour(_node,_colour);
       }
   }
   else
      printf("TreeInterface: Not Accessible Colour was chosen.\n");
    
}




/*****************************************************************
                       NewNode
******************************************************************/


void TreeInterface::NewNode(int father,int newNode,int colour)

   // Creates a new node and enters it into the Tree.
   // Observe that this includes computing new coordinates
   // for all of the nodes of the tree and causes a Repaint in
   // the DrawArea.

   // THIS PROCEDURE IS ONLY USED BY EXTERNAL PROGRAMMS
{
   _T.enter_new_node(father,newNode,colour);
   if (father == 0)
      NewTreePaint(TRUE);
   else
      NewTreePaint(FALSE);
   
}



/*****************************************************************
                       NewNode
******************************************************************/


void TreeInterface::NewNode(int father,int newNode,int colour,int draw )

   // Creates a new node and enters it into the Tree.
   // Observe that this includes computing new coordinates
   // for all of the nodes of the tree and causes a Repaint in
   // the DrawArea if the variable draw is set to TRUE.

   // THIS PROCEDURE IS ONLY USED BY EXTERNAL PROGRAMMS
{
   _T.enter_new_node(father,newNode,colour);
   if (draw)
   {
      if (father == 0)
         NewTreePaint(TRUE);
      else
         NewTreePaint(FALSE);
   }
   
}




/*****************************************************************
                      RepaintTree
******************************************************************/


void TreeInterface::RepaintTree()

{
   if (!_T.tree_empty())
      NewTreePaint(TRUE);
   else
      printf ("ERROR in TreeInterface::RepaintTree(). No Tree to paint.\n");
}






/*****************************************************************
                      UpperBound
******************************************************************/


void TreeInterface::UpperBound(double bound)

{
   if (_upperBound != bound)
   {
      _upperBound = bound;
      lprintf("UpperBound %f:\n",bound);
   }
}





/*****************************************************************
                      LowerBound
******************************************************************/


void TreeInterface::LowerBound(double bound)

{
   if (_lowerBound != bound)
   {
      _lowerBound = bound;
      lprintf("Lower Bound %f:\n",bound);
   }
}



/*****************************************************************
                     FinishAlgorithm
******************************************************************/


void TreeInterface::FinishAlgorithm(int boolean,int returnvalue)

{
   if (boolean)
   {
      _visualizeAlgo->activate();
      if (_branchAndCutActive)
         _branchAndCutCmd->activate();
      printf("Algorithm has come to a stop.\n");
      if (!returnvalue)
         printf("Program exited normally.\n");
      else
         printf("Program did exit with %d.\n",returnvalue);
      
   }
}

   


/*****************************************************************
                    readCompleteTree
******************************************************************/


void TreeInterface::readCompleteTree(char *filename)

   // Loads a Tree and calls a function for drawing the Tree.
{
      
                                        // Activate some commandsq.
   _visualizeAlgo->activate();

                                        // Load the tree into the class tree.
   filename >> _T;

   if (_T.tree_empty())
      printf("ERROR : FILE CORRUPT\n");
   else
      NewTreePaint(TRUE);

}





/*****************************************************************
                    NewTreePaint
******************************************************************/


void TreeInterface::NewTreePaint(int newTree)

   // Computes the coordinates of a Tree and initializes the
   // graphical class graph. 
   
{
   double  x_max = 0.0;
   double  y_max = 0.0;

   _node_nb = _T.get_node_nb();
                                        // Compute the coordinates of the
					// nodes of the tree.
   _T.positiontree(&x_max,&y_max);

                                        // Initialize the graphical class
					// graph, if necessary
					// and register the nodes.
   graph()->initNodes(_node_nb);
   if (newTree || _firstVisualize)
   {
      for (int i = 1; i <= COLOURCOUNT; i++)
         graph()->setRadius(i,_actualRadius);
      _firstVisualize = FALSE;
   }

   for (int i = 1; i <= _node_nb; i++)
   {
      graph()->registerNode(_T.get_node_colour(i),i,_T.get_Xcoord(i),
                            _T.get_Ycoord(i),TRUE);
   }

                                        // Draw the edges.
				        // Changed after using gcc 2.7.0
   for (int j = 1; j < _node_nb; j++)
      graph()->drawEdge(STANDARDEDGECOLOUR,_T.getnode_edge_first(j),
                          _T.getnode_edge_sec(j));

	                                // Initialize the Coordinate system.
   Rect<CoordType> r;
   if (_node_nb <= 3 )
   {
      x_max = x_max + (4*_actualRadius);
      y_max = y_max + (4*_actualRadius);
      r.newInit(0,0,x_max,y_max);
      r.growPercent(100.0);
   }
   else if ( _node_nb <= 12 )
   {
      x_max = x_max + (2*_actualRadius);
      y_max = y_max + (2*_actualRadius);
      r.newInit(0,0,x_max,y_max);
      r.growPercent(25.0);
   }
   else
   {
      x_max = x_max + (2*_actualRadius);
      y_max = y_max + (2*_actualRadius);
      r.newInit(0,0,x_max,y_max);
      r.growPercent(10.0);
   }
   
   theDrawArea->setCoordinateRange(&r,FALSE);
   

}





/*****************************************************************
                    Mybutton1Motion
******************************************************************/


void TreeInterface::Mybutton1Motion(XEvent *event)

   // After a motion of the mouse with pressed left button,
   // this procedure is called.
{
                                        // Get the point in the coordinate
					// system, where the cursor was
					// registered.
   Point<CoordType> point(theDrawArea->scaler()->xReConvert(event->xmotion.x),
                          theDrawArea->scaler()->yReConvert(event->xmotion.y));

				        // Execute the treeSearcherCmd, if
					// it is active.
					// Nothing will happen if it is not.
   _treeSearcherCmd->executeWithPoint(&point);   

}




/*****************************************************************
                    Mybutton1Release
******************************************************************/


void TreeInterface::Mybutton1Release(XEvent *event)

   // After the left mouse button is released, this procedure is called. 
{
   _treeSearcherCmd->cleanup();
                                       // If the browser mode is still active
				       // we keep the crosshair cursor.
   if (_treeSearcherCmd->active())
     XDefineCursor(theDrawArea->display(),theDrawArea->window(),_cursor);
}




/*****************************************************************
                button1MotionEventHandler
******************************************************************/


void TreeInterface::button1MotionEventHandler(Widget widget,
                             XtPointer clientData,
			     XEvent *event,
			     Boolean *continueToDiapatch)

   // Event Handler for mouse button input. 
{
   TreeInterface* object = (TreeInterface *) clientData;
   object->Mybutton1Motion(event);
}





/*****************************************************************
                buttonReleaseEventHandler1
******************************************************************/


void TreeInterface::buttonReleaseEventHandler(Widget widget,
                             XtPointer clientData,
			     XEvent *event,
			     Boolean *continueToDiapatch)

   // Event Handler for mouse button input. 
{
   TreeInterface* object = (TreeInterface *) clientData;   
   object->Mybutton1Release(event);
}




/*****************************************************************
                       button3Press
******************************************************************/


void TreeInterface::button3Press(Point<CoordType> point)

   // Inherited function for mouse button click on the right
   // button. Is always active.
   // Finds the nearest node next to the position where the button
   // was clicked, and calls up a dialog-menue, containing
   // all important information of the node.
{

				// Get the node next to the clicked position.
   int  a_node = graph()->nextNode(&point);

   if (!_T.tree_empty())
      if (!_T.is_displayed(a_node))
         _treedialog->callDialog(a_node);

}




/*****************************************************************
                   keyPressSpaceBarEventHandler
******************************************************************/


void TreeInterface::keyPressSpaceBarEventHandler(Widget widget,
                             XtPointer clientData,
			     XEvent *event,
			     Boolean *continueToDiapatch)
   
   // Event Handler for any kind of event.
   // if a key is pressed, this procedure is called.
{
   TreeInterface* object = (TreeInterface *) clientData;   
   object->SpaceBarPressEvent(event);
}







/*****************************************************************
                    SpaceBarPressEvent
******************************************************************/


void TreeInterface::SpaceBarPressEvent(XEvent *event)

   // After a key is pressed, while the mouse is in the DrawArea
   // this function is called by keyPressSpaceBarEventHandler.
   // It checks whether the key which is pressed is the Space key
   // and if so, then calls the function nextStep of the class
   // EmulationCmd, to perform the next action in the emulation
   // of a tree algorithm. If there is no emulation is active,
   // nothing will happen.
{
   char buffer[20];
   KeySym keysym;
   XComposeStatus compose;

   if (event->type == KeyPress)
   {
      XKeyEvent* keyevent = (XKeyEvent*) event;
      int charcount = XLookupString(keyevent, buffer, sizeof(buffer),
				 &keysym, &compose);
      if (keysym == XK_space)
         _emulationCmd->nextStep();
   }

}
