/*****************************************************************

            Filename    :  TreeDiagManager.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/


#include "GFE.inc"
#include "PopupManager.h"
#include "TreeDiagManager.h"
#include "TreeInterface.h"
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/SelectioB.h>


/*****************************************************************
                TreeDiagManager
******************************************************************/


TreeDiagManager::TreeDiagManager(int dialogNum,char* a,Widget d)
   : PopupManager(a,d)   // call base-class constructor

   // Constructor.
{
   _isPosted = FALSE;
   _dialogNum = dialogNum;
}


/*****************************************************************
                TreeDiagManager
******************************************************************/


TreeDiagManager::~TreeDiagManager()

   // Destructor
{
}

/*****************************************************************
                   ok
******************************************************************/


void TreeDiagManager::ok()

   // Called when OK button is pressed.
   // A derived function of PopupManager.
{
                                // Redraw the node in the colour, that
				// it used to have before the PopUp
				// Menue was opend and the node was
				// highlighted. If the colour of the node
				// should have changed in the mean time,
				// this causes no problems, since we get
				// this colour by the following call:
   int _colour = theTreeInterface->Tree()->get_under_colour(_node); 

   theTreeInterface->graph()->eraseNode(_node);
   theTreeInterface->graph()->drawNode(_colour,_node);
   theTreeInterface->Tree()->set_node_colour(_node,_colour);
   theTreeInterface->Tree()->display(_node,FALSE);
   theTreeInterface->Tree()->highlight(_node,FALSE);
   _isPosted = FALSE;
   theTreeInterface->decreaseOpenWin(_dialogNum);   
}



/*****************************************************************
                   post
******************************************************************/


void TreeDiagManager::post(char* message, char* shortinfo,int nb)

   // A derived function from PopupManager.
   // Posts the PopUpMenue with the nodes information on the screen,
   // highlights the node.
{

   _node = nb;

                               // Draw the node in a new color.
   theTreeInterface->graph()->eraseNode(_node);
   theTreeInterface->graph()->drawNode(STANDARDHIGHLIGHT,_node);
   theTreeInterface->Tree()->set_node_colour(_node,STANDARDHIGHLIGHT);
   theTreeInterface->Tree()->highlight(_node,TRUE);

   PopupManager::post();
   XtVaSetValues(_displayNumber, XmNvalue,shortinfo, NULL);
   XtVaSetValues(_displayInfo, XmNvalue, message, NULL);

   _isPosted = TRUE;

}




/*****************************************************************
                   changeWindow
******************************************************************/


void TreeDiagManager::changeWindow(char* message, char* shortinfo)

   // If the information of a node is posted, it might be necessary
   // to change the Information shown in the PopUpMenue.
   // This can be done in this procedure.
{
   XtVaSetValues(_displayNumber, XmNvalue,shortinfo, NULL);
   XtVaSetValues(_displayInfo, XmNvalue, message, NULL);
}




/*****************************************************************
                   createControlArea
******************************************************************/


void TreeDiagManager::createControlArea()

   // A derived function from PopupManager. When the PopUpMenue is
   // allocated, this procedure creates the form of the Window.
{

                                // Father Widget for _frameNumber
				// and _displayInfo
   _displayControl = XtVaCreateManagedWidget("displayControl",
                                        xmFormWidgetClass,
					_w,
				        NULL);

				// Widget, that creates a frame around
				// the important information.

   _frameNumber =  XtVaCreateManagedWidget("frameNumber",
                                       xmFrameWidgetClass,
				       _displayControl,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNresizeHeight, TRUE,
				       XmNresizeWidth, TRUE,
				       NULL);


                                // Widget for displaying important information
   _displayNumber = XtVaCreateManagedWidget("displayNumber",
                                       xmTextWidgetClass,
				       _frameNumber,
				       XmNshadowThickness, 0,
				       XmNhighlightThickness, 0,
				       XmNalignment, XmALIGNMENT_BEGINNING,
				       XmNeditable, FALSE,
				       XmNeditMode, XmMULTI_LINE_EDIT,
				       XmNwordWrap, FALSE,
				       XmNresizeHeight, TRUE,
				       XmNresizeWidth, TRUE,
				       XmNmarginHeight, 5,
				       XmNmarginWidth, 5,
				       XmNcursorPositionVisible, FALSE,
				       NULL);
				       
                                // Widget for displaying information.
   _displayInfo = XtVaCreateManagedWidget("displayInfo",
                                       xmTextWidgetClass,
				       _displayControl,
				       XmNtopAttachment, XmATTACH_WIDGET,
				       XmNtopWidget, _frameNumber,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_FORM,
				       XmNshadowThickness, 0,
				       XmNhighlightThickness, 0,
				       XmNalignment, XmALIGNMENT_BEGINNING,
				       XmNeditable, FALSE,
				       XmNeditMode, XmMULTI_LINE_EDIT,
				       XmNwordWrap, FALSE,
				       XmNcursorPositionVisible, FALSE,
				       XmNresizeHeight, TRUE,
				       XmNresizeWidth, TRUE,
				       XmNmarginHeight, 5,
				       XmNmarginWidth, 5,
				       NULL);


}




/*****************************************************************
                   createActionArea
******************************************************************/


void TreeDiagManager::createActionArea()

   // Creates the Action Area.
   // Unmanages the Cancel Button. So it won't be visible.
{

   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_CANCEL_BUTTON));

}
