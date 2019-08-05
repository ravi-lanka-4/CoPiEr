/*****************************************************************

            Filename    :  EmlSetupDiag.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
			   
******************************************************************/

#include "EmlSetupDiag.h"
#include "TreeInterface.h"
#include <sstream>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleBG.h>


/*****************************************************************
                    Constructor
******************************************************************/


EmlSetupDiag::EmlSetupDiag(char* name,Widget parent)
   :  PopupManager(name,parent)          // Call base-class constructor

   // Constructor.
{
   _isPosted = FALSE;
   _yesnoStep = NULL;
   _yesnoTree = NULL;

   _enthour = 0;
   _entmin = 0;
   _entsec = 0;
   _entms = 0;
}



/*****************************************************************
                    Destructor
******************************************************************/


EmlSetupDiag::~EmlSetupDiag()

   // Destructor
{

   delete _yesnoStep;
   delete _yesnoTree;

}




/*****************************************************************
                       cancel
******************************************************************/



void EmlSetupDiag::cancel()

   // This procedure is called when the Cancel button is pushed.
{
   _isPosted = FALSE;
}




/*****************************************************************
                         ok
******************************************************************/



void EmlSetupDiag::ok()

   // This procedure is called when the Ok button is pushed.
   // Does the same as the Apply button and closes the window.
{
   apply();
   _isPosted = FALSE;
}



/*****************************************************************
                        apply
******************************************************************/



void EmlSetupDiag::apply()

   // This procedure is called when the Apply button or Ok button
   // are pushed. It checks, if the user has made some changes
   // in the Setup Dialog window, and starts the approprite action
   // to obey the users wishes.
{
   char* value;
   char  input[4];
   int   hour = 0;
   int   min  = 0;
   int   sec  = 0;
   int   mill = 0;

                                       // First check the textwidget
				       // input field, if the user wants
				       // to run the emulation in a certain
				       // time.
   std::stringstream is;

   XtVaGetValues(_enterHour,XmNvalue,&value,NULL);
   strncpy(input,value,sizeof(input));
   XtFree(value);
   is << input << std::endl;
   is >> hour;
   
   XtVaGetValues(_enterMin,XmNvalue,&value,NULL);
   strncpy(input,value,sizeof(input));
   XtFree(value);
   is << input << std::endl;
   is >> min;

   XtVaGetValues(_enterSec,XmNvalue,&value,NULL);
   strncpy(input,value,sizeof(input));
   XtFree(value);
   is << input << std::endl;
   is >> sec;

   XtVaGetValues(_enterMilli,XmNvalue,&value,NULL);
   strncpy(input,value,sizeof(input));
   XtFree(value);
   is << input << std::endl;
   is >> mill;

                                       // If the values of the textwidget
				       // differ from the last known values,
				       // the user must have made some
				       // changes. Call a function of the
				       // class EmualtionCmd for a new
				       // calculation of the Scaler, which
				       // is multiplied to the time intervalls
				       // in order to run the emulation in
				       // the desired time.
   if (hour != _enthour || min != _entmin || sec != _entsec || mill != _entms)
   {
      theTreeInterface->emulationCmd()->interpretScaler(hour,min,sec,mill);
                                       // Remember the new values.
      _enthour = hour;
      _entmin = min;
      _entsec = sec;
      _entms = mill;
   }
 

                                       // Check if some buttons of the
				       // Radiofield have changed. We just
				       // get the actual value (which is
				       // either one or two) interpret them
				       // as TRUE or FALSE and then call
				       // functions of class EmulationCmd
				       // which decide if there are flags
				       // that have to be changed.
   if (_yesnoStep->getValue() == TRUE)
                                         // yes Button is pressed
      theTreeInterface->emulationCmd()->StepWise(TRUE);
   else
                                         // no Button is pressed
      theTreeInterface->emulationCmd()->StepWise(FALSE);
   
   if (_yesnoTree->getValue() == TRUE)
                                         // yes Button is pressed
      theTreeInterface->emulationCmd()->completeTree(TRUE);
   else
                                         // no Button is pressed
      theTreeInterface->emulationCmd()->completeTree(FALSE);
   
}




/*****************************************************************
                         post
******************************************************************/


