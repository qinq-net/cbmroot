/** @file CbmStsTimeBasedQa.h
 ** @author Grigory Kozlov <g.kozlov@gsi.de>
 ** @date 19.04.2016
 **/

#ifndef CBMSTSTIMEBASEDQA_H_
#define CBMSTSTIMEBASEDQA_H_

#include "FairTask.h"
#include "CbmStsDigi.h"
#include "CbmTimeSlice.h"
#include "CbmHistManager.h"
#include "CbmMCDataArray.h"

class TClonesArray;

using std::string;
using std::vector;

class CbmStsTimeBasedQa : public FairTask
{
public:
  CbmStsTimeBasedQa();

  virtual ~CbmStsTimeBasedQa();

  virtual InitStatus Init();

  virtual void Exec(Option_t* opt);

  virtual void Finish();

  void SetOutputDir(const string& outputDir) { fOutputDir = outputDir; }

  void UseDaq(Bool_t daq) { fDaq = daq; }

  void CreateHistograms(const string& type);

  void CreateNofObjectsHistograms(const string& type);

  void CreateHitParametersHistograms(const string& type);

  void Create2dHistograms(const string& type);

  void ProcessDigisAndPoints(
		  const vector<CbmStsDigi> digis,
		   CbmMCDataArray* points,
		  const string& type);
  void ProcessDigisAndPoints(
		  const TClonesArray* digis,
		  const CbmMCDataArray* points,
		  const string& type);
  void ProcessClusters(
		  const TClonesArray* clusters,
		  const TClonesArray* clusterMatches,
		  CbmMCDataArray* points,
		  const string& type);
  void ProcessHits(
		  const TClonesArray* hits,
		  const TClonesArray* hitMatches,
		  const string& type);

  void FillResidualAndPullHistograms(
		  CbmMCDataArray* points,
		  const TClonesArray* hits,
		  const TClonesArray* hitMatches,
		  const string& type);

private:
  void ReadDataBranches();

  CbmHistManager* fHM;
  string fOutputDir;

  CbmTimeSlice* fTimeSlice;
  TClonesArray* fStsDigis;
  TClonesArray* fStsClusters;
  TClonesArray* fStsHits;
  TClonesArray* fStsClusterMatches;
  TClonesArray* fStsHitMatches;
  CbmMCDataArray* fStsPoints;
  vector<CbmStsDigi> fStsDigiData;

  Bool_t fDaq;
  Int_t fMaxScale;

  vector<Int_t> fMCinCell;
  vector<Int_t> fHitsinCell;
  vector<Float_t> fEffinCell;

  CbmStsTimeBasedQa(const CbmStsTimeBasedQa&);
  CbmStsTimeBasedQa& operator=(const CbmStsTimeBasedQa&);

  ClassDef(CbmStsTimeBasedQa, 1);
};

#endif
