
#ifndef NODEPOPUP_H
#define NODEPOPUP_H

#include "PopupManager.h"
#include "CCChanger.h"

class NodeCategorie;

class NodePopup : public PopupManager
{
   private:

      NodeCategorie    *_nodeCategorie;

      Widget	  _label;

      CCChanger	  *_colorChanger;
      CCChanger	  *_fontChanger;

      Widget	  _mainRow;

      RadioField  *_number;
      RadioField  *_filled;
      RadioField  *_circle;
      Widget	  _radiusScale;

      Widget	  _buttonRow;
      Widget	  _colorButton;
      Widget	  _fontButton;

      static char *yesno[2];
      static char *circleString[2];

      CoordType	  _maxscaleValue;
      int	  _decimalPoints;
      double	  _decimalFactor;
      static int  _lwsnov;

      static void colorCallback(Widget, XtPointer, XtPointer);
      static void fontCallback(Widget, XtPointer, XtPointer);

   protected:

      void createControlArea();
      void createActionArea();

      void enableCallbacks();
      void disableCallbacks();

      void ok();
      void apply();
      void cancel();

      void nodeColor();
      void nodeFont();

   public:

      NodePopup(char*, Widget);
      ~NodePopup();

      void post(NodeCategorie*);

      virtual const char *const className() { return ("NodePopup"); }
};

#endif
