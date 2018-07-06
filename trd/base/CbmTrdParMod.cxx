#include "CbmTrdParMod.h"


//___________________________________________________________________
CbmTrdParMod::CbmTrdParMod(const char* name, const char* title)
  : TNamed(name, title)
  ,fModuleId(0)
{

}

//___________________________________________________________________
CbmTrdParMod::~CbmTrdParMod()
{
  printf("AB :: clear %s[%s]\n", GetName(), GetTitle());
}

ClassImp(CbmTrdParMod)
