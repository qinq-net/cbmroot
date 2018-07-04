/** @file CbmTrdRecoQaTb.h
 ** @author Alex Bercuci <abercuci@niham.nipne.ro>
 ** @date 19.12.2017
 **/

#ifndef CBMTRDRECOQATB_H_
#define CBMTRDRECOQATB_H_

#include "FairTask.h"
#include "CbmTrdDigi.h"
#include "CbmTimeSlice.h"
#include "CbmHistManager.h"
#include "CbmMCDataArray.h"

#include <string>
#include <vector>

class TClonesArray;
//class CbmStsSetup;

class CbmTrdRecoQaTb : public FairTask
{
public:
  CbmTrdRecoQaTb();

  virtual ~CbmTrdRecoQaTb();

  virtual InitStatus Init();

  virtual void Exec(Option_t* opt);

  virtual void Finish();

  void SetOutputDir(const std::string& outputDir) { fOutputDir = outputDir; }

  void UseDaq(Bool_t daq) { fDaq = daq; }

  void CreateHistograms(const std::string& type);

  void CreateNofObjectsHistograms(const std::string& type);

  void CreateHitParametersHistograms(const std::string& type);

  void Create2dHistograms(const std::string& type);

  void ProcessDigisAndPoints(
		  const std::vector<CbmTrdDigi> digis,
		   CbmMCDataArray* points,
		  const std::string& type);
  void ProcessDigisAndPoints(
		  const TClonesArray* digis,
		  const CbmMCDataArray* points,
		  const std::string& type);
  void ProcessClusters(
		  const TClonesArray* clusters,
		  const TClonesArray* clusterMatches,
		  CbmMCDataArray* points,
		  const std::string& type);
  void ProcessHits(
		  const TClonesArray* hits,
		  const TClonesArray* hitMatches,
		  const std::string& type);

  void FillResidualAndPullHistograms(
		  CbmMCDataArray* points,
		  const TClonesArray* hits,
		  const TClonesArray* hitMatches,
		  const std::string& type);

private:
  void ReadDataBranches();

  CbmHistManager* fHM;
  std::string fOutputDir;

//  CbmStsSetup* fSetup;

  CbmTimeSlice* fTimeSlice;
  TClonesArray* fTrdDigis;
  TClonesArray* fTrdClusters;
  TClonesArray* fTrdHits;
  TClonesArray* fTrdClusterMatches;
  TClonesArray* fTrdHitMatches;
  CbmMCDataArray* fTrdPoints;
  std::vector<CbmStsDigi> fTrdDigiData;

  Bool_t fDaq;
  Int_t fMaxScale;

  std::vector<Int_t> fMCinCell;
  std::vector<Int_t> fHitsinCell;
  std::vector<Float_t> fEffinCell;

  CbmTrdRecoQaTb(const CbmTrdRecoQaTb&);
  CbmTrdRecoQaTb& operator=(const CbmTrdRecoQaTb&);

  ClassDef(CbmTrdRecoQaTb, 1);
};

#endif
