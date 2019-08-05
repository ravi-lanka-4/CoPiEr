//
//
//
#ifndef FONTINFORMATION_H
#define FONTINFORMATION_H

#include "GFE.inc"


class FontInformation
{
   private:
   
      char	       **_fontNames;
      int	       _numberOfFonts;

   protected:

   public:

      FontInformation();
      ~FontInformation();

      int numberOfFonts() { return _numberOfFonts; }
      char **fontNames()  { return _fontNames; }

      int fontNamePosition(char*);
};

#endif

