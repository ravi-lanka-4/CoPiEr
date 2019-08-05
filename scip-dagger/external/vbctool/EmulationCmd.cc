/*****************************************************************

            Filename    :  EmulationCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
			   
******************************************************************/

#include "TreeInterface.h"
#include "EmulationCmd.h"
#include "def.glb"
#include <iostream>
#include <string.h>
#include <fstream>


/*****************************************************************
                    EmulationCmd
******************************************************************/


EmulationCmd::EmulationCmd(char* name,int active)
       : NoUndoCmd(name,active)

   // Constructor
{

   hour = 0;
   min = 0;
   sec = 0;
   millisec = 0;
   maxhour = 0;
   maxmin = 0;
   maxsec = 0;
   maxmilli = 0;
   position = 0;
   
   _time = 0;
   _totaltime = 0;
   _lasttime = 0;
   _scaler = 1;
   _interruptInterval = 500;
   
   STOPPED = TRUE;
   INTERRUPT = FALSE;
   STEPWISE = FALSE;
   COMPLETE = FALSE;
   MODECOMPLETE = FALSE;
   STEPWISECHANGED = FALSE;
   
   _app = theApplication->appContext();
}



/*****************************************************************
                    ~EmulationCmd
******************************************************************/


EmulationCmd::~EmulationCmd()

   // Destructor

{
}







/*****************************************************************
                    setFileName
******************************************************************/


void EmulationCmd::setFileName(char* filename)

   // The name of the file that has been read as input is set.
   // Furthermore the time, that is needed by this emulation is
   // read and initialized, so when the user calls the Setup tool
   // we are able to present him this time.
{
   strcpy(_FileName,filename);

   int    endline = 0;    
   char   infochar;
   
   std::ifstream inClientFile(_FileName,std::ios::in);

			               // The time needed by the algorithm
				       // is obviously the last time
				       // entrance in the file. So go to
				       // the end of the file and get this
				       // entrance.
   inClientFile.seekg(0,std::ios::end);
   inClientFile.seekg(-1,std::ios::cur);
   while (!endline)
   {
      inClientFile.seekg(-1,std::ios::cur);
      if ((infochar = inClientFile.peek()) == '\n')
         endline = TRUE;
   }
   inClientFile.seekg(1,std::ios::cur);
   inClientFile >> maxhour;
   inClientFile.seekg(1,std::ios::cur);
   inClientFile >> maxmin;
   inClientFile.seekg(1,std::ios::cur);
   inClientFile >> maxsec;
   inClientFile.seekg(1,std::ios::cur);
   inClientFile >> maxmilli;

   _totaltime = (maxhour*3600000)+(maxmin*60000)+(maxsec*1000)+(maxmilli*10);


}




/*****************************************************************
                        doit
******************************************************************/


void EmulationCmd::doit()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It starts an emulation
   // of any kind of tree algorithm by reading the time label in front
   // of every action and perfoming this action corresponding
   // to the time in which the user wants to see the algorithm.
   // This time is by default real time.

{
   char   inputline[INFOSIZE];
                                       // Reinitialize some of the variables
				       // that might have been changed by an
				       // earlier call of the function.
   STOPPED = FALSE;
   INTERRUPT = FALSE;
   position = 0;
   _lasttime = 0;
   lprintf("");


   std::ifstream inClientFile(_FileName,std::ios::in);
                                       // Throw the first four lines away.

   inClientFile.seekg(0,std::ios::beg);
   inClientFile.getline(inputline,INFOSIZE);
   inClientFile.getline(inputline,INFOSIZE);
   inClientFile.getline(inputline,INFOSIZE);
   inClientFile.getline(inputline,INFOSIZE);
   inClientFile.getline(inputline,INFOSIZE);

   char *input_ptr = inputline;
   if (!strcmp(input_ptr,FIFTHLINE_2))
   {
      inClientFile.getline(inputline,INFOSIZE);   
      inClientFile.getline(inputline,INFOSIZE);   
   }

                                       // In case there has been a tree
				       // erlier: Clean it up.
   theTreeInterface->Tree()->clean_tree();

                                       // The user might have already called
				       // the Setup tool and decided that he
				       // wants to have the complete tree
				       // shown. In this case read the tree
				       // initialize the datastructur tree
				       // and draw it in STANDARDSHADECOLOUR.
   if (COMPLETE)
      MODECOMPLETE = TRUE;
   if (MODECOMPLETE)
   {
      MODECOMPLETE = FALSE;
      computeTree(0);
   }
                                       // Now start the emulation.
   installTimer(inClientFile);
}





/*****************************************************************
                    actionCallback
******************************************************************/


