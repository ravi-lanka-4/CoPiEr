/*****************************************************************

            Filename    :  VBCCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
			   
******************************************************************/

#include "TreeInterface.h"
#include "VBCCmd.h"
#include "UserExample.h"
#include <string.h>


/*****************************************************************
                       VBCCmd
******************************************************************/


VBCCmd::VBCCmd(char* name,int active)
       : NoUndoCmd(name,active)

   // Constructor
{
}



/*****************************************************************
                    setFileName
******************************************************************/


void VBCCmd::setFileName(FILE *ptr,char* filename)

   // The name of the file that has been read as input is set.
{
   strcpy(_FileName,filename);
   _FilePtr = ptr;
}




/*****************************************************************
                        doit
******************************************************************/


void VBCCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It will then call
   // a user defined function.
{
   printf("Execute Clients Program.\n");
   theTreeInterface->Tree()->clean_tree();
   char* arg[3];
   arg[1] = _FileName;
   theTreeInterface->DeactivationCall();
/*
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   Insert the call for the user defined function here.

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/   
   clientmain(1,arg);
}

