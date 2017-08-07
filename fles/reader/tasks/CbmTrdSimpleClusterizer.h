#ifndef CBMTRDSIMPLECLUSTERIZER_H
#define CBMTRDSIMPLECLUSTERIZER_H


#include "CbmTrdQABase.h"

class CbmTrdSimpleClusterizer : public CbmTrdQABase
{
 protected:
  TClonesArray* fDigis;
  TClonesArray* fClusters;

 public:
  CbmTrdSimpleClusterizer();
  CbmTrdSimpleClusterizer(const CbmTrdSimpleClusterizer&) = delete;
  CbmTrdSimpleClusterizer& operator=(const CbmTrdSimpleClusterizer&) = delete;
  //  ~CbmTrdSimpleClusterizer();
  InitStatus Init();
  void Exec(Option_t*);
  void CreateHistograms();
    
ClassDef(CbmTrdSimpleClusterizer,1);
};

#endif

