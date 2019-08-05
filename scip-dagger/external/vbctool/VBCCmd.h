/*****************************************************************

            Filename    :  VBCCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Starts a Brunch and Cut Algorithm by
	                   Stefan Thienel and visualizes it.
	                   A derived Class from NoUndoCmd.
			   
******************************************************************/

#ifndef VBCCMD_H
#define VBCCMD_H

#include "NoUndoCmd.h"

class VBCCmd : public NoUndoCmd{

public:

   VBCCmd(char*, int);

   void setFileName(FILE *ptr,char* filename);

protected:

   void doit();

private:

   char  _FileName[INFOSIZE];
   FILE  *_FilePtr;

};


#endif

