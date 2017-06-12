/** @file  CbmMCDataObject.cxx
 ** @author //Dr.Sys <mikhail.prokudin@cern.ch>
 **/


#include "CbmMCDataObject.h"

#include "TObject.h"
#include "TChain.h"

#include "FairRootManager.h"
#include "FairLogger.h"

#include <iostream>

using namespace std;


// --- Standard constructor
CbmMCDataObject::CbmMCDataObject(const char* branchname,
		                           const std::vector<std::list<TString> >& fileList)
  : fLegacy(0), fLegacyObject(NULL), fBranchName(branchname), fSize(-1111), fChains(), fTArr(), fN(), fArrays()
{
  list<TString>::const_iterator p;
  Int_t i;
  Int_t s=fileList.size();

  fSize=s;
  fChains.resize(s);
  fTArr.resize(s);
  fN.resize(s);

  for(i=0;i<s;i++)
  {
    fN[i]=0;
    fTArr[i]=NULL;
    fChains[i]=NULL;
    if (fileList[i].size()==0) continue;
    fChains[i]=new TChain("cbmsim");
    for(p=fileList[i].begin();p!=fileList[i].end();++p)
      fChains[i]->AddFile(*p);
    fChains[i]->SetBranchAddress(branchname, &(fTArr[i]));
    fN[i]=fChains[i]->GetEntries();
  }

  fArrays.resize(s);
  for(i=0;i<s;i++)
    fArrays[i].clear();
}

// --- Make TChain number chainNum2 friend of TChain number chainNum2
void CbmMCDataObject::AddFriend(Int_t chainNum1, Int_t chainNum2)
{
  if (fLegacy==1)
  {
    LOG(ERROR) << "AddFriend method should not be called in legacy mode" << endl;
    return;
  }
  if (chainNum1<0 || chainNum1>=static_cast<Int_t>(fChains.size()) || fChains[chainNum1]==NULL)
  {
    LOG(ERROR) << "chainNum1=" << chainNum1 << " is not a correct chain number." << endl;
    return;
  }
  if (chainNum2<0 || chainNum2>=static_cast<Int_t>(fChains.size()) || fChains[chainNum2]==NULL)
  {
    LOG(ERROR) << "chainNum2=" << chainNum2 << " is not a correct chain number." << endl;
    return;
  }
  fChains[chainNum1]->AddFriend(fChains[chainNum2]);
}

// --- Legacy constructor
CbmMCDataObject::CbmMCDataObject(const char* branchname)
  : fLegacy(1), fLegacyObject(NULL), fBranchName(branchname), fSize(-1111), fChains(), fTArr(), fN(), fArrays()
{
  FairRootManager* fManager=FairRootManager::Instance();
  if (!fManager)
  {
    LOG(FATAL) << "CbmMCDataObject():	Can't find a Root Manager." << endl;
    return;
  }
  fLegacyObject=(TObject*)fManager->GetObject(branchname);
  if (!fLegacyObject)
  {
    LOG(FATAL) << "CbmMCDataObject(): Can't find " << fBranchName << " in the system.";
    return;
  }
}

// --- Legacy Get
TObject* CbmMCDataObject::LegacyGet(Int_t fileNumber, Int_t eventNumber)
{
  if (fileNumber>=0||eventNumber>=0)
    LOG(DEBUG1) << "LegacyGet:	Trying to get object with fileNum=" << fileNumber
                << ", entryNum=" << eventNumber << " in legacy mode."
                << FairLogger::endl;
  
  return fLegacyObject;
}


// --- Get an object
TObject* CbmMCDataObject::Get(Int_t fileNumber, Int_t eventNumber)
{
  if (fLegacy==1) return LegacyGet(fileNumber, eventNumber);
  if (fileNumber<0||fileNumber>=fSize) return NULL;
  if (eventNumber<0||eventNumber>=fN[fileNumber]) return NULL;
  
  // --- Cached objects
  map<Int_t, TObject*> &arr=fArrays[fileNumber];

  // --- If the object for this event is already in the cache, use it.
  if (arr.find(eventNumber)!=arr.end()) return arr[eventNumber];

  // --- If not, copy the object from the chain into the cache
  TChain* ch=fChains[fileNumber];
  ch->GetEntry(eventNumber);
//  arr[eventNumber]=(TObject*)(fTArr[fileNumber]->Clone());
  arr[eventNumber]=fTArr[fileNumber]->Clone();

  return arr[eventNumber];
}

// --- At end of one event: clear the cache to free the memory
void CbmMCDataObject::FinishEvent()
{
  if (fLegacy==1) return;

  Int_t i;
  map<Int_t, TObject*>::const_iterator p;

  for(i=0;i<fSize;i++)
  {
    for(p=fArrays[i].begin();p!=fArrays[i].end();++p)
      delete (p->second);
    fArrays[i].clear();
  }
}


// --- Clean up
void CbmMCDataObject::Done()
{
  if (fLegacy==1) return;
  Int_t i;

  FinishEvent();
  for(i=0;i<fSize;i++)
    delete fChains[i];
}

ClassImp(CbmMCDataObject)
