/*****************************************************************

            Filename    :  CopyrightCmd.cc

	    Version     :  08.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/

#include "CopyrightCmd.h"
#include "GFEWindow.h"



/*****************************************************************
                    Constructor
******************************************************************/


CopyrightCmd::CopyrightCmd(char* name,int active)
             : NoUndoCmd(name,active)

   // Constructor
{
   _copyrightDiag = new CopyrightDiag("Copyright",
                                      theGFEWindow->baseWidget());
}



/*****************************************************************
                     Destructor
******************************************************************/


CopyrightCmd::~CopyrightCmd()

   // Destructor
{
   delete _copyrightDiag;
}



/*****************************************************************
                      cleanup
******************************************************************/


void CopyrightCmd::cleanup()

   // If a new graph or problem is loaded, while the copyright window
   // is still posted, the window has to be removed from the screen.
{
   _copyrightDiag->unpost();
}



/*****************************************************************
                       doit
******************************************************************/


void CopyrightCmd::doit()

   // If the Command button Scaler in the menuebar is pressed,
   // this procedure is called. It pops up a window, where the user
   // can choose new scaling parameters
{
   _copyrightDiag->post();
}


