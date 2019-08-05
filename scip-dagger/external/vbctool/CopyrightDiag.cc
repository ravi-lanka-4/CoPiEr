/*****************************************************************

            Filename    :  CopyrightDiag.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/


#include "GFE.inc"
#include "PopupManager.h"
#include "CopyrightDiag.h"
#include "TreeInterface.h"
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/SelectioB.h>


/*****************************************************************
                CopyrightDiag
******************************************************************/


CopyrightDiag::CopyrightDiag(char* a,Widget d)
   : PopupManager(a,d)   // call base-class constructor

   // Constructor.
{
   _isPosted = FALSE;
}


/*****************************************************************
                CopyrightDiag
******************************************************************/


CopyrightDiag::~CopyrightDiag()

   // Destructor
{
}

/*****************************************************************
                   ok
******************************************************************/


void CopyrightDiag::ok()

   // Called when OK button is pressed.
   // A derived function of PopupManager.
{
}



/*****************************************************************
                   post
******************************************************************/


void CopyrightDiag::post()

   // A derived function from PopupManager.
   // Posts the PopUpMenue with the nodes information on the screen,
   // highlights the node.
{

   char _heading[INFOSIZE];
   strcpy(_heading,"VBCTOOL - a graphical Interface for ");
   strcat(_heading,"Visualization of Branch and Cut algorithms\n");
   strcat(_heading,"\t\t The Tree Interface Version 1.0.1\n\n");
   
   char _firstInfo[INFOSIZE];

   strcpy(_firstInfo,"The Tree Interface and the files included ");
   strcat(_firstInfo,"along with the program may by ");
   strcat(_firstInfo,"freely\ncopied and distributed ");
   strcat(_firstInfo,"under the GPL.\n");
   strcat(_firstInfo,"All users are kindly asked to clearly mark ");
   strcat(_firstInfo,"modified versions of the code in order to ");
   strcat(_firstInfo,"compatibility intact.");
   strcat(_firstInfo,"The authors have tried\ntheir best to produce ");
   strcat(_firstInfo,"correct and useful programs, in order to help ");
   strcat(_firstInfo,"promote\ncomputer science research, but no ");
   strcat(_firstInfo,"warranty of any kind should be assumed.\n");

   char _secInfo[INFOSIZE];
   strcpy(_secInfo,"The Motif Application Framework is copyrighted to Douglas A. Young. ");
   strcat(_secInfo,"The GFE was written by Martin Diehl and ");
   strcat(_secInfo,"Joachim Kupke, Universität zu Köln, ");
   strcat(_secInfo,"Institut für\nInformatik ");
   strcat(_secInfo,"and the Graph Interface is due to ");
   strcat(_secInfo,"Joachim Kupke.\nThe Tree Interface is ");
   strcat(_secInfo,"due to Sebastian Leipert, Universität zu Köln, ");
   strcat(_secInfo,"Institut\nfür Informatik.\n\n");
   strcat(_secInfo,"Köln, August 1996\n");
   

   char _adress[INFOSIZE];
   strcpy(_adress,"All updates of the Tree Interface are available at:\n");
   strcpy(_adress,"http://www.informatik.uni-koeln.de/ls_juenger/research/vbctool/\n\n");
   strcat(_adress,"If there are any questions or suggestions about "); 
   strcat(_adress,"the Tree Interface,\nplease contact us under the ");
   strcat(_adress,"following e-mail adress:\n");
   strcat(_adress,"vbctool@informatik.uni-koeln.de");
      
   PopupManager::post();
   XtVaSetValues(_copyrightHeading, XmNvalue,_heading, NULL);
   XtVaSetValues(_copyrightDisplayOne, XmNvalue,_firstInfo, NULL);
   XtVaSetValues(_copyrightDisplayTwo, XmNvalue, _secInfo, NULL);
   XtVaSetValues(_copyrightAdress, XmNvalue, _adress, NULL);

   _isPosted = TRUE;

}



/*****************************************************************
                   createControlArea
******************************************************************/


void CopyrightDiag::createControlArea()

   // A derived function from PopupManager. When the PopUpMenue is
   // allocated, this procedure creates the form of the Window.
{

                                // Father Widget for _frameNumber
				// and _displayInfo
   _copyrightControl = XtVaCreateManagedWidget("copyrightControl",
                                        xmFormWidgetClass,
					_w,
				        NULL);

   _copyrightHeading = XtVaCreateManagedWidget("copyrightHeading",
                                       xmTextWidgetClass,
				       _copyrightControl,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNshadowThickness, 0,
				       XmNhighlightThickness, 0,
				       XmNalignment, XmALIGNMENT_CENTER,
				       XmNeditable, FALSE,
				       XmNeditMode, XmMULTI_LINE_EDIT,
				       XmNwordWrap, FALSE,
				       XmNresizeHeight, TRUE,
				       XmNresizeWidth, TRUE,
				       XmNmarginHeight, 5,
				       XmNmarginWidth, 5,
				       XmNcursorPositionVisible, FALSE,
				       NULL);
   
   _copyrightDisplayOne = XtVaCreateManagedWidget("copyrightDisplayOne",
                                       xmTextWidgetClass,
				       _copyrightControl,
				       XmNtopAttachment, XmATTACH_WIDGET,
				       XmNtopWidget, _copyrightHeading,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
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
   _copyrightDisplayTwo = XtVaCreateManagedWidget("copyrightDisplayTwo",
                                       xmTextWidgetClass,
				       _copyrightControl,
				       XmNtopAttachment, XmATTACH_WIDGET,
				       XmNtopWidget, _copyrightDisplayOne,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
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


   _copyrightFrame =  XtVaCreateManagedWidget("copyrightFrame",
                                       xmFrameWidgetClass,
				       _copyrightControl,
				       XmNtopAttachment, XmATTACH_WIDGET,
				       XmNtopWidget, _copyrightDisplayTwo,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNbottomAttachment, XmATTACH_FORM,
				       XmNresizeHeight, TRUE,
				       XmNresizeWidth, TRUE,
				       NULL);


   _copyrightAdress = XtVaCreateManagedWidget("copyrightAdress",
                                       xmTextWidgetClass,
				       _copyrightFrame,
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


void CopyrightDiag::createActionArea()

   // Creates the Action Area.
   // Unmanages the Cancel Button. So it won't be visible.
{

   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON));

}
