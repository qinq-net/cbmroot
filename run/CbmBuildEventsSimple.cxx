#include "CbmBuildEventsSimple.h"

#include "CbmTimeSlice.h"
#include "CbmDigi.h"

#include "CbmStsDigi.h"
#include "CbmStsAddress.h"

#include "FairRootManager.h"
#include "FairLogger.h"

#include "TClonesArray.h"

#include <iostream>
#include <list>

using namespace std;

CbmBuildEventsSimple::CbmBuildEventsSimple()
  : FairTask(), fSlice(NULL), fSTSDigi(NULL),
    fNSTSDigis(0), fEv(0), fSliceN(0), fNDigis(0),
    fSN(-1111), fST(-1111),
    fWindDur(2), fMinusDeltaT(2), fPlusDeltaT(5), fDeadT(10),
    fMinHitStations(8), fMinDigis(5000)
{
}

CbmBuildEventsSimple::CbmBuildEventsSimple(const char* name, Int_t iVerbose)
  : FairTask(name, iVerbose), fSlice(NULL), fSTSDigi(NULL),
    fNSTSDigis(0), fEv(0), fSliceN(0), fNDigis(0),
    fSN(-1111), fST(-1111),
    fWindDur(2), fMinusDeltaT(2), fPlusDeltaT(5), fDeadT(10),
    fMinHitStations(8), fMinDigis(5000)
{
}

void CbmBuildEventsSimple::Exec(Option_t*)
{
  Int_t i=0;
  Int_t j=0;
//  Int_t n=0;
  Int_t k;
  Int_t nsts=fSlice->GetDataSize(kSTS);
  Double_t t;
  CbmStsDigi* digi;
  CbmStsDigi* digi2;
  Int_t n0;
  Int_t n1;

  LOG(INFO) << "CbmBuildEventsSimple:	Sts digis in slice " << nsts << ". Slice start: " << fSlice->GetStartTime() << FairLogger::endl;
  fNDigis=0;
  for(i=0;i<16;i++) fNStsDigis[i]=0;
  fSN=0;
  fST=-1111;
  
  for(i=0;i<nsts;i++)
  {
    digi=(CbmStsDigi*)fSlice->GetData(kSTS, i);
    if (digi==NULL) continue;
    t=digi->GetTime();
    fNDigis++;
    k=CbmStsAddress::GetElementId(digi->GetAddress(), kStsStation);
    fNStsDigis[k]++;
    if (fST==-1111) { fSN=i; fST=t; }
    if (t-fST>fWindDur)
    {
      for(j=fSN;j<i;j++)
      {
        digi2=(CbmStsDigi*)fSlice->GetData(kSTS, j);
        if (digi2==NULL) continue;
	fST=digi2->GetTime();
        if (t-fST<=fWindDur) { fSN=j; break; }
	fNDigis--;
        k=CbmStsAddress::GetElementId(digi2->GetAddress(), kStsStation);
        fNStsDigis[k]--;
      }
      if (j==i) { fSN=i; fST=t; }
    } 

    if (fNDigis>=fMinDigis)
    {
      k=0;
      for(j=0;j<16;j++)
	if (fNStsDigis[j]>0) k++;
      if (k>=fMinHitStations)
      {
	// Reached required number of digis and hit stations
        for(j=i;j>-1;j--)
	{
	  digi2=(CbmStsDigi*)fSlice->GetData(kSTS, j);
          if (digi2==NULL) continue;
	  if (fST-digi2->GetTime()>fMinusDeltaT) break;
	}
	n0=j+1;
	for(j=i+1;j<nsts;j++)
	{
	  digi2=(CbmStsDigi*)fSlice->GetData(kSTS, j);
          if (digi2==NULL) continue;
	  if (digi2->GetTime()-t>fPlusDeltaT) break;
	}
	n1=j-1;
	FillEvent(n0, n1);
	if (j==nsts)
	{
	  //Reached the end of the slice
	  i=j; break;
	}
	for(j=n1+1;j<nsts;j++)
	{
	  digi2=(CbmStsDigi*)fSlice->GetData(kSTS, j);
          if (digi2==NULL) continue;
	  if (digi2->GetTime()-t>fPlusDeltaT+fDeadT) break;
	}
	if (j==nsts)
	{
	  //Reached the end of the slice
	  i=j; break;
	}
	i=j-1;
	for(j=0;j<16;j++) fNStsDigis[j]=0;
	fNDigis=0; fSN=i+1; 
        digi2=(CbmStsDigi*)fSlice->GetData(kSTS, i+1);
	fST=digi2->GetTime();
/*	
	cout << "--> ";
        digi2=(CbmStsDigi*)fSlice->GetData(kSTS, n0);
	cout << n0 << "(" << digi2->GetTime() << ")" << ", ";
        digi2=(CbmStsDigi*)fSlice->GetData(kSTS, n1);
	cout << n1 << "(" << digi2->GetTime() << ")" << ": ";
        digi2=(CbmStsDigi*)fSlice->GetData(kSTS, i);
	cout << i << "(" << digi2->GetTime() << ")" << endl;
*/	
      }

    }
//    cout << t << " " << fST << " " << fNDigis << " :";
//    for(k=0;k<8;k++) cout << " " << fNStsDigis[k];
//    cout <<  endl;
  }
  fSliceN++;
}

/** Fills Stsdigis array. STS separate, because start and end] end digi is known. **/
void CbmBuildEventsSimple::FillEvent(Int_t st, Int_t end)
{
  Int_t i;
  CbmStsDigi* digi;

  fNSTSDigis=0; fSTSDigi->Delete(); fEv++;
  for(i=st;i<=end;i++)
  {
    digi=(CbmStsDigi*)fSlice->GetData(kSTS, i);
    new ((*fSTSDigi)[fNSTSDigis])CbmStsDigi(*digi);
    fNSTSDigis++;
//    cout << fSTSDigi->GetEntries() << " " << fNSTSDigis << endl;
  }
  FairRootManager::Instance()->Fill();
  LOG(INFO) << "CbmBuildEventsSimple:	Event constructed. Digis used from " << st << " to " << end << "." << FairLogger::endl;
}

void CbmBuildEventsSimple::Finish()
{
  LOG(INFO) << "CbmBuildEventsSimple:	Events constructed: " << fEv << ". Slices processed: " << fSliceN << FairLogger::endl;
}

InitStatus CbmBuildEventsSimple::Init()
{
  FairRootManager* mgr=FairRootManager::Instance();

  if (mgr==NULL)
  {
    LOG(FATAL) << "Can't find root manager in the system." << FairLogger::endl;
    return kFATAL;
  }
  fSlice=(CbmTimeSlice*)mgr->GetObject("TimeSlice.");
  if (fSlice==NULL)
  {
    LOG(FATAL) << "No timeslice object in the system" << FairLogger::endl;
    return kFATAL;
  }

  fSTSDigi=new TClonesArray("CbmStsDigi", 10000);
  mgr->Register("StsDigi", "Digital response in STS" ,fSTSDigi, IsOutputBranchPersistent("StsDigi"));
  fSTSDigi->Delete(); fNSTSDigis=0;

  return kSUCCESS;
}

CbmBuildEventsSimple::~CbmBuildEventsSimple()
{
  ;
}

ClassImp(CbmBuildEventsSimple)

