#include "CbmTutorial2DetGeoPar.h"

#include "FairParamList.h"

#include "TObjArray.h"

#include <iostream>

ClassImp(CbmTutorial2DetGeoPar)

CbmTutorial2DetGeoPar ::CbmTutorial2DetGeoPar(const char* name,
					    const char* title,
					    const char* context)
  : FairParGenericSet(name,title,context) {
  
  fGeoSensNodes = new TObjArray();
  fGeoPassNodes = new TObjArray();

}

CbmTutorial2DetGeoPar::~CbmTutorial2DetGeoPar(void) {
}

void CbmTutorial2DetGeoPar::clear(void) {
  if(fGeoSensNodes) delete fGeoSensNodes;
  if(fGeoPassNodes) delete fGeoPassNodes;
}

void CbmTutorial2DetGeoPar::putParams(FairParamList* l) {
  if (!l) return;
  l->addObject("FairGeoNodes Sensitive List", fGeoSensNodes);
  l->addObject("FairGeoNodes Passive List", fGeoPassNodes);
}

Bool_t CbmTutorial2DetGeoPar::getParams(FairParamList* l) {
  if (!l) return kFALSE;
  if (!l->fillObject("FairGeoNodes Sensitive List", fGeoSensNodes)) return kFALSE;
  if (!l->fillObject("FairGeoNodes Passive List", fGeoPassNodes)) return kFALSE;
  return kTRUE;
}
