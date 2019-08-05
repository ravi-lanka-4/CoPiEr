/*****************************************************************

            Filename    :  EmlSetupDiag.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages a PopUp Menue for emulating
	                   algorithms. In this menue, the user
			   is able to chose how much time the
			   emulation should need
			   A derived Class from PopupManager.
			   
******************************************************************/


#ifndef EMLSETUPDIAG_H
#define EMLSETUPDIAG_H

#include "GFE.inc"
#include "PopupManager.h"
#include "def.glb"


class EmlSetupDiag : public PopupManager {

public:

   EmlSetupDiag(char*,Widget);
   ~EmlSetupDiag();
   
   virtual void post();

   
   int   isPosted(){ return _isPosted; };
   void  newTime();

   
protected:


   virtual void cancel();
   virtual void ok();
   virtual void apply();

private:

   virtual void createControlArea();
   virtual void createActionArea();

   void    createControlTimeArea();
   void    createControlRadioArea();


   int _enthour;
   int _entmin;
   int _entsec;
   int _entms;

   Widget _displayControl;
   Widget _timeControl;
   Widget _radioControl;

   Widget _rightText1;
   Widget _rightText2;
   Widget _enterHour;
   Widget _enterMin;
   Widget _enterSec;
   Widget _enterMilli;

   RadioField*  _yesnoStep;
   RadioField*  _yesnoTree;


   int  _isPosted;

};

#endif