void EmulationCmd::actionCallback(XtPointer clientData, XtIntervalId *)

   // This procedure is called after the time intervall is finished.
   // It calls the procedure nextPaint of this object.
{
   EmulationCmd * object = (EmulationCmd *) clientData;
   object->nextAction();
}






/*****************************************************************
                      nextAction
******************************************************************/


void EmulationCmd::nextAction()

   // The Procedure nextPaint is envoked after the finish of the time
   // intervall and called by the procedure paintCallback.
   // Or it is called when the user performing stepwise action
   // calls the next next action by pressing the Space key (See
   // TreeInterface for more detail.).
   // nextAction will read the current line of the file and proceed
   // according to the command of this line. Then it calls the function
   // installTimer in order to start the next time intervall to wait until
   // the next action has to be performed, or to wait until the user again
   // presses the Space key.
{

   int    node = 0;
   int    father = 0;
   int    colour = 0;
   char   inputline[INFOSIZE];
   char*  input_ptr = NULL;
   char   ident;
   char   infochar;
   double bound;

                                       // If the user has decided during
				       // the emulation that he wishes to
				       // see the complete tree, initialize
				       // the missing nodes of the tree and
				       // draw the tree in STANDARDSHADECOLOUR.
   if (!STOPPED && MODECOMPLETE)
   {
      MODECOMPLETE = FALSE;
      computeTree(position);
   }

                                       // During a time intervall, while
				       // the emulation has been waiting,
				       // the user might have stopped or
				       // interrupted the emulation. So
				       // do nothing.
   if (!STOPPED && !INTERRUPT)
   {
                                       // Open the file for reading.
      std::ifstream inClientFile(_FileName,std::ios::in);
                                       // Jump to the last known position, 
				       // where we have been reading and go on
				       // reading there.
      inClientFile.seekg(position,std::ios::beg);

                                       // Check what kind of identifier this
				       // line has, in order to decide what
				       // command has to be performed.
				       // Observe that the time, which comes
				       // first in every line, has been already
				       // read during the installation of
				       // the timer.
      inClientFile >> ident;
      switch (ident)
      {
         case 'I':                     // Line contains information of a node.

            inClientFile >> node;
            if ( (infochar = inClientFile.peek()) == ' ')
				       // If the first char of the information
				       // is a blanc, skip it.
               infochar = inClientFile.get();
	                               // Get the rest of the line.
            inClientFile.getline(inputline,INFOSIZE);
            input_ptr = inputline;
            theTreeInterface->SetNodeInfo(node,input_ptr);
	    if (STEPWISE)
	       printf("New Information received at node %d.\n",node);
            break;


	 case 'A':                     // Line conatins new information that
	                               // that has to be added to the old one.

	    inClientFile >> node;
            inClientFile.getline(inputline,INFOSIZE);
            input_ptr = inputline;
            theTreeInterface->AddNodeInfo(node,input_ptr);
	    if (STEPWISE)
	       printf("New Information added at node %d.\n",node);
            break;


	 case'N':                      // Line contains a new node that
	                               // has to be added to the tree.

	    inClientFile >> father >> node >> colour;
	    
	    if (COMPLETE)
	                               // The Tree is already complete, since
				       // the user decided to see the complete
				       // tree during the emulation. So we
				       // do not add the node to the tree
				       // just change its colour as assigned
				       // in the input.
	       theTreeInterface->PaintNode(node,colour);
	       
	    else
	                               // Add the node to the tree.         
               theTreeInterface->NewNode(father,node,colour);
	    break;


	 case 'P':                     // Line contains a new colour of a node.
	 
            inClientFile >> node >> colour;
            theTreeInterface->PaintNode(node,colour);
	    break;
	  

	 case 'U':                     // Line contains a new upper bound.

	    inClientFile >> bound;
	    theTreeInterface->UpperBound(bound);
	    break;
	    

	 case 'L':                     // Line contains a new lower bound.

	    inClientFile >> bound;
	    theTreeInterface->LowerBound(bound);
	    break;
	    
      }

                                       // Install the timer again in order
				       // to perform the next step.
				       // This is done even when the user
				       // whishes to go on stepwise.
      installTimer(inClientFile);
   }


}



/*****************************************************************
                      installTimer
******************************************************************/


