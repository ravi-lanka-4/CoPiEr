/*
/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	GFE_C-Code.c
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C
//
//	Purpose	 :	used c-Functions
// 
//
////////////////////////////////////////////////////////////////////////	
*/
/*
 * scalefonts.c
 *
 * Written by David Flanagan.  Copyright 1991, O'Reilly && Associates.
 * This program is freely distributable without licensing fees and
 * is provided without guarantee or warranty expressed or implied.
 * This program is -not- in the public domain.
 *
 * This program demonstrates the use of scalable fonts in X11R5.
 * Invoke it with a fully specified scalable font name as its
 * only argument.  It will load that font scaled to 7 point,
 * 11 point, 15 point, and 19 point precisely at the resolution
 * of your screen.  It opens a very simple window and draws
 * the font name at each of those point sizes.
 *
 */



#include "GFE_C-Code.h"

/*
 * This routine returns True only if the passed name is a well-formed
 * XLFD style font name with a pixel size, point size, and average
 * width (fields 7,8, and 12) of "0".
 */ 
Bool IsScalableFont(name) 
char *name; 
{
    int i, field;
    
    if ((name == NULL) || (name[0] != '-')) return False;
    
    for(i = field = 0; name[i] != '\0' && field <= 14; i++) {
	if (name[i] == '-') {
	    field++;
	    if ((field == 7) || (field == 8) || (field == 12))
		if ((name[i+1] != '0') || (name[i+2] != '-'))
		    return False;
	}
    }
    
    if (field != 14) return False;
    else return True;
}


/*
 * This routine is passed a scalable font name and a point size.
 * It returns an XFontStruct for the given font scaled to the 
 * specified size and the exact resolution of the screen.
 * The font name is assumed to be a well-formed XLFD name,
 * and to have pixel size, point size, and average width fields
 * of "0" and implementation dependent x-resolution and y- 
 * resolution fields.  Size is specified in tenths of points.
 * Returns NULL if the name is malformed or no such font exists.
 */
XFontStruct *LoadQueryScalableFont(dpy, screen, name, size)
Display *dpy;
int screen;
char *name;
int size;
{
    int i,j, field;
    char newname[500];        /* big enough for a long font name */
    int res_x, res_y;         /* resolution values for this screen */
    
    /* catch obvious errors */
    if ((name == NULL) || (name[0] != '-')) return NULL;
    
    /* calculate our screen resolution in dots per inch. 25.4mm = 1 inch */
    res_x = DisplayWidth(dpy, screen)/(DisplayWidthMM(dpy, screen)/25.4);
    res_y = DisplayHeight(dpy, screen)/(DisplayHeightMM(dpy, screen)/25.4);
    
    /* copy the font name, changing the scalable fields as we do so */
    for(i = j = field = 0; name[i] != '\0' && field <= 14; i++) {
	newname[j++] = name[i];
	if (name[i] == '-') {
	    field++;
	    switch(field) {
	    case 7:  /* pixel size */
	    case 12: /* average width */
		/* change from "-0-" to "-*-" */
		newname[j] = '*'; 
		j++;
		if (name[i+1] != '\0') i++;
		break;
	    case 8:  /* point size */
		/* change from "-0-" to "-<size>-" */
		(void)sprintf(&newname[j], "%d", size);
		while (newname[j] != '\0') j++;
		if (name[i+1] != '\0') i++;
		break;
	    case 9:  /* x resolution */
	    case 10: /* y resolution */
		/* change from an unspecified resolution to res_x or res_y */
		(void)sprintf(&newname[j], "%d", (field == 9) ? res_x : res_y);
		while(newname[j] != '\0') j++;
		while((name[i+1] != '-') && (name[i+1] != '\0')) i++;
		break;
	    }
	}
    }
    newname[j] = '\0';
    
    /* if there aren't 14 hyphens, it isn't a well formed name */
    if (field != 14) return NULL;
    
    return XLoadQueryFont(dpy, newname);
}




/*  c - Functions for switching the Cursor */

/*  not probed yet

static Widget  *pointerToWidgetArray = NULL;
static Cursor  *pointerToCursorArray = NULL;
static int     numberOfSwitches = 0;

void SetCursor(Widget w, int cursorShape)
{
   Widget *newWidgetArray;
   Cursor *newCursorArray;
   int i;

   Display *dsp = XtDisplay(w);

   Cursor cursor = XCreateFontCursor(dsp, cursorShape);

   XDefineCursor(dsp, XtWindow(w), cursor);

   newWidgetArray = (Widget*) malloc((numberOfSwitches+1)*sizeof(Widget));
   newCursorArray = (Cursor*) malloc((numberOfSwitches+1)*sizeof(Cursor));

   for(i = 0; i <numberOfSwitches; i++)
   {
      newWidgetArray[i] = pointerToWidgetArray[i];
      newCursorArray[i] = pointerToCursorArray[i];
      if (w == pointerToWidgetArray[i]) break;
   }

   if (i == numberOfSwitches)
   {
      if (pointerToWidgetArray) free(pointerToWidgetArray);
      if (pointerToCursorArray) free(pointerToCursorArray);

      pointerToWidgetArray = newWidgetArray;
      pointerToCursorArray = newCursorArray;

      pointerToWidgetArray[i] = w;
      pointerToCursorArray[numberOfSwitches++] = cursor;
   }
   else
   {
      free (newWidgetArray);
      free (newCursorArray);

      XFreeCursor(XtDisplay(pointerToWidgetArray[i]), pointerToCursorArray[i]);

      pointerToWidgetArray[i] = w;
      pointerToCursorArray[i] = cursor;
   }
}

void ResetCursor(Widget w)
{
   Widget *newWidgetArray;
   Cursor *newCursorArray;
   int i, j;

   Display *dsp = XtDisplay(w);

   if(!numberOfSwitches) return;

   for(i = 0; i < numberOfSwitches; i++)
      if (pointerToWidgetArray[i] == w) break;

   if (i == numberOfSwitches) return;

   numberOfSwitches--;
   newWidgetArray = (Widget*) malloc((numberOfSwitches)*sizeof(Widget));
   newCursorArray = (Cursor*) malloc((numberOfSwitches)*sizeof(Cursor));

   j = 0;
   for(i = 0; i < numberOfSwitches; i++)
   {
      if (pointerToWidgetArray[i] == w)
      {
	 j = 1;
	 XFreeCursor(XtDisplay(pointerToWidgetArray[i]), pointerToCursorArray[i]);
      }

      newWidgetArray[i] = pointerToWidgetArray[i+j];
      newCursorArray[i] = pointerToCursorArray[i+j];
   }
}
*/      

Cursor SetCursor(Widget w, int cursorShape)
{
   Display *dsp = XtDisplay(w);

   Cursor cursor = XCreateFontCursor(dsp, cursorShape);
   XDefineCursor(dsp, XtWindow(w), cursor);

   return cursor;
}

void ResetCursor(Widget w, Cursor cursor)
{
   Display *dsp = XtDisplay(w);
   
   XDefineCursor(dsp, XtWindow(w), (Cursor) NULL);
   XFreeCursor(dsp, cursor);
}
