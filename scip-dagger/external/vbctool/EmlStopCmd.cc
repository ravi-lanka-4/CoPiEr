/*****************************************************************

            Filename    :  EmlStopCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h file.
			   
******************************************************************/

#include "EmlStopCmd.h"
#include "TreeInterface.h"


/*****************************************************************
                    EmlStopCmd
******************************************************************/


EmlStopCmd::EmlStopCmd(char* name,int active)
       : NoUndoCmd(name,active)

   // Constructor
{
}




/*****************************************************************
                        doit
******************************************************************/


void EmlStopCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It simply stops
   // the emualtion process.
{
   theTreeInterface->emulationCmd()->Stop();
}
