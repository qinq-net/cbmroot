#include "CbmTrdParMod.h"
#include <FairLogger.h>

//___________________________________________________________________
CbmTrdParMod::CbmTrdParMod(const char* name, const char* title)
  : TNamed(name, title)
  ,fModuleId(0)
{

}

//___________________________________________________________________
CbmTrdParMod::~CbmTrdParMod()
{
  LOG(DEBUG) << GetName() << "::delete["<< GetTitle() <<"]"<< FairLogger::endl;
}

ClassImp(CbmTrdParMod)
