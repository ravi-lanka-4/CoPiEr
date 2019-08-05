/*****************************************************************

            Filename    :  TreeDiagManager.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages a PopUp Menue, that shows
	                   the Informations stored in a node.
			   A derived Class from PopupManager.
			   
******************************************************************/


#ifndef TREEDIAGMANAGER_H
#define TREEDIAGMANAGER_H

/*#include "GFE.inc"*/
#include "PopupManager.h"

class TreeDiagManager : public PopupManager {

public:

   TreeDiagManager(int,char*,Widget);
   ~TreeDiagManager();

   virtual void post(char*,char*,int);

   void   changeWindow(char*,char*); 
   int    IsPosted(){ return _isPosted; };

protected:

   virtual void ok();

private:

   virtual void createControlArea();
   virtual void createActionArea();

   Widget         _displayControl;
   Widget         _frameNumber;
   Widget         _displayInfo;
   Widget	  _displayNumber;

   int            _dialogNum;

   int            _node;
   int            _isPosted;

};

#endif

