/*****************************************************************

            Filename    :  StandardInputCmd.cc

	    Version     :  1.1.0 1997

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h file.
			   
******************************************************************/

#include "StandardInputCmd.h"
#include "TreeInterface.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>

#ifndef LINUX

#include <stropts.h>
#include <poll.h>

#else

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#endif


/*****************************************************************
   All member-function-calls of the input-stream "std::cin" in the class
   "StandardInputCmd" are linked (e.g. replaced) to the following
   functions. In case of using LINUX you have to compile with the
   option "-DLINUX" and then the input-functions will use the
   Standard-C-library for performing the input. Otherwise the
   "std::cin"-stream-member-functions will be used.
*****************************************************************/

inline
int  StandardInputCmd::_peek()
{
#ifdef LINUX
   int c = fgetc(stdin);
   fseek(stdin, -1, SEEK_CUR);
   return c;
#else
   return std::cin.peek();
#endif
}

inline
std::istream &StandardInputCmd::_getline(char *str, int lng, char trenn)
{
#ifdef LINUX
   int i;
   for (i = 0; i < lng-1; i++) {
      int c = fgetc(stdin);
      if (c == EOF || c == trenn) break;
      str[i] = (char) c;
   }
   str[i] = '\0';
#else
   return std::cin.getline(str, lng, trenn);
#endif
}

inline
int  StandardInputCmd::_get()
{
#ifdef LINUX
   return fgetc(stdin);
#else
   return std::cin.get();
#endif
}



/*****************************************************************
                    StandardInputCmd
******************************************************************/


StandardInputCmd::StandardInputCmd(char* name,int active)
       : NoUndoCmd(name,active)
   // Constrcuctor
{
    _interrupted = FALSE;
}




/*****************************************************************
                        doit
******************************************************************/


void StandardInputCmd::doit()

   // The procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed.
   // If the client wishes to pipe the output of a program to
   // the TreeInterface, he has to push after calling the
   // TreeInterface the corrsponding button in the menue bar,
   // which envokes this procedure. It then calls the function
   // standardIn for reading the input on standard in.

{
   theTreeInterface->ActivationCall();
   standardIn();
}



/*****************************************************************
                      standardIn
******************************************************************/

