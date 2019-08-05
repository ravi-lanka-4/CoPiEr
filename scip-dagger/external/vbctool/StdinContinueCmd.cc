/*****************************************************************

            Filename    :  StdinContinueCmd.cc

	    Version     :  08.1997

	    Author      :  Joachim Kupke

	    Language    :  C++

            Purpose     :  See .h file.
			   
******************************************************************/

#include "StdinContinueCmd.h"
#include "TreeInterface.h"


/*****************************************************************
                    EmlContinueCmd
******************************************************************/


StdinContinueCmd::StdinContinueCmd(char* name,int active)
       : NoUndoCmd(name,active)
   // Constructor
{
}




/*****************************************************************
                        doit
******************************************************************/


void StdinContinueCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. 

{
   theTreeInterface->standardInputCmd()->Continue();
}