void EmulationCmd::installTimer(std::istream &inClientFile)

   // The procedure is called by the procedure nextPaint in order
   // to install the timer again.
{
  
   if (inClientFile >> hour)
                                       // There are still some actions
				       // to perform.
   {
                                       // Get the time of the next action.
				       // This is done even when the user
				       // whishes to perform stepwise
				       // emulation. We need the time in 
				       // order to show the remaining time
				       // in the setup dialog window.
      inClientFile.seekg(1,std::ios::cur);
      inClientFile >> min;
      inClientFile.seekg(1,std::ios::cur);
      inClientFile >> sec;
      inClientFile.seekg(1,std::ios::cur);
      inClientFile >> millisec;

                                       // Remember the position in file
				       // so we can go on reading there
				       // when the next action has to be
				       // performed.
      position = inClientFile.tellg();

      _time = (hour*3600000) + (min*60000) + (sec*1000) + (millisec*10);
      unsigned long _timediff = _time - _lasttime;
                                       // _timediff now is the timedifference
				       // in milliseconds between the last
				       // and the next action.
      _interval = int (_timediff/_scaler);
                                       // _interval is now the timedifference
				       // in milliseconds, after it has been
				       // scaled by the users scaler. By
				       // default _scaler = 1.
      _lasttime = _time;

                                       // If the setup dialog window is posted
				       // place the remaing time in the window.
      if (_interval > 0 && theTreeInterface->emlSetupCmd()->isPosted())
         theTreeInterface->emlSetupCmd()->newTime();

      if (!STEPWISE)
                                       // If we do not perform stepwise
				       // action,start the timer.
      {
         if (_interval > 0)
         {
            _timerID = XtAppAddTimeOut(_app,_interval,
	                               &EmulationCmd::actionCallback,
				       (XtPointer) this);
         }
         else
            nextAction();
      }
   }
   else
      Stop();
   
}




/*****************************************************************
                       Interrupt
******************************************************************/


void EmulationCmd::Interrupt()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It interrupts an
   // emulation and starts a timer, which checks after a certain time
   // if the user wishes to continue the programm.
{
   if (!STOPPED)
   {
      INTERRUPT = TRUE;
      installInterruptTimer();   
      theTreeInterface->emlInterruptCmd()->deactivate();
      theTreeInterface->emlContinueCmd()->activate();
   }
   
}



/*****************************************************************
                    interruptCallback
******************************************************************/


void EmulationCmd::interruptCallback(XtPointer clientData, XtIntervalId *)

   // This procedure is called after the interrupt time intervall is finished.
   // It calls the procedure interruptAction of this object.
{
   EmulationCmd * object = (EmulationCmd *) clientData;
   object->interruptAction();
}




/*****************************************************************
                    interruptAction
******************************************************************/


void EmulationCmd::interruptAction()

   // The Procedure interruptAction is envoked after the finish of the time
   // interrupt intervall and called by the procedure interruptCallback.
   // The emulation has been interrupted and this function checks after
   // the time interrupt intervall has been finished, if the user decided
   // to continue the emulation. If the user decided to continue the
   // emulation, we continue performing the next action. If the user decided
   // to stop the emulation, nothing is done. We simply let it die.
   // If the user did nothing, we again install the interrupt timer.
{
   if (INTERRUPT && !STOPPED)
      installInterruptTimer();

   else if (!STOPPED)
      nextAction();
}




/*****************************************************************
                   installInterruptTimer
******************************************************************/


void EmulationCmd::installInterruptTimer()

   // The procedure is called by the procedures Interrupt and
   // interruptAction in order to install the interrupt timer again.
{
   _interruptID = XtAppAddTimeOut(_app,_interruptInterval,
                                  &EmulationCmd::interruptCallback,
                                  (XtPointer) this);      
}





/*****************************************************************
	                   Continue
******************************************************************/


void EmulationCmd::Continue()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It continues an 
   // interrupted emulation.
{
   if (!STOPPED)
   {
      INTERRUPT = FALSE;
      theTreeInterface->emlInterruptCmd()->activate();
      theTreeInterface->emlContinueCmd()->deactivate();
   }

}



/*****************************************************************
			Stop
******************************************************************/


void EmulationCmd::Stop()

   // The Procedure is called when the button in the menue bar,
   // that belongs to this command, is pressed. It stops an 
   // emulation.
{
   if (!STOPPED)
      lprintf("\n Emulation Stopped.\n\n");

   STOPPED = TRUE;
   hour = 0;
   min = 0;
   sec = 0;
   millisec = 0;
//   _scaler = 1;
   this->activate();
   
   theTreeInterface->emlInterruptCmd()->deactivate();
   theTreeInterface->emlStopCmd()->deactivate();
}




/*****************************************************************
                   getRemainingTime
******************************************************************/


