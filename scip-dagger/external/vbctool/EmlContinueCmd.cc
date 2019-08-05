/*****************************************************************

            Filename    :  EmlContinueCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h file.
			   
******************************************************************/

#include "EmlContinueCmd.h"
#include "TreeInterface.h"


/*****************************************************************
                    EmlContinueCmd
******************************************************************/


EmlContinueCmd::EmlContinueCmd(char* name,int active)
       : NoUndoCmd(name,active)
   // Constructor
{
}




/*****************************************************************
                        doit
******************************************************************/


void EmlContinueCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. 

{
   theTreeInterface->emulationCmd()->Continue();
}
