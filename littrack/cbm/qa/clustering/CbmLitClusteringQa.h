/**
 * \file CbmLitClusteringQa.h
 * \brief FairTask for clustering performance calculation.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 */

#ifndef CBMLITCLUSTERINGQA_H_
#define CBMLITCLUSTERINGQA_H_

#include "FairTask.h"
#include "../cbm/base/CbmLitDetectorSetup.h"
#include "CbmMuchGeoScheme.h"
#include "CbmTimeSlice.h"
#include "CbmMCEventList.h"
#include <string>
#include <vector>
using std::string;
using std::vector;
using std::map;
class CbmHistManager;
class CbmMCDataArray;

class CbmLitClusteringQa : public FairTask
{
public:
   /**
    * \brief Constructor.
    */
   CbmLitClusteringQa();

   /**
    * \brief Destructor.
    */
   virtual ~CbmLitClusteringQa();

   /**
     * \brief Derived from FairTask.
     */
    virtual InitStatus Init();

    /**
     * \brief Derived from FairTask.
     */
    virtual void Exec(
       Option_t* opt);

    /**
     * \brief Derived from FairTask.
     */
    virtual void Finish();

    /** Setters */
    void SetOutputDir(const string& outputDir) { fOutputDir = outputDir; }
    void SetMuchDigiFileName(const string& digiFileName) { fMuchDigiFileName = digiFileName; }

private:

    Int_t GetStationId(
          Int_t address,
          ECbmModuleId detId);

    /**
     * \brief Read data branches.
     */
    void ReadDataBranches();

    void InitMuchGeoScheme(const string& digiFileName);

    void ProcessPoints(
          Int_t iEvent,
          CbmMCDataArray* points,
          const string& detName,
          ECbmModuleId detId);

    void ProcessDigis(
          const TClonesArray* digis,
          const TClonesArray* digiMatches,
          const string& detName,
          ECbmModuleId detId);

    void ProcessClusters(
          const TClonesArray* clusters,
          const TClonesArray* clusterMatches,
          const string& detName,
          ECbmModuleId detId);

    void ProcessHits(
          const TClonesArray* hits,
          const TClonesArray* hitMatches,
          const string& detName,
          ECbmModuleId detId);

    /**
     *
     */
    void FillEventCounterHistograms(Int_t iEvent);

    /**
     *
     */
    void CreateHistograms();

    /**
     *
     */
    void CreateNofObjectsHistograms(
          ECbmModuleId detId,
          const string& detName);

    /**
     *
     */
    void CreateNofObjectsHistograms(
          ECbmModuleId detId,
          const string& detName,
          const string& parameter,
          const string& xTitle);

    void CreateClusterParametersHistograms(
          ECbmModuleId detId,
          const string& detName);

    void FillResidualAndPullHistograms(
          CbmMCDataArray* points,
          const TClonesArray* hits,
          const TClonesArray* hitMatches,
          const string& detName,
          ECbmModuleId detId);

    void FillHitEfficiencyHistograms(
          Int_t iEvent,
          CbmMCDataArray* points,
          const TClonesArray* hits,
          const TClonesArray* hitMatches,
          const string& detName,
          ECbmModuleId detId);

    void CreateHitEfficiencyHistograms(
          ECbmModuleId detId,
          const string& detName,
          const string& parameter,
          const string& xTitle,
          Int_t nofBins,
          Double_t minBin,
          Double_t maxBin);

    CbmHistManager* fHM; // Histogram manager
    string fOutputDir; // Output directory for results
    CbmLitDetectorSetup fDet; // For detector setup determination

    // Pointers to data arrays
    CbmMCDataArray* fMCTracks;

    CbmMCDataArray* fMvdPoints; // CbmMvdPoint array
    TClonesArray* fMvdDigis; // CbmMvdDigi array
    TClonesArray* fMvdClusters; // CbmMvdClusters array
    TClonesArray* fMvdHits; // CbmMvdHit array

    CbmMCDataArray* fStsPoints; // CbmStsPoint array
    TClonesArray* fStsDigis; // CbmStsDigi array
    TClonesArray* fStsClusters; // CbmStsCluster array
    TClonesArray* fStsHits; // CbmStsHit array
    TClonesArray* fStsDigiMatches; // CbmMatch array
    TClonesArray* fStsClusterMatches; // CbmMatch array
    TClonesArray* fStsHitMatches; // CbmMatch array

    CbmMCDataArray* fRichPoints; // CbmRichPoint array
    TClonesArray* fRichHits; // CbmRichHits array

    CbmMCDataArray* fMuchPoints; // CbmMuchPoint array
    TClonesArray* fMuchDigis; // CbmMuchDigi array
    TClonesArray* fMuchClusters; // CbmMuchCluster array
    TClonesArray* fMuchPixelHits; // CbmMuchPixelHits array
    TClonesArray* fMuchStrawHits; // CbmMuchStrawHits array
    TClonesArray* fMuchDigiMatches; // CbmMatch array
    TClonesArray* fMuchClusterMatches; // CbmMatch array
    TClonesArray* fMuchPixelHitMatches; // CbmMatch array
    TClonesArray* fMuchStrawHitMatches; // CbmMatch array

    CbmMCDataArray* fTrdPoints; // CbmTrdPoint array
    TClonesArray* fTrdDigis; // CbmTrdDigi array
    TClonesArray* fTrdClusters; // CbmTrdCluster array
    TClonesArray* fTrdHits; // CbmTrdHit array
    TClonesArray* fTrdDigiMatches; // CbmMatch array
    TClonesArray* fTrdClusterMatches; // CbmMatch array
    TClonesArray* fTrdHitMatches; // CbmMatches array

    CbmMCDataArray* fTofPoints; // CbmTofPoint array
    TClonesArray* fTofHits; // CbmTofHit array

    string fMuchDigiFileName;
    
    CbmTimeSlice* fTimeSlice;
    CbmMCEventList* fEventList;

    CbmLitClusteringQa(const CbmLitClusteringQa&);
    CbmLitClusteringQa& operator=(const CbmLitClusteringQa&);
    
    ClassDef(CbmLitClusteringQa, 1);
};

#endif /* CBMLITCLUSTERINGQA_H_ */
