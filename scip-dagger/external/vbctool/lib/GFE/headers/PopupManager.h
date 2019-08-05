/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	PopupManager.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see .cc
// 
//
////////////////////////////////////////////////////////////////////////	

// abstract Base-Classes for Popup-Menus

#ifndef POPUPMANAGER_H
#define POPUPMANAGER_H

#include "GFE.inc"
#include "UIComponent.h"

class PopupManager : public UIComponent
{   
   private:

      // Callbacks:

      static void okCallback(Widget, XtPointer, XtPointer);
      static void cancelCallback(Widget, XtPointer, XtPointer);
      static void applyCallback(Widget, XtPointer, XtPointer);
      static void helpCallback(Widget, XtPointer, XtPointer);

      static void mapCallback(Widget, XtPointer, XtPointer);

      unsigned char _modality;

   protected:

      Widget	     _parent;

      void getDefaultDialog();
      void getSelectionDialog();

      void catenate(Widget*, Widget*, Widget*, Widget*, Widget*, int);


      virtual void ok() {}
      virtual void cancel() {}
      virtual void apply() {}
      virtual void help() {}

      virtual void map() {}

      virtual void createActionArea() {}
      virtual void createControlArea() = 0;

      virtual void enableCallbacks();
      virtual void disableCallbacks();

      virtual void getDialog();

   public:

      PopupManager(char *, Widget, unsigned char modality = XmDIALOG_MODELESS);
      ~PopupManager();

      virtual void post(); // allow to add your displayed values before posting
      void unpost();

      void centerPopupShell(Widget);

};




// other classes with combined Widgets for using in a Popup-menu (s.c. fields)

class InputField
{
   private:

      Widget   _row;
      Widget   _label;
      Widget   _text;

   public:

      InputField(Widget parent, char *row, char* label, char* defaultText,
		 unsigned char orientation, Bool frame, Bool textbox = True);

      char *getValue(); // You have to free the returned Pointer with XtFree(p) after using
      void setValue(char *);
      Widget row() { return _row; }
};


class RadioField
{
   private:

      Widget   _row;
      Widget   *_buttons;
      int      _numberOfButtons;

   public:

      RadioField(Widget parent, char *row, int numberOfButtons,
		 char **buttonLabels, int activeButton,
		 unsigned char orientation, short columns, Bool frame);

      ~RadioField();

      int getValue();
      void setValue(int);
      Widget row() {return _row; }
};

class MultipleInputField
{
   private:

      Widget	  _row;
      Widget	  *_text;
      
      int	  _numberOfInputs;

   public:

      MultipleInputField(Widget parent, char *row, char *label, int numberOfInputs,
			 char **inputLabels, unsigned char orientation, short columns,
			 Bool frame);

      ~MultipleInputField();

      char **getValues(); // don't forget to free the return-value with kill(p, c)
      Widget row() { return _row; };
      void setDefaults(char **inputDefaults);
};

#endif