void EmlSetupDiag::post()

   // The procedure is called when the button Setup in the menue bar
   // is pushed. It posts the Setup dialog window and fills
   // it with valid information.
{
   PopupManager::post();

   _isPosted = TRUE;

                                       // Get the maximum time that will
				       // be needed in total by the emulation.
   int maxhour  = theTreeInterface->emulationCmd()->maxHour();
   int maxmin   = theTreeInterface->emulationCmd()->maxMin();
   int maxsec   = theTreeInterface->emulationCmd()->maxSec();
   int maxmilli = theTreeInterface->emulationCmd()->maxMilli();

   int actualhour = 0;
   int actualmin = 0;
   int actualsec = 0;
   int actualmilli = 0;

                                       // Get the time that is needed
				       // to complete the emulation.
   theTreeInterface->emulationCmd()->getRemainingTime(&actualhour,
                                                      &actualmin,
						      &actualsec,
						      &actualmilli);

                                       // Save the values of the total time
				       // and the remaining time in strings.
   std::ostringstream maxtime;
   maxtime << maxhour << ":" << maxmin << ":" << maxsec << ":" << maxmilli << std::ends;

   std::ostringstream ac;
   ac << actualhour << ":" << actualmin << ":" << actualsec << ":"
      << actualmilli << std::ends;

                                       // Place the strings in the reserved
				       // text widgets.
   XtVaSetValues(_rightText1, XmNvalue, maxtime.str().c_str(), NULL);
   XtVaSetValues(_rightText2, XmNvalue, ac.str().c_str(), NULL);


                                       // Place the values for the time that
				       // the emulation should need in the
				       // reserved windows. Those values have
				       // been choosen by the user before.
				       // By default they are 0.
   //char ac1[SHORTDIM];
   std::ostringstream ac1;
   ac1 << _enthour << std::ends;
   XtVaSetValues(_enterHour, XmNvalue, ac1.str().c_str(), NULL);

   //char ac2[SHORTDIM];
   std::ostringstream ac2;
   ac2 << _entmin << std::ends;
   XtVaSetValues(_enterMin, XmNvalue, ac2.str().c_str(), NULL);

   //char ac3[SHORTDIM];
   std::ostringstream ac3;
   ac3 << _entsec << std::ends;
   XtVaSetValues(_enterSec, XmNvalue, ac3.str().c_str(), NULL);

   //char ac4[SHORTDIM];
   std::ostringstream ac4;
   ac4 << _entms << std::ends;
   XtVaSetValues(_enterMilli, XmNvalue, ac4.str().c_str(), NULL);
}






/*****************************************************************
                       newTime
******************************************************************/



void EmlSetupDiag::newTime()

   // The procedure is called to update the remaining time in the
   // "Time not edited" window. This is the "real"-time that the
   // emulation will need to get finished. It is called during the
   // emulation process by the class EmulationCmd.
{
   int actualhour = 0;
   int actualmin = 0;
   int actualsec = 0;
   int actualmilli = 0;

   theTreeInterface->emulationCmd()->getRemainingTime(&actualhour,
                                                      &actualmin,
						      &actualsec,
						      &actualmilli);

   std::ostringstream ac;
   ac << actualhour << ":" << actualmin << ":" << actualsec << ":"
      << actualmilli << std::ends;

   XtVaSetValues(_rightText2, XmNvalue, ac.str().c_str(), NULL);

}





/*****************************************************************
                   createControlArea
******************************************************************/



void EmlSetupDiag::createControlArea()

   // Creates the control area
{

   _displayControl = XtVaCreateManagedWidget("displayControl",
                                        xmFormWidgetClass,
					_w,
					NULL);

   createControlTimeArea();
   createControlRadioArea();


}





/*****************************************************************
                   createActionArea
******************************************************************/



void EmlSetupDiag::createActionArea()

   // Manages or unmanages some of the buttons of the window in the
   // action area.
{
   XtManageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w,XmDIALOG_HELP_BUTTON));

}






/*****************************************************************
                   createControlTimeArea
******************************************************************/



