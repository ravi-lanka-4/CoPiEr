////////////////////////////////////////////////////////////////////////
//
//    Filename :  GFEWindow.cc
//
//    Version  :  24.03.94
//
//    Author   :  Martin Diehl & Joachim Kupke
//
//    Language :  C++
//
//    Purpose  :  see GFEWindow.h, creates DrawArea and DisplayArea
//
//
////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <Xm/PanedW.h>

#include "Application.h"
#include "GFEWindow.h"
#include "DrawArea.h"
#include "CmdList.h"
#include "Cmd.h"
#include "QuitCmd.h"
#include "ManageCmd.h"
#include "UndoCmd.h"
#include "NoUndoCmd.h"
#include "MenuBar.h"
#include "Geometric.h"
#include "MenuWindow.h"
#include "ClientInterface.h"
#include "LoadCmd.h"
#include "SaveCmd.h"
#include "DisplayArea.h"
#include "Filenames.h"

// my quit for freeing memory before exit

class GFEWindow;

class QuitWithMemoryFreeCmd : public QuitCmd
{
   protected:

      void doit()
      {
	 delete theGFEWindow;
	 exit(0);
      }

   public:

      QuitWithMemoryFreeCmd (char *name, int active) : QuitCmd(name, active)
      {}
};


GFEWindow::GFEWindow(char *name) : MenuWindow(name)
{
   theGFEWindow = this;

   _drawArea	    = NULL;
   _displayArea	    = NULL;
}

GFEWindow::~GFEWindow()
{
   delete theClientInterface;
   delete _drawArea;
   delete _displayArea;
}

Widget GFEWindow::createWorkArea( Widget parent)
{
   Widget pane = XtCreateWidget ("workArea",
				 xmPanedWindowWidgetClass,
				 parent,
				 NULL, 0);

   // Create a DrawArea as the WorkArea
   Rect<double> coordRange(0.0, 0.0, 1.0, 1.0);

   Boolean show;
   if (theClientInterface->configuration() & DISPLAYAREA)
      show = True;
   else show = False;

   _displayArea = new DisplayArea(pane, "displayArea", show);

   _drawArea = new DrawArea(pane, 
			    "drawArea",
			    coordRange,
			    True,
			    _displayArea);

   if (show)
      _displayArea->manage();

   _drawArea->manage();
   
   return (pane);
}

void GFEWindow::createMenuPanes()
{
   // Call the ClientInterfaces init() function
   theClientInterface->cmdInit();
  
   // Create the FILE menu

   CmdList  *cmdListFile = new CmdList();
   CmdList  *cmdListEdit = new CmdList();
   CmdList  *cmdListView = new CmdList();

   cmdListFile->add(theClientInterface->loadCmd());
   cmdListFile->add(theClientInterface->saveCmd());
   
   Cmd	    *quit    = new QuitWithMemoryFreeCmd("Quit", True);

   // Create the EDIT menu, with just the undo cmd

   cmdListEdit->add(theUndoCmd);

   // Call the draw area's function that attach its menu entries
   // to the menu bar.
   _drawArea->createMenuEntries(cmdListFile, cmdListEdit, cmdListView);
   theClientInterface->addToExistingMenu(cmdListFile, cmdListEdit, cmdListView);

   cmdListFile->add(quit);

   _menuBar->addCommands(cmdListFile, "File");
   _menuBar->addCommands(cmdListEdit, "Edit");
   _menuBar->addCommands(cmdListView, "View");

   delete(cmdListFile);
   delete(cmdListEdit);
   delete(cmdListView);

   theClientInterface->init();
}
