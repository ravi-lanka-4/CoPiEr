/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	GFEResourceManager.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see .cc
// 
//
////////////////////////////////////////////////////////////////////////	

#ifndef GFERESOURCEMANAGER_H
#define GFERESOURCEMANAGER_H

#include "Filenames.h"
#include "GFE.inc"


class GFEResourceManager
{
   private:
      FILE	     *_file;
      char	     *_sourceName;
      char	     *_line;
      Bool	     _error;
      static char    _prefix[20];
      static char    _path[500];

   protected:

   public:

      GFEResourceManager(char*);
      ~GFEResourceManager();

      void copy(FILE*);
			      
      char *nextline();
      char *findline(char*);
      char *getstring(char*);
      char *getstring(char*, char*);
      char *nextRgbName(int &a, int &b, int &c);
      void rewindFile();
      char *nextFontName();

      char *prefix() { return _prefix; }
};

#endif