void EmlSetupDiag::createControlTimeArea()

   // Creates a part of the control Area.
{

   _timeControl = XtVaCreateManagedWidget("timeControl",
                                     xmFormWidgetClass,
				     _displayControl,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     NULL);

   Widget _timeRowControl = XtVaCreateManagedWidget("timeRowControl",
                                     xmRowColumnWidgetClass,
				     _timeControl,
				     XmNorientation, XmVERTICAL,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     NULL);



   Widget _infoColumn1 = XtVaCreateManagedWidget("infoColumn1",
                                     xmRowColumnWidgetClass,
				     _timeRowControl,
				     XmNorientation, XmHORIZONTAL,
				     NULL);


   Widget _infoColumn2 = XtVaCreateManagedWidget("infoColumn2",
                                     xmRowColumnWidgetClass,
				     _timeRowControl,
				     XmNorientation, XmHORIZONTAL,
				     NULL);


   Widget _left1 = XtVaCreateManagedWidget("Complete Time\t\t:",
                                     xmLabelWidgetClass,
				     _infoColumn1,
				     NULL);


   Widget _left2 = XtVaCreateManagedWidget("Time not edited\t:",
                                     xmLabelWidgetClass,
				     _infoColumn2,
				     NULL);


				     
   _rightText1 = XtVaCreateManagedWidget("right1",
                                     xmTextWidgetClass,
				     _infoColumn1,
				     XmNalignment, XmALIGNMENT_BEGINNING,
				     XmNeditable, FALSE,
				     XmNeditMode, XmSINGLE_LINE_EDIT,
				     XmNwordWrap, FALSE,
				     XmNcursorPositionVisible, FALSE,
				     XmNmarginHeight, 2,
				     XmNmarginWidth, 5,
				     XmNresizeHeight, FALSE,
				     XmNresizeWidth, FALSE,
				     XmNcolumns, 12,
				     NULL);
				     

   _rightText2 = XtVaCreateManagedWidget("right2",
                                     xmTextWidgetClass,
				     _infoColumn2,
				     XmNalignment, XmALIGNMENT_BEGINNING,
				     XmNeditable, FALSE,
				     XmNeditMode, XmSINGLE_LINE_EDIT,
				     XmNwordWrap, FALSE,
				     XmNcursorPositionVisible, FALSE,
				     XmNmarginHeight, 2,
				     XmNmarginWidth, 5,
				     XmNresizeHeight, FALSE,
				     XmNresizeWidth, FALSE,
				     XmNcolumns, 12,
				     NULL);
				     

   Widget _enterTimeFrame = XtVaCreateManagedWidget("enterTimeFrame",
                                     xmFrameWidgetClass,
				     _timeRowControl,
				     NULL);

   Widget _enterTimeOuterRow = XtVaCreateManagedWidget("enterTimeRow",
                                     xmRowColumnWidgetClass,
				     _enterTimeFrame,
				     XmNorientation, XmVERTICAL,
				     NULL);



   Widget _enterTimeLabel = XtVaCreateManagedWidget("Time to be used:",
                                     xmLabelWidgetClass,
				     _enterTimeOuterRow,
				     XmNalignment, XmALIGNMENT_CENTER,
				     NULL);



   Widget _enterTimeRow = XtVaCreateManagedWidget("enterTimeRow",
                                     xmRowColumnWidgetClass,
				     _enterTimeOuterRow,
				     XmNorientation, XmHORIZONTAL,
				     XmNpacking, XmPACK_COLUMN,
				     XmNnumColumns, 2,
				     XmNisAligned, TRUE,
				     XmNentryAlignment, XmALIGNMENT_END,
				     NULL);








   Widget _hours = XtVaCreateManagedWidget("Hours:",
                                     xmLabelWidgetClass,
				     _enterTimeRow,
				     NULL);
				     
   _enterHour = XtVaCreateManagedWidget("enterHour",
                                     xmTextWidgetClass,
				     _enterTimeRow,
				     XmNalignment, XmALIGNMENT_BEGINNING,
				     XmNeditable, TRUE,
				     XmNeditMode, XmMULTI_LINE_EDIT,
				     XmNwordWrap, FALSE,
				     XmNcursorPositionVisible, TRUE,
				     XmNresizeHeight, TRUE,
				     XmNresizeWidth, TRUE,
				     XmNmarginHeight, 2,
				     XmNmarginWidth, 5,
				     XmNresizeHeight, FALSE,
				     XmNresizeWidth, FALSE,
				     XmNcolumns, 4,
				     NULL);



   Widget _minutes = XtVaCreateManagedWidget("Minutes:",
                                     xmLabelWidgetClass,
				     _enterTimeRow,
				     NULL);
				     

   _enterMin = XtVaCreateManagedWidget("enterMin",
                                     xmTextWidgetClass,
				     _enterTimeRow,
				     XmNalignment, XmALIGNMENT_BEGINNING,
				     XmNeditable, TRUE,
				     XmNeditMode, XmMULTI_LINE_EDIT,
				     XmNwordWrap, FALSE,
				     XmNcursorPositionVisible, TRUE,
				     XmNresizeHeight, TRUE,
				     XmNresizeWidth, TRUE,
				     XmNmarginHeight, 2,
				     XmNmarginWidth, 5,
				     XmNresizeHeight, FALSE,
				     XmNresizeWidth, FALSE,
				     XmNcolumns, 4,
				     NULL);





   Widget _seconds = XtVaCreateManagedWidget("Seconds:",
                                     xmLabelWidgetClass,
				     _enterTimeRow,
				     NULL);
				     
   _enterSec = XtVaCreateManagedWidget("enterSec",
                                     xmTextWidgetClass,
				     _enterTimeRow,
				     XmNalignment, XmALIGNMENT_BEGINNING,
				     XmNeditable, TRUE,
				     XmNeditMode, XmMULTI_LINE_EDIT,
				     XmNwordWrap, FALSE,
				     XmNcursorPositionVisible, TRUE,
				     XmNresizeHeight, TRUE,
				     XmNresizeWidth, TRUE,
				     XmNmarginHeight, 2,
				     XmNmarginWidth, 5,
				     XmNresizeHeight, FALSE,
				     XmNresizeWidth, FALSE,
				     XmNcolumns, 4,
				     NULL);



   Widget _milliseconds = XtVaCreateManagedWidget("Millisec.:",
                                     xmLabelWidgetClass,
				     _enterTimeRow,
				     NULL);
				     
   
   _enterMilli = XtVaCreateManagedWidget("enterMilli",
                                     xmTextWidgetClass,
				     _enterTimeRow,
				     XmNalignment, XmALIGNMENT_BEGINNING,
				     XmNeditable, TRUE,
				     XmNeditMode, XmMULTI_LINE_EDIT,
				     XmNwordWrap, FALSE,
				     XmNcursorPositionVisible, TRUE,
				     XmNresizeHeight, TRUE,
				     XmNresizeWidth, TRUE,
				     XmNmarginHeight, 2,
				     XmNmarginWidth, 5,
				     XmNresizeHeight, FALSE,
				     XmNresizeWidth, FALSE,
				     XmNcolumns, 4,
				     NULL);



}






