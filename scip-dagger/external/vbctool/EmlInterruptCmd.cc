/*****************************************************************

            Filename    :  EmlInterruptCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h file.
			   
******************************************************************/

#include "EmlInterruptCmd.h"
#include "TreeInterface.h"


/*****************************************************************
                    EmlInterruptCmd
******************************************************************/


EmlInterruptCmd::EmlInterruptCmd(char* name,int active)
       : NoUndoCmd(name,active)
{
}




/*****************************************************************
                        doit
******************************************************************/


void EmlInterruptCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It simply interrupts
   // the emulation process. Observe that it is now possible to
   // continue the process, since calling the doit function activates
   // the command Continue in the menue bar
{
   theTreeInterface->emulationCmd()->Interrupt();
}



