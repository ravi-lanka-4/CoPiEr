#ifndef NODESELECTION_H
#define NODESELECTION_H

#include "GFEClient.inc"

class NodeSelection : public PopupManager
{
   private:
   
      Widget _mainRow;
      Widget _label;
      InputField *_nodeIF;

      void (*gotoOk) (int);
      void (*gotoCancel) ();

   protected:

      void ok();
      void cancel();

      void createActionArea();
      void createControlArea();
      

   public:

      NodeSelection(char*, Widget);
      ~NodeSelection();

      void post(void (*ok)(int), void (*cancel)());

      void setValue(int);
      int getValue();

};
#endif
