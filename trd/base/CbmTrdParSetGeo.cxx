#include "CbmTrdParSetGeo.h"
#include "CbmTrdParModGeo.h"

#include <FairParamList.h>
#include <FairGeoNode.h>

#include <TObjArray.h>

//________________________________________________________________________________________
CbmTrdParSetGeo::CbmTrdParSetGeo(const char* name,const char* title,const char* context)
: CbmTrdParSet(name,title,context)
{
}

//________________________________________________________________________________________
CbmTrdParSetGeo::~CbmTrdParSetGeo(void) {
}

//________________________________________________________________________________________
void CbmTrdParSetGeo::putParams(FairParamList* l) 
{
  if (!l) return;
  TObjArray *snodes(NULL), *pnodes(NULL);
  //for()
  l->addObject("FairGeoNodes Sensitive List", snodes);
  l->addObject("FairGeoNodes Passive List", pnodes);
}

//________________________________________________________________________________________
Bool_t CbmTrdParSetGeo::getParams(FairParamList* l) 
{
  if (!l) return kFALSE;
  TObjArray *snodes(NULL), *pnodes(NULL);
  if (!l->fillObject("FairGeoNodes Sensitive List", snodes)) return kFALSE;
  if (!l->fillObject("FairGeoNodes Passive List", pnodes)) return kFALSE;
  
  //CbmTrdParModGeo *geo(NULL);
  for (Int_t i=0; i < snodes->GetEntriesFast(); i++){
//     fModPar[moduleId[i]] = new CbmTrdParModGeo(GetName());
//     fModPar

    ((FairGeoNode*)(*snodes)[i])->print();
  }
  return kTRUE;
}

ClassImp(CbmTrdParSetGeo)

