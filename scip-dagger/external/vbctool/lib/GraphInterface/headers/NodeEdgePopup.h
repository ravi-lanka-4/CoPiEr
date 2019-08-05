/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      NodeEdgePopup.h
//
//      Version  :      21.02.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////
#ifndef NODEEDGEPOPUP_H
#define NODEEDGEPOPUP_H

#include "NoUndoCmd.h"
#include "PopupManager.h"

class NodePopup;
class CCChanger;

class NodeListPopup : public NoUndoCmd, public PopupManager 
{
   private:

      NodePopup	    *_nodePopup;
      
      XmStringTable  _list;
      int	     _numItems;

   protected:

      void ok();
      void cancel();

      virtual void doit() { post(); }

      void getDialog() { getSelectionDialog(); }
      void createControlArea();
      void createActionArea();

   public:

      NodeListPopup(char*, int, char*, Widget);
      ~NodeListPopup();

};

class EdgeListPopup : public NoUndoCmd, public PopupManager 
{
   private:

      CCChanger     *_edgePopup;

      XmStringTable  _list;
      int	     _numItems;

   protected:

      void ok();
      void cancel();

      virtual void doit() { post(); }

      void getDialog() { getSelectionDialog(); }
      void createControlArea();
      void createActionArea();

   public:

      EdgeListPopup(char*, int, char*, Widget);
      ~EdgeListPopup();

};

#endif
