/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	PrintPopup.h
//
//	Version	 :	23.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see PrinterPopup.cc
// 
//
////////////////////////////////////////////////////////////////////////	

#ifndef PRINTPOPUP_H
#define PRINTPOPUP_H


#include "PopupManager.h"


class PrintPopup : public PopupManager
{

   private:

      Widget   _mainRow;
      Widget   _printName;
      Widget   _printFrame;
      Widget   _printTitle;
      Widget   _titlePositionRow;

      RadioField  *_formatRF;
      RadioField  *_orientationRF;
      RadioField  *_titlePositionRF;

      InputField  *_filenameIF;
      InputField  *_formatIF;

      MultipleInputField   *_multipleIF;
      static char *_multipleLabels[];
      char *_multipleDefaults[4];

      static void printTitleTBCallback(Widget, XtPointer, XtPointer);
      void printTitleTB();

      static char *dins[];
      static char *orie[];
      static char *posi[];

   protected:

      void createControlArea();
      void createActionArea();

      void enableCallbacks();
      void disableCallbacks();
      
      void ok();
      void cancel();

   public:

      PrintPopup(char *, Widget);
      ~PrintPopup();

      void post(int, int, int, int);

};

#endif

