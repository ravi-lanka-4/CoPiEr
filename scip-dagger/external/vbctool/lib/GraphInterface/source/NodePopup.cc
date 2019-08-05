#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>
#include <Xm/PushB.h>

#include <Xm/Label.h>

#include "NodePopup.h"
#include "NodeInformation.h"
#include "DrawArea.h"
#include "Context.h"
#include "GFEResourceManager.h"
#include "ColorContext.h"

NodePopup::NodePopup(char *name, Widget p)
   : PopupManager(name, p)
{
   _nodeCategorie = NULL;
   _number = NULL;
   _filled = NULL;
   _circle = NULL;

   _colorChanger = NULL;
   _fontChanger  = NULL;

   GFEResourceManager rsc(STANDARDRESOURCE);
   char *h = rsc.getstring("RadiusScaleNumberOfValues");
   if (h)
      if(sscanf(h, "%d", &_lwsnov)<=0) _lwsnov = 1000;
}


NodePopup::~NodePopup()
{
   kill(_number);
   kill(_filled);
   kill(_circle);
   kill(_colorChanger);
   kill(_fontChanger);
}

void NodePopup::createControlArea()
{
   _mainRow = XtVaCreateManagedWidget("nodePopupMainRow",
				      xmRowColumnWidgetClass,
				      _w,
				      NULL);

   _label = XtVaCreateManagedWidget("label",
				    xmLabelWidgetClass,
				    _mainRow,
				    XmNalignment, XmALIGNMENT_CENTER,
				    NULL);

   Widget r1 = XtVaCreateManagedWidget("r1", xmRowColumnWidgetClass, _mainRow,
				       XmNorientation, XmHORIZONTAL, NULL);

   Widget r2 = XtVaCreateManagedWidget("r2", xmRowColumnWidgetClass, _mainRow,
				       XmNorientation, XmHORIZONTAL, NULL);

   Widget r3 = XtVaCreateManagedWidget("r3", xmRowColumnWidgetClass, _mainRow,
				       XmNorientation, XmHORIZONTAL, NULL);

   Widget l1 = XtVaCreateManagedWidget("Display numbers in nodes:",
				       xmLabelWidgetClass, r1, NULL);

   Widget l2 = XtVaCreateManagedWidget("Fill the nodes:",
				       xmLabelWidgetClass, r2, NULL);

   Widget l3 = XtVaCreateManagedWidget("Nodes are:",
				       xmLabelWidgetClass, r3, NULL);

   _number = new RadioField(r1, "nr", 2, yesno, 2,
			    XmHORIZONTAL, 1, False);

   _filled = new RadioField(r2, "fr", 2, yesno, 2,
			    XmHORIZONTAL, 1, False);

   _circle = new RadioField(r3, "cr", 2, circleString, 1,
			    XmHORIZONTAL, 1, False);

   _radiusScale = XtVaCreateManagedWidget("radiusScale",
					  xmScaleWidgetClass,
					  _mainRow,
					  XmNorientation, XmHORIZONTAL,
					  NULL);

   _buttonRow = XtVaCreateManagedWidget("buttonRow",
					xmRowColumnWidgetClass,
					_mainRow,
					XmNorientation, XmHORIZONTAL,
					NULL);

   XtVaCreateManagedWidget("Change Context of:",
			   xmLabelWidgetClass,
			   _buttonRow,
			   NULL);

   Widget helpRow = XtVaCreateManagedWidget("helpButtonRow",
					    xmRowColumnWidgetClass,
					    _buttonRow,
					    XmNorientation, XmHORIZONTAL,
					    XmNpacking,     XmPACK_COLUMN,
					    NULL);

   _colorButton = XtVaCreateManagedWidget(" Color ",
					  xmPushButtonWidgetClass,
					  helpRow,
					  XmNalignment, XmALIGNMENT_CENTER,
					  NULL);

   _fontButton = XtVaCreateManagedWidget(" Font ",
					 xmPushButtonWidgetClass,
					 helpRow,
					 XmNalignment, XmALIGNMENT_CENTER,
					 NULL);

   _colorChanger = new CCChanger("Changer", _w, MAXCHANGERS + 1, theDrawArea->colorContextPopup());
   _fontChanger  = new CCChanger("Changer", _w, MAXCHANGERS + 2, theDrawArea->colorContextPopup());

}