void StandardInputCmd::standardIn()

   // This function reads the information that is sent to the
   // TreeInterface on standard in. After reading information
   // it will immediately do the necessary actions and repaints.
{

   char inputline[INFOSIZE];
   char inputline1[INFOSIZE];
   char inputline2[INFOSIZE];
   char inputline3[INFOSIZE];
   char inputline4[INFOSIZE];
   char inputline5[INFOSIZE];
   char *inputptr1;
   char *inputptr2;
   char *inputptr3;
   char *inputptr4;
   char *inputptr5;

   char    ident;

   int     error  = FALSE;
   int     delay  = TRUE;
   int     head   = 6;
   int     father = 6;
   int     colour = 6;
   double  bound  = 0.0;


   
   while (((ident = _peek()) != '$') && (ident != EOF))
   {
	                                  // Standard Information received.
					  // This information will be printed
					  // in the right Display Menue.
      _getline(inputline1,INFOSIZE);
      printf(inputline1);
      printf("\n");
   }




					  // Get the identifiers.
   if (_peek() == '$')
   {
      ident = _get();
      _getline(inputline1,INFOSIZE);
      inputptr1 = inputline1;
   }
   else
   {
      printf("ERROR: StandardInput corrupt.\n");
      error = TRUE;
   }

   if (_peek() == '$')
   {
      ident = _get();
      _getline(inputline2,INFOSIZE);
      inputptr2 = inputline2;
   }
   else
   {
      printf("ERROR: StandardInput corrupt.\n");
      error = TRUE;
   }

   if (_peek() == '$')
   {
      ident = _get();
      _getline(inputline3,INFOSIZE);
      inputptr3 = inputline3;
   }
   else
   {
      printf("ERROR: StandardInput corrupt.\n");
      error = TRUE;
   }

   if (_peek() == '$')
   {
      ident = _get();
      _getline(inputline4,INFOSIZE);
      inputptr4 = inputline4;
   }
   else
   {
      printf("ERROR: StandardInput corrupt.\n");
      error = TRUE;
   }

   if (_peek() == '$')
   {
      ident = _get();
      _getline(inputline5,INFOSIZE);
      inputptr5 = inputline5;
   }
   else
   {
      printf("ERROR: StandardInput corrupt.\n");
      error = TRUE;
   }




                                          // Check if the information
					  // will be sent according to
					  // the identifiers in correct
					  // format.
   if ((!error)				 &&
       (!strcmp(inputptr1,FIRSTLINE))    &&
       (!strcmp(inputptr2,SECONDLINE_1)) &&
       (!strcmp(inputptr4,FOURTHLINE_1)) &&
       (!strcmp(inputptr5,FIFTHLINE_1)))
   {
                                          // Do necessary cleanups.
      theTreeInterface->Tree()->clean_tree();
      theTreeInterface->Tree()->set_standard(TRUE);

                                          // Start reading from stdin
					  // and do not stop until the
					  // following line is read:
					  // #END_OF_OUTPUT.
      while(1)
      {
#ifndef LINUX
	  struct pollfd pfd;
	  pfd.fd = fileno(stdin);
	  pfd.events = POLLIN;
	  while ((!poll(&pfd,1,100)) || _interrupted)
	     dispatchEvents();
#else
	  fd_set rfds;
	  struct timeval tv;
	      
	  do {
	      /* Watch stdin (fd 0) to see when it has input. */
	      FD_ZERO(&rfds);
	      FD_SET(0, &rfds);
	      /* Wait up to five seconds. */
	      tv.tv_sec = 0;
	      tv.tv_usec = 100;

	      dispatchEvents();

	  } while(!select(1, &rfds, NULL, NULL, &tv) || _interrupted);
#endif

                                          // Get the next line from stdin.
         _getline(inputline2,INFOSIZE);
	                                  // Check if end of input is read.
	 if (!strcmp(inputline2,LASTLINE))
	    break;

	 else
	 {
	   std::istringstream instring(std::string(inputline2,INFOSIZE));
	                                  // Get the next identifier.

	    dispatchEvents();
	    
	    if (instring.peek() == '$')
	    {
	                                  // Line contains specific
					  // informations of the tree.
	       ident = instring.get();
	       ident = instring.get();


	       switch (ident)
	                                  // Make decessions according
					  // To the identifiers.
	       {
	          case 'A':   instring >> head;
	                                  // Add information to a node.
	                      instring.getline(inputline2,INFOSIZE);
			      inputptr2 = inputline2;
			      theTreeInterface->AddNodeInfo(head,inputptr2);
			      break;

	          case 'I':   instring >> head;
	                                  // Set information of a node.
	                      instring.getline(inputline2,INFOSIZE);
			      inputptr2 = inputline2;
			      theTreeInterface->SetNodeInfo(head,inputptr2);
			      break;

	          case 'L':   instring >> bound;
	                                  // Print out the lower bound.
	                      theTreeInterface->LowerBound(bound);
			      break;
	       
	          case 'N':   instring >> father >> head >> colour;
	                                  // Add a new node to the tree.
			      theTreeInterface->NewNode(father,head,colour);
			      break;

	          case 'D':   instring >> father >> head >> colour >> delay;
	                                  // Add a new node to the tree.
			      theTreeInterface->NewNode(father,head,colour,delay);
			      break;

	          case 'P':   instring >> head >> colour;
	                                  // Paint a node in a new colour.
	                      theTreeInterface->PaintNode(head,colour);
			      break;

	          case 'U':   instring >> bound;
	                                  // Print out the upper bound.
	                      theTreeInterface->UpperBound(bound);
			      break;
	       }
	    }

	    else
	    {
	                                  // Standard Information received.
					  // This information will be printed
					  // in the right Display Menue.
	       theDisplayArea->printRight(inputline2);
	       printf("\n");
	    }
	 }
	 dispatchEvents();
      }
   }
      
   else
      printf("ERROR: StandardInput corrupt.\n");
}