/*****************************************************************
                   createControlRadioArea
******************************************************************/



void EmlSetupDiag::createControlRadioArea()

   // Creates the part of the control area, where the radio buttons are
   // shown.
{


   _radioControl = XtVaCreateManagedWidget("radioControl",
                                     xmFormWidgetClass,
				     _displayControl,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, _timeControl,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     NULL);

   Widget _radioRowControl = XtVaCreateManagedWidget("radioRowControl",
                                     xmRowColumnWidgetClass,
				     _radioControl,
				     XmNorientation, XmVERTICAL,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     NULL);







   Widget _enterStepFrame = XtVaCreateManagedWidget("enterStepFrame",
                                     xmFrameWidgetClass,
				     _radioRowControl,
				     NULL);

   Widget _enterStepRow = XtVaCreateManagedWidget("enterStepRow",
                                     xmRowColumnWidgetClass,
				     _enterStepFrame,
				     XmNorientation, XmHORIZONTAL,
				     NULL);



   Widget _enterStepLabel = XtVaCreateManagedWidget("Single Step:",
                                     xmLabelWidgetClass,
				     _enterStepRow,
				     NULL);

    char* buttonLabels[] = {"yes","no"};
   _yesnoStep = new RadioField(_enterStepRow,
                               "yesnoStep", 2,
			       buttonLabels, 2,
			       XmHORIZONTAL, 1,
			       FALSE);
			       
			       

   Widget _enterTreeFrame = XtVaCreateManagedWidget("enterTreeFrame",
                                     xmFrameWidgetClass,
				     _radioRowControl,
				     NULL);

   Widget _enterTreeRow = XtVaCreateManagedWidget("enterTreeRow",
                                     xmRowColumnWidgetClass,
				     _enterTreeFrame,
				     XmNorientation, XmHORIZONTAL,
				     NULL);



   Widget _enterTreeLabel = XtVaCreateManagedWidget("Show complete Tree:",
                                     xmLabelWidgetClass,
				     _enterTreeRow,
				     NULL);

   _yesnoTree = new RadioField(_enterTreeRow,
                               "yesnoTree", 2,
			       buttonLabels, 2,
			       XmHORIZONTAL, 1,
			       FALSE);

			      			        
}




