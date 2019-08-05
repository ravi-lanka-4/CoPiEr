/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      InterruptBasePopup.h
//
//      Version  :      24.02.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////
#ifndef INTERRUPTBASEPOPUP_H
#define INTERRUPTBASEPOPUP_H

#include "PopupManager.h"

class InterruptBasePopup : public PopupManager
{
   private:

   protected:

      void map();

   public:

      InterruptBasePopup(char*);

};

#endif
