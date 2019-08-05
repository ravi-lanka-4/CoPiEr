/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	GFEResourceManager.cc
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	methods for reading Resource-files in ./GFEResource/
// 
//
////////////////////////////////////////////////////////////////////////	

#include "GFEResourceManager.h"
#include "Application.h"
#include "GFEWindow.h"
#include "ClientInterface.h"

#include <Xm/Label.h> // for getting the path-component

GFEResourceManager::GFEResourceManager(char *sourceName)
      {
	 if (_prefix[0] == EOF)
	 {
	    strcpy(_prefix, theClientInterface->prefix());

	    // get the path of the Resource-Dir if it is specified

	    Widget dummy = XtVaCreateWidget("ResourceDirectoryPath",
					     xmLabelWidgetClass,
					     theApplication->baseWidget(),
					     NULL);

	    XmString str;
	    XtVaGetValues(dummy, XmNlabelString, &str, NULL);

	    char *c;
	    XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &c);
	    strcpy(_path, c);
	    kill(c);
	    XmStringFree(str);

	    XtDestroyWidget(dummy);

	    if(!strncmp(_path, "ResourceDirectoryPath", 21))
	    {
	       strcpy (_path, _prefix);
	       strcat (_path, GFERESOURCEDIR);
	       strcat (_path, "/");
	    }
	 }

	 _line = new char[256];
	 
	 _sourceName = strdup (sourceName);
	 
	 char name[500];

	 strcpy (name, _path);
	 strcat (name, _prefix);
	 strcat (name, sourceName);

	 _file = fopen(name, "r");
	 if (!_file)
	 {
	    fprintf(stderr, "NO RESOURCE-FILE CALLED %s\n", name);
	    _error = TRUE;
	 }
	 else _error = FALSE;
      }

GFEResourceManager::~GFEResourceManager()
      {
	 int error = fclose(_file);
	 if (error == EOF && _error == FALSE)
	    fprintf(stderr, "Can not close Source-File %s correctly\n", _sourceName);
	    
	 kill (_sourceName);
	 delete (_line);
      }

void GFEResourceManager::copy(FILE *target)
      {
	 if (!_error)
	 {
	    int c;
	    while((c = fgetc(_file)) != EOF)
	       fputc(c, target);
	 }
      }
			      
char *GFEResourceManager::nextline()
      {
	 if (!_error)
	 {
	    int i=0;
	    int c;
	    char cc;
	    while (TRUE)
	    {
	       while (((c = fgetc(_file)) != EOF) && c != '\n' && i<255)
		  _line[i++] = (char) c;
	       _line[i]=0;
	       if ((cc=_line[0]) != 0 && cc!=' ' && cc!='#' && cc!='\t') break;
	       if (c == EOF) return NULL;
	       i=0;
	    }
	    return _line;
	 }
	 else return NULL;
      }

char *GFEResourceManager::findline(char *searchString)
      {
	 if (!_error)
	 {
	    char *line;
	    while(line = nextline())
	    {
	       if (!strncmp(line, searchString, strlen(searchString)))
		  break;
	    }
	    if (line) return line;
	    else
	       return NULL;
	 }
	 return NULL;
      }

char *GFEResourceManager::getstring(char *searchString, char *defaultString)
{
   char *r = getstring(searchString);
   if (r) return r;
   else return defaultString;
}

char *GFEResourceManager::getstring(char *searchString)
      {
	 if (!_error)
	 {
	    char *line , *lp;
	    char ss[256];

	    rewind(_file);

	    strcpy(ss, searchString);
	    
	    strcat(ss, ":");
	    int sl = strlen(ss);
	    while(line = nextline())
	       if (!strncmp(line, ss, sl))
		  break;

	    if (!line) return NULL;

	    lp = line + sl;
	       
	    while (lp[0] && (lp[0]==' ' || lp[0]=='\t')) lp++;
	       
	    if (!lp[0])
	       return NULL;

	    return lp;
	 }
	 else return NULL;
      }


char *GFEResourceManager::nextRgbName(int &a, int &b, int &c)
{
   if (!_error)
   {
      if (fscanf(_file, "%d%d%d", &a, &b, &c)==EOF)
	 return NULL;

      int i=0;
      int cc;

      // read all tabs and spaces before the rgb-names and reposition the file-pointer
      while (((cc = fgetc(_file)) != EOF) && (cc == '\t' || cc == ' '));
      if (cc == EOF) return NULL;

#define SEEK_CUR 1
      fseek(_file, -1, SEEK_CUR);

      while (((cc = fgetc(_file)) != EOF) && cc != '\n' && i<99)
	 _line[i++] = (char) cc;
      _line[i]=0;

      if (cc==EOF) return NULL;
   
      
      return _line;
   }
   else return NULL;
}

void GFEResourceManager::rewindFile()
{
   if (!_error)
      rewind(_file);
}

char *GFEResourceManager::nextFontName()
{
   if (!_error)
   {
      while(True)
      {
	 // read next line
	 char *c = nextline();

	 if (!c)
	    return NULL;

	 char *help = c;
      
	 while(help[0])
	 {
	    if (help[0] == ':')
	    {
	       help[0] = 0;
	       return c;
	    }
	
	    if (!strncmp(help, ".ps", 3)) break;
	    help++;
	 }
      }
   }
   else return NULL;
}