void NodePopup::createActionArea()
{
   XtManageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
   XtSetSensitive(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON), False);
}

void NodePopup::ok()
{
   apply();
//   theDrawArea->colorContextPopup()->unpostAllSpecifiedCC();
   cancel();
}

void NodePopup::apply()
{
   if (_number->getValue() == 1)
      _nodeCategorie->_withNumbers = True;
   else
      _nodeCategorie->_withNumbers = False;

   if (_filled->getValue() == 1)
      _nodeCategorie->_filled = True;
   else
      _nodeCategorie->_filled = False;

   if (_circle->getValue() == 1)
      _nodeCategorie->_circle = True;
   else
      _nodeCategorie->_circle = False;

   int val;
   XtVaGetValues(_radiusScale, XmNvalue, &val, NULL);
   _nodeCategorie->_radius = val/_decimalFactor;

   theDrawArea->repaint();
}

void NodePopup::cancel()
{
//   theDrawArea->colorContextPopup()->unpostAllSpecifiedCC();
   _colorChanger->unpost();
   _fontChanger->unpost();
}

void NodePopup::post(NodeCategorie *nc)
{
   _nodeCategorie = nc;
   
   if (!_w)
      getDialog();

   XmString str = XmStringCreateSimple(nc->customName());
   XtVaSetValues(_label, XmNlabelString, str,
		 XmNalignment, XmALIGNMENT_CENTER,
		 NULL);
   XmStringFree(str);
   
   if (_nodeCategorie->_withNumbers)
      _number->setValue(1);
   else
      _number->setValue(2);

   if (_nodeCategorie->_filled)
      _filled->setValue(1);
   else
      _filled->setValue(2);

   if (_nodeCategorie->_circle)
      _circle->setValue(1);
   else
      _circle->setValue(2);


   _maxscaleValue = minimum(theDrawArea->coordinateRange()->width(),
			    theDrawArea->coordinateRange()->height())/5;

   for (_decimalPoints = -5; _decimalPoints < 6; _decimalPoints++)
         if (1/(_decimalFactor = pow(10, _decimalPoints)) <= _maxscaleValue/_lwsnov) break;

   XtVaSetValues(_radiusScale,
		 XmNminimum,	    0,
		 XmNmaximum,	    round(_maxscaleValue*_decimalFactor),
		 XmNdecimalPoints,  _decimalPoints,
		 XmNshowValue,      True,
		 XtVaTypedArg,      XmNtitleString, XmRString,
		 "Radius",	    10,
		 XmNscaleMultiple,  1,
		 NULL);

   int lw = round(_nodeCategorie->_radius * _decimalFactor);
   XtVaSetValues(_radiusScale, XmNvalue,
		 lw, NULL);


   PopupManager::post();
}


void NodePopup::colorCallback(Widget, XtPointer clientData, XtPointer)
{
   NodePopup *obj = (NodePopup *) clientData;

   obj->nodeColor();
}

void NodePopup::fontCallback(Widget, XtPointer clientData, XtPointer)
{
   NodePopup *obj = (NodePopup *) clientData;

   obj->nodeFont();
}

void NodePopup::nodeColor()
{
//   theDrawArea->colorContextPopup()->postSpecifiedCC("Nodes");
   if (_colorChanger->setCC(theDrawArea->getContext(_nodeCategorie->outer())->customName()) == SUCCESS)
      _colorChanger->post();
}

void NodePopup::nodeFont()
{
//   theDrawArea->colorContextPopup()->postSpecifiedCC("Node Font");
   if (_fontChanger->setCC(theDrawArea->getContext(_nodeCategorie->inner())->customName()) == SUCCESS)
       _fontChanger->post();
}

void NodePopup::enableCallbacks()
{
   XtAddCallback(_colorButton, XmNactivateCallback,
		 &NodePopup::colorCallback, (XtPointer) this);
		 
   XtAddCallback(_fontButton, XmNactivateCallback,
		 &NodePopup::fontCallback, (XtPointer) this);
		 
   PopupManager::enableCallbacks();
}

void NodePopup::disableCallbacks()
{
   XtRemoveCallback(_colorButton, XmNactivateCallback,
                    &NodePopup::colorCallback, (XtPointer) this);

   XtRemoveCallback(_fontButton, XmNactivateCallback,
                    &NodePopup::fontCallback, (XtPointer) this);

   PopupManager::disableCallbacks();
}

