/*****************************************************************

            Filename    :  EmlSetupCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/

#include "EmlSetupCmd.h"
#include "GFEWindow.h"



/*****************************************************************
                    Constructor
******************************************************************/


EmlSetupCmd::EmlSetupCmd(char* name,int active)
             : NoUndoCmd(name,active)

   // Constructor
{
   _setupDialog = new EmlSetupDiag("Emulation Setup",
                                      theGFEWindow->baseWidget());
}



/*****************************************************************
                     Destructor
******************************************************************/


EmlSetupCmd::~EmlSetupCmd()

   // Destructor
{
   delete _setupDialog;
}



/*****************************************************************
                      cleanup
******************************************************************/


void EmlSetupCmd::cleanup()

   // If a new graph or problem is loaded, while the Scaler Dialog Window
   // is still posted, the Window has to be removed from the screen.
{
   _setupDialog->unpost();
}



/*****************************************************************
                       doit
******************************************************************/


void EmlSetupCmd::doit()

   // If the Command button Scaler in the menuebar is pressed,
   // this procedure is called. It pops up a window, where the user
   // can choose new scaling parameters
{
   _setupDialog->post();
}


