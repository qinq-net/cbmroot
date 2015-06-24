#ifndef CBMTRDCLUSTERANALYSIS_H
#define CBMTRDCLUSTERANALYSIS_H

#include "FairTask.h"
#include "CbmHistManager.h"

#include "TClonesArray.h"

class CbmTrdClusterAnalysis : public FairTask
{
 public:

  CbmTrdClusterAnalysis();
 ~CbmTrdClusterAnalysis();
 virtual InitStatus Init();
 virtual InitStatus ReInit();
 virtual void Exec(Option_t* opt);
 virtual void SetParContainers();
 virtual void Finish();

 private:

 TClonesArray *fDigis;
 TClonesArray *fClusters;
 TClonesArray *fHits;

 CbmHistManager *fHM;

 void CreateHistograms();

 CbmTrdClusterAnalysis(const CbmTrdClusterAnalysis&);
 CbmTrdClusterAnalysis operator=(const CbmTrdClusterAnalysis&);

 ClassDef(CbmTrdClusterAnalysis,1);
};

#endif
