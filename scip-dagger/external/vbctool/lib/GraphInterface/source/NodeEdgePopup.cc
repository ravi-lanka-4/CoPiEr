/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      NodeEdgePopup.cc
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

#include <Xm/SelectioB.h>
#include <Xm/Text.h>

#include "GFEWindow.h"
#include "NodeEdgePopup.h"
#include "GraphInterface.h"
#include "NodePopup.h"
#include "GFE.inc"

NodeListPopup::NodeListPopup(char *button, int active, char *name, Widget p)
   : NoUndoCmd(button, active), PopupManager(name, p, XmDIALOG_FULL_APPLICATION_MODAL)
{
   _numItems = 0;
   for (int i = 0; i < theGraphInterface->maxNodeCategories(); i++)
      if (!(theGraphInterface->nodeCategorie()[i+1])->locked())
	 _numItems++;

   _list = (XmStringTable) XtMalloc(_numItems * sizeof(XmString *));
   int j = 0;
   for (int i = 0; i < theGraphInterface->maxNodeCategories(); i++)
      if (!(theGraphInterface->nodeCategorie()[i+1])->locked())
	 _list[j++] = XmStringCreateSimple((theGraphInterface->nodeCategorie()[i+1])->customName());

   _nodePopup = NULL;
}

NodeListPopup::~NodeListPopup()
{
   for (int i = 0; i < _numItems; i++)
      XmStringFree(_list[i]);
   XtFree((char*) _list);

   delete _nodePopup;
}

void NodeListPopup::ok()
{
   char *text;
   text = XmTextGetString(XmSelectionBoxGetChild(_w, XmDIALOG_TEXT));

   if (text) {
      int len = strlen(text);
      for (int i = 1; i <= theGraphInterface->maxNodeCategories(); i++)
	 if (!strcmp(theGraphInterface->nodeCategorie()[i]->customName(), text))
	    _nodePopup->post(theGraphInterface->nodeCategorie()[i]);

      XtFree(text);
   }
}

void NodeListPopup::cancel()
{}


void NodeListPopup::createControlArea()
{
   XmString listLabel = XmStringCreateSimple("Registered Node-Categories");
   XmString okString = XmStringCreateSimple("Change");

   XtVaSetValues(_w,
		 XmNlistLabelString,   listLabel,
		 XmNlistItems,	       _list,
		 XmNlistItemCount,     _numItems,
		 XmNokLabelString,     okString,
		 NULL);

   XmStringFree(listLabel);
   XmStringFree(okString);

   _nodePopup = new NodePopup("Look of Nodes", theGFEWindow->baseWidget());
}

void NodeListPopup::createActionArea()
{
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
}





EdgeListPopup::EdgeListPopup(char *button, int active, char *name, Widget p)
   : NoUndoCmd(button, active), PopupManager(name, p, XmDIALOG_FULL_APPLICATION_MODAL)
{
   _numItems = 0;
   for (int i = 0; i < theGraphInterface->maxEdgeCategories(); i++)
      if (!(theGraphInterface->edgeCategorie()[i+1])->locked())
	 _numItems++;

   _list = (XmStringTable) XtMalloc(_numItems * sizeof(XmString *));
   int j = 0;
   for (int i = 0; i < theGraphInterface->maxEdgeCategories(); i++)
      if (!(theGraphInterface->edgeCategorie()[i+1])->locked())
	 _list[j++] = XmStringCreateSimple((theGraphInterface->edgeCategorie()[i+1])->customName());

   _edgePopup = NULL;
}

EdgeListPopup::~EdgeListPopup()
{
   for (int i = 0; i < _numItems; i++)
      XmStringFree(_list[i]);
   XtFree((char*) _list);

   delete _edgePopup;
}

void EdgeListPopup::ok()
{
   CoordType r, rad = 0.0;
   for (int i = 0; i < theGraphInterface->maxNodeCategories(); i++)
      if ((r = theGraphInterface->nodeCategorie()[i+1]->_radius) > rad)
	 rad = r;
   rad *= 2.0;
   if (rad == 0.0) rad = 0.00001;
   for (int i = 0; i < theGraphInterface->maxEdgeCategories(); i++){
      Context *c = theDrawArea->getContext(theGraphInterface->edgeCategorie()[i+1]->outer());
      if (c->lineWidth() > rad)
	 c->setLineWidth(rad);
      theDrawArea->setMaxLineWidth(theGraphInterface->edgeCategorie()[i+1]->outer(), rad);
   }

      

   char *text;
   text = XmTextGetString(XmSelectionBoxGetChild(_w, XmDIALOG_TEXT));

   if (text) {
      int len = strlen(text);
      for (int i = 1; i <= theGraphInterface->maxEdgeCategories(); i++)
	 if (!strcmp(theGraphInterface->edgeCategorie()[i]->customName(), text))
	    if (_edgePopup->setCC(theDrawArea->getContext(
		 theGraphInterface->edgeCategorie()[i]->outer())->customName()) == SUCCESS)
	       _edgePopup->post();

      XtFree(text);
   }
}

void EdgeListPopup::cancel()
{}


void EdgeListPopup::createControlArea()
{
   XmString listLabel = XmStringCreateSimple("Registered Edge-Categories");
   XmString okString = XmStringCreateSimple("Change");

   XtVaSetValues(_w,
		 XmNlistLabelString,   listLabel,
		 XmNlistItems,	       _list,
		 XmNlistItemCount,     _numItems,
		 XmNokLabelString,     okString,
		 NULL);

   XmStringFree(listLabel);
   XmStringFree(okString);

   _edgePopup = new CCChanger("Changer", theGFEWindow->baseWidget(),
			      MAXCHANGERS + 3, theDrawArea->colorContextPopup());

//   _edgePopup = new EdgePopup();
}

void EdgeListPopup::createActionArea()
{
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
}
