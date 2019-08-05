/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	ColorContext.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see ColorContext.cc
// 
//
////////////////////////////////////////////////////////////////////////	

#ifndef COLORCONTEXT_H
#define COLORCONTEXT_H

#define MAXCHANGERS  2 // changer from 0 to 2
#define MAXCOLORS    800

// ColorContext - Test-Class

#include "PopupManager.h"

class CCChanger;
class FontInformation;

class ColorContext : public PopupManager
{
   friend class CCChanger;

   private:

      CCChanger	  *_changerList[MAXCHANGERS + 1];
      int	  _countChangers;

      XmStringTable  _colorList;
      int	     _colorListItems;

      FontInformation	*_fontInformation;

      XmStringTable  _fontList;
      int	     _fontListItems;

      void oneCleared(int);

   protected:

      void createControlArea();
      void createActionArea();
      
      void getDialog();

      void ok();
      void cancel();

   public:

      ColorContext(char *, Widget);
      ~ColorContext();

      void post();

      void postSpecifiedCC(char *);
      void unpostAllSpecifiedCC();

};

#endif

