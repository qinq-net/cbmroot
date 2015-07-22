#ifndef CBMTUTORIAL2DETGEOPAR_H
#define CBMTUTORIAL2DETGEOPAR_H

#include "FairParGenericSet.h"

class TObjArray;
class FairParamList;

class CbmTutorial2DetGeoPar       : public FairParGenericSet {
public:
 
  /** List of FairGeoNodes for sensitive  volumes */
  TObjArray      *fGeoSensNodes; 

  /** List of FairGeoNodes for sensitive  volumes */
  TObjArray      *fGeoPassNodes; 

  CbmTutorial2DetGeoPar(const char* name="CbmTutorial2DetGeoPar",
		       const char* title="CbmTutorial2Det Geometry Parameters",
		       const char* context="TestDefaultContext");
  ~CbmTutorial2DetGeoPar(void);
  void clear(void);
  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);
  TObjArray* GetGeoSensitiveNodes() {return fGeoSensNodes;}
  TObjArray* GetGeoPassiveNodes()   {return fGeoPassNodes;}

  ClassDef(CbmTutorial2DetGeoPar,1)
};

#endif /* CBMTUTORIALDETGEOPAR_H */
