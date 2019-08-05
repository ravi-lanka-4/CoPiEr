
#include "FontInformation.h"
#include "GFEResourceManager.h"


FontInformation::FontInformation()
{
   GFEResourceManager	fontFile(FONTFILENAME);

   char *c;
   int i = 0;

   while (c=fontFile.nextFontName())
      i++;

   _numberOfFonts = i;
   fontFile.rewindFile();
   i = 0;
   _fontNames = new char*[_numberOfFonts];

   while (c=fontFile.nextFontName())
   {
      if (i == _numberOfFonts) break;
	       
      _fontNames[i++] = strdup(c);

   }
}

FontInformation::~FontInformation()
{
   for (int j = 0; j<_numberOfFonts; j++)
      delete _fontNames[j];

   delete _fontNames;
}


int FontInformation::fontNamePosition(char *name)
{
   int i = 0;
   for (; i<_numberOfFonts; i++)
      if (!strcmp(_fontNames[i], name)) break;

   return ++i;
}
