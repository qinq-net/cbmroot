#include "CbmMCDataManager.h"

#include "CbmMCDataArray.h"

#include "FairRootManager.h"
#include "FairLogger.h"

#include <iostream>

using namespace std;

// --- Default constructor
CbmMCDataManager::CbmMCDataManager()
  : FairTask("MCDataManager", 0), fLegacy(0), fFileList(), fActive(), fFriends()
{
  fFileList.clear();
  fActive.clear();
}


// --- Standard constructor
CbmMCDataManager::CbmMCDataManager(const char* name, Int_t legacy)
  : FairTask(name,0), fLegacy(legacy), fFileList(), fActive(), fFriends()
{
  fFileList.clear();
  fActive.clear();
}


// --- Destructor
CbmMCDataManager::~CbmMCDataManager()
{
  map<TString, CbmMCDataArray*>::const_iterator p;
  for(p=fActive.begin();p!=fActive.end();++p) delete p->second;
  fActive.clear();
}


// --- Add an additional input file
Int_t CbmMCDataManager::AddFile(const char* name)
{
  Int_t n=fFileList.size();
  fFileList.resize(n+1);
  fFileList[n].clear();
  fFileList[n].push_back(name);
  return n;
}


// --- Add a file to a given input chain
Int_t CbmMCDataManager::AddFileToChain(const char* name, Int_t number)
{
  Int_t i;
  Int_t n=fFileList.size();

  if (number<0) return -1111;
  if (number>=n)
  {
    fFileList.resize(number+1);
    for(i=n;i<=number;i++) fFileList[i].clear();
  }
  fFileList[number].push_back(name);

  return fFileList[number].size();
}


// --- End of event action
void CbmMCDataManager::FinishEvent()
{
  map<TString, CbmMCDataArray*>::const_iterator p;

  for(p=fActive.begin();p!=fActive.end();++p)
    p->second->FinishEvent();
}


// --- Initialisation
InitStatus CbmMCDataManager::Init()
{
  FairRootManager* fManager=FairRootManager::Instance();
  if (!fManager) return kFATAL;

  fManager->Register("MCDataManager", "Stack", (TNamed*)this, kFALSE);

  return kSUCCESS;
}


// --- Instantiate a data branch
CbmMCDataArray* CbmMCDataManager::InitBranch(const char* brname)
{
  CbmMCDataArray* arr;
  TString nm=brname;
  map<Int_t, Int_t>::const_iterator p;

  if (fActive.find(nm)!=fActive.end())
  {
    LOG(INFO) << "InitBranch: " << nm << " " << fActive[nm] << FairLogger::endl;
    return fActive[nm];
  }
  if (fLegacy==0)
  {
    arr=new CbmMCDataArray(brname, fFileList);
    for(p=fFriends.begin();p!=fFriends.end();++p)
    {
      arr->AddFriend(p->first, p->second);
    }
  }
  else
  {
    FairRootManager* fManager=FairRootManager::Instance();
    if (!fManager)
    {
      LOG(FATAL) << "CbmMCDataManager::InitBranch(): Can't find a Root Manager." << FairLogger::endl;
      return NULL;
    }
    arr = NULL;
    // Create Data Array only if corresponding TClonesArray exists
    if( NULL != fManager->GetObject(brname) )
      arr=new CbmMCDataArray(brname);
  }

  if( NULL != arr )
  {
    fActive[nm]=arr;
    LOG(INFO) << "InitBranch: " << nm << " " << arr << FairLogger::endl;
  }
  else
  {
    LOG(INFO) << "InitBranch: " << nm << " could not be initialized, pointer is " << arr << FairLogger::endl;
  }

  return arr;
}



ClassImp(CbmMCDataManager)