void EmulationCmd::getRemainingTime(int* h,int* m,int* s,int* ms)

   // A procedure used mainly by EmlSetupDiag.*, the setup dialog
   // window. Gets the remaing time, that means the difference of
   // the time used so far and the total time
{
   *ms = maxmilli;
   *s = maxsec;
   *m = maxmin;
   *h = maxhour;

   if (*ms < millisec)
   {
      *ms = *ms + 1000 - millisec;
      (*s)--;
   }
   else
      *ms = *ms - millisec;

   if (*s < sec)
   {
      *s = *s + 60 - sec;
      (*m)--;
   }
   else
      *s = *s - sec;

   if (*m < min)
   {
      *m = *m + 60 - min;
      (*h)--;
   }
   else
      *m = *m -min;

   if (*h >= hour)
      *h = *h - hour;
   else
      printf("ERROR in EmulationCmd.cc while computing Time-Difference.\n");
}





/*****************************************************************
                   interpretScaler
******************************************************************/


void EmulationCmd::interpretScaler(int h,int m,int s,int ms)

   // If the user decided to run the remaining time in h:m:s.ms
   // time, this procedure is called in EmlSetupDiag.** .
   // It calculates the scaler for the time intervalls. 
{
   int rh = 0;
   int rm = 0;
   int rs = 0;
   int rms = 0;

   
   getRemainingTime(&rh,&rm,&rs,&rms);

   
   int Rtime = (rh*3600000)+(rm*60000)+(rs*1000)+rms;
   int time = (h*3600000)+(m*60000)+(s*1000)+ms;

   _scaler = double(Rtime)/double(time);
   if (time == Rtime)
      _scaler = 1;
   
}
   




/*****************************************************************
                      nextStep
******************************************************************/


void EmulationCmd::nextStep()

    // Performs the next action when the user is in stepwise mode
    // and has pressed the space key.
{
   if (STEPWISE)
      nextAction();
   else if (STEPWISECHANGED == TRUE)
   {
      STEPWISECHANGED = FALSE;
      nextAction();
   }
   
}





/*****************************************************************
                    comleteTree
******************************************************************/


void EmulationCmd::completeTree(int boolean)

   // The user wishes to see the complete tree. This
   // procedure set the flags.
{
   if (COMPLETE == FALSE && boolean == TRUE)
   {
      COMPLETE = boolean;
      MODECOMPLETE = TRUE;
   }     
   if (STOPPED)
      COMPLETE = boolean;
}



/*****************************************************************
                      computeTree
******************************************************************/


void EmulationCmd::computeTree(int filepos)

   // If the user wishes to see the complete tree during the emulation
   // this procedure draws it.
   // Therefor we run through the file skipping all lines with node
   // informations and new colours, reading only those lines, where
   // a new node is introduced into the tree. We build the tree according
   // to this information, giving every node STANDARDSHADECOLOUR, and then
   // paint it.
{

   char   inputline[INFOSIZE];
   char   ident;
   int    father = 0; 
   int    node = 0;
   int    colour = 0;
   int    help;
   
   std::ifstream inClientFile(_FileName,std::ios::in);
   if (filepos == 0)
   {
                                       // Skip the first four lines,
				       // if necessary.
      inClientFile.getline(inputline,INFOSIZE);
      inClientFile.getline(inputline,INFOSIZE);
      inClientFile.getline(inputline,INFOSIZE);
      inClientFile.getline(inputline,INFOSIZE);
      inClientFile.getline(inputline,INFOSIZE);   
   }
   else
   {
      inClientFile.seekg(position,std::ios::beg);
      inClientFile >> ident;
                                       // If position != 0, the file must
				       // have stoped after a time label.
				       // Thus get the next identifier.
                                       // Check the line identifier.
				       // If line contains a new node,
				       // enter the node into the tree.
				       // Else skip the line.
      if (ident == 'N')
      {
         inClientFile >> father >> node >> colour;
         theTreeInterface->NewNode(father,node,STANDARDSHADE,FALSE);
      }
      else
         inClientFile.getline(inputline,INFOSIZE);
   }
   
   while (inClientFile >> help)
   {
                                       // Skip the time label.
      inClientFile.seekg(1,std::ios::cur);
      inClientFile >> help;
      inClientFile.seekg(1,std::ios::cur);
      inClientFile >> help;
      inClientFile.seekg(1,std::ios::cur);
      inClientFile >> help;

      inClientFile >> ident;

                                       // Check the line identifier.
				       // If line contains a new node,
				       // enter the node into the tree.
				       // Else skip the line.
      if (ident == 'N')
      {
         inClientFile >> father >> node >> colour;
         theTreeInterface->NewNode(father,node,STANDARDSHADE,FALSE);
      }
      else
         inClientFile.getline(inputline,INFOSIZE);

   }
   theTreeInterface->RepaintTree();

}



