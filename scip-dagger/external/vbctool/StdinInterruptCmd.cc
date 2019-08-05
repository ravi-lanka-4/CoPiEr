/*****************************************************************

            Filename    :  StdinInterruptCmd.cc

	    Version     :  08.1997

	    Author      :  Joachim Kupke

	    Language    :  C++

            Purpose     :  See .h file.
			   
******************************************************************/

#include "StdinInterruptCmd.h"
#include "TreeInterface.h"


/*****************************************************************
                    EmlInterruptCmd
******************************************************************/


StdinInterruptCmd::StdinInterruptCmd(char* name,int active)
       : NoUndoCmd(name,active)
{
}




/*****************************************************************
                        doit
******************************************************************/


void StdinInterruptCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It simply interrupts
   // the reading of stdin. Observe that it is now possible to
   // continue the process, since calling the doit function activates
   // the command Continue in the menue bar
{
   theTreeInterface->standardInputCmd()->Interrupt();
}



