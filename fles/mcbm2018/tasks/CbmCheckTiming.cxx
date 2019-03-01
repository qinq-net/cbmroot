/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *              GNU Lesser General Public Licence (LGPL) version 3,             *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#include "CbmCheckTiming.h"

#include "CbmTofDigiExp.h"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"
#include "TH1.h"

#include <iomanip> 
using std::fixed;
using std::setprecision;

// ---- Default constructor -------------------------------------------
CbmCheckTiming::CbmCheckTiming()
  : FairTask("CbmCheckTiming")
  , fT0Digis{nullptr}
  , fStsDigis{nullptr}
  , fMuchDigis{nullptr}
  , fTofDigis{nullptr}

{

}

// ---- Destructor ----------------------------------------------------
CbmCheckTiming::~CbmCheckTiming()
{

}

// ----  Initialisation  ----------------------------------------------
void CbmCheckTiming::SetParContainers()
{
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmCheckTimingDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmCheckTiming::Init()
{

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level
  
  fT0Digis = static_cast<TClonesArray*>(ioman->GetObject("CbmT0Digi"));
  if ( ! fT0Digis ) {
    LOG(info) << "No TClonesArray with T0 digis found.";
  }
  
  fStsDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmStsDigi"));
  if ( ! fStsDigis ) {
    LOG(info) << "No TClonesArray with STS digis found.";
  }
  
  fMuchDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmMuchBeamTimeDigi"));
  if ( ! fMuchDigis ) {
    LOG(info) << "No TClonesArray with MUCH digis found.";
  }
  
  fTofDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmTofDigi"));
  if ( ! fTofDigis ) {
    LOG(info) << "No TClonesArray with TOF digis found.";
  }

  if (fCheckInterSystemOffset) CreateHistos();
  
  return kSUCCESS;
  
}

void CbmCheckTiming::CreateHistos()
{
  if (fOffsetRange < 1001) {
    fBinWidth = 1;
  } else if (fOffsetRange < 10001) {
    fBinWidth = 10;
  } else if (fOffsetRange < 100001) {
    fBinWidth = 100;
  } else {
    fBinWidth = 100;
  }
  
  Int_t nrOfBins = fOffsetRange/fBinWidth *2;
  // T0 vs. Sts
  fT0StsDiff = new TH1F("fT0StsDiff","T0-Sts;time diff [ns];Counts",
			nrOfBins, -fOffsetRange, fOffsetRange);
  // T0 vs. Much
  fT0MuchDiff = new TH1F("fT0MuchDiff","T0-Much;time diff [ns];Counts",
			 nrOfBins, -fOffsetRange, fOffsetRange);
  // To vs. Tof
  fT0TofDiff = new TH1F("fT0TofDiff","T0-Tof;time diff [ns];Counts",
			nrOfBins, -fOffsetRange, fOffsetRange);

  // T0 vs. T0
  fT0T0Diff = new TH1F("fT0T0Diff","T0-T0_prev;time diff [ns];Counts",
		       2100, -100.5, 1999.5);
  // sts vs. Sts
  fStsStsDiff = new TH1F("fStsStsDiff","Sts-Sts_prev;time diff [ns];Counts",
		       2100, -100.5, 1999.5);
  // Much vs. Much
  fMuchMuchDiff = new TH1F("fMuchMuchDiff","Much-Much_prev;time diff [ns];Counts",
		       2100, -100.5, 1999.5);
  // Tof vs. Tof
  fTofTofDiff = new TH1F("fTofTofDiff","Tof-Tof_prev;time diff [ns];Counts",
		       2100, -100.5, 1999.5);
}  
// ---- ReInit  -------------------------------------------------------
InitStatus CbmCheckTiming::ReInit()
{
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmCheckTiming::Exec(Option_t* /*option*/)
{

  fNrTs++;
  LOG(info) << "executing TS " << fNrTs;
  
  if (fCheckTimeOrdering) CheckTimeOrder();
  if (fCheckInterSystemOffset) CheckInterSystemOffset();

}

void CbmCheckTiming::CheckInterSystemOffset() 
{
  LOG(info) <<"Begin";
  Int_t nrT0Digis=fT0Digis->GetEntriesFast();
  Int_t nrStsDigis=fStsDigis->GetEntriesFast();
  Int_t nrMuchDigis=fMuchDigis->GetEntriesFast();
  Int_t nrTofDigis=fTofDigis->GetEntriesFast();

  LOG(info) << "T0Digis: " << nrT0Digis;
  LOG(info) << "StsDigis: " << nrStsDigis;
  LOG(info) << "MuchDigis: " << nrMuchDigis;
  LOG(info) << "TofDigis: " << nrTofDigis;

  if (nrT0Digis < 100000) { 
  
    for (Int_t iT0 = 0; iT0 < nrT0Digis; ++iT0) {

      if (iT0%1000 == 0) LOG(info) << "Executing entry " << iT0;

      CbmDigi* T0Digi = static_cast<CbmDigi*>(fT0Digis->At(iT0));
      
      Double_t T0Time = T0Digi->GetTime();  
      
      if (nrStsDigis < 300000) FillSystemOffsetHistos(fStsDigis, fT0StsDiff, T0Time);
      if (nrMuchDigis < 300000) FillSystemOffsetHistos(fMuchDigis, fT0MuchDiff, T0Time);
      if (nrTofDigis < 300000) FillSystemOffsetHistos(fTofDigis, fT0TofDiff, T0Time);
    }
  }
}

void CbmCheckTiming::FillSystemOffsetHistos(TClonesArray* array,
						 TH1* histo,
						 const Double_t T0Time)
{
  Int_t nrDigis=array->GetEntriesFast();

  for (Int_t i = 0; i < nrDigis; ++i) {
							 
    CbmDigi* Digi = static_cast<CbmDigi*>(array->At(i));
    Double_t diffTime = T0Time - Digi->GetTime();

    if (diffTime > fOffsetRange) continue; // not yes in interesting range
    if (diffTime < -fOffsetRange) break;     // already past interesting range
    histo->Fill(diffTime);
  }  
}

void CbmCheckTiming::CheckTimeOrder() 
{
  if (fT0Digis) {
    Int_t nrT0Digis = fT0Digis->GetEntriesFast();
    fNrOfT0Digis += nrT0Digis; 
    fNrOfT0Errors += CheckIfSorted(fT0Digis, fT0T0Diff, fPrevTimeT0, "T0");
  }
  if (fStsDigis) {
    Int_t nrStsDigis = fStsDigis->GetEntriesFast();
    fNrOfStsDigis += nrStsDigis; 
    fNrOfStsErrors += CheckIfSorted(fStsDigis, fStsStsDiff, fPrevTimeSts, "Sts");
  }
  if (fMuchDigis) {
    Int_t nrMuchDigis = fMuchDigis->GetEntriesFast();
    fNrOfMuchDigis += nrMuchDigis; 
    fNrOfMuchErrors += CheckIfSorted(fMuchDigis, fMuchMuchDiff, fPrevTimeMuch, "Much");
  }
  if (fTofDigis) {
    Int_t nrTofDigis = fTofDigis->GetEntriesFast();
    fNrOfTofDigis += nrTofDigis; 
    fNrOfTofErrors += CheckIfSorted(fTofDigis, fTofTofDiff, fPrevTimeTof, "Tof");
  }
}

Int_t CbmCheckTiming::CheckIfSorted(TClonesArray* array, TH1* histo, Double_t& prevTime, TString detector) 
{
  Int_t nrOfErrors=0;
  Int_t nrDigis=array->GetEntriesFast();

  for (Int_t i = 0; i < nrDigis; ++i) {
 
     CbmDigi* digi = static_cast<CbmDigi*>(array->At(i));

     Double_t diffTime = digi->GetTime() - prevTime;
     histo->Fill(diffTime);

     if (diffTime < 0.) {
       LOG(info)  << fixed << setprecision(15) << diffTime << "ns";
       LOG(info) << "Previous " << detector << " digi (" << fixed << setprecision(15) 
                  << prevTime * 1.e-9 
                  << ") has a larger time than the current one ("
                  << digi->GetTime() * 1.e-9 << ") for digi " 
                  << i << " of ts " << fNrTs; 
       nrOfErrors++;
     }
     
     prevTime = digi->GetTime();

  } 

  return nrOfErrors;
}

// ---- Finish --------------------------------------------------------
void CbmCheckTiming::Finish()
{
  if (fCheckTimeOrdering) {
    LOG(info) << "Total number of T0 out of order digis: " << fNrOfT0Errors;
    LOG(info) << "Total number of T0 digis: " << fNrOfT0Digis;
    LOG(info) << "Total number of Sts out of order digis: " << fNrOfStsErrors;
    LOG(info) << "Total number of Sts digis: " << fNrOfStsDigis;
    LOG(info) << "Total number of Much out of order digis: " << fNrOfMuchErrors;
    LOG(info) << "Total number of Much digis: " << fNrOfMuchDigis;
    LOG(info) << "Total number of Tof out of order digis: " << fNrOfTofErrors;
    LOG(info) << "Total number of Tof digis: " << fNrOfTofDigis;
  }
  WriteHistos();

}

void CbmCheckTiming::WriteHistos()
{
  TFile* old = gFile;
  TFile* outfile = TFile::Open(fOutFileName,"RECREATE");
  
  fT0StsDiff->Write();
  fT0MuchDiff->Write();
  fT0TofDiff->Write();

  fT0T0Diff->Write();  
  fStsStsDiff->Write();
  fMuchMuchDiff->Write();
  fTofTofDiff->Write();

  outfile->Close();
  delete outfile;

  gFile = old;
}

ClassImp(CbmCheckTiming)