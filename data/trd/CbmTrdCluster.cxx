/**
 * \file CbmTrdCluster.cxx
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 */
#include "CbmTrdCluster.h"
#include <FairLogger.h>
#include <TMath.h>
#include <sstream>
#include <iostream>

//____________________________________________________________________
CbmTrdCluster::CbmTrdCluster()
  : CbmCluster()
  ,fNCols(0)
  ,fNRows(0xff)
  ,fStartCh(0xffff)
  ,fStartTime(0xffff)
{
}

//____________________________________________________________________
CbmTrdCluster::CbmTrdCluster(const CbmTrdCluster &ref)
  : CbmCluster(ref.GetDigis(), ref.GetAddress())
  ,fNCols(ref.fNCols)
  ,fNRows(ref.fNRows)
  ,fStartCh(ref.fStartCh)
  ,fStartTime(ref.fStartTime)
{
}

//____________________________________________________________________
CbmTrdCluster::CbmTrdCluster(const std::vector<Int_t>& indices, Int_t address)
  : CbmCluster(indices,address)
  ,fNCols(0)
  ,fNRows(0xff)
  ,fStartCh(0xffff)
  ,fStartTime(0xffff)
{
}

//____________________________________________________________________
CbmTrdCluster::CbmTrdCluster(Int_t address, Int_t idx, Int_t ch, Int_t row, Int_t time)
  : CbmCluster()
  ,fNCols(0)
  ,fNRows(0xff)
  ,fStartCh(0xffff)
  ,fStartTime(0xffff)
{
  ReInit(address, row, time);
  AddDigi(idx, ch);
}

//____________________________________________________________________
CbmTrdCluster::~CbmTrdCluster(){
}

//____________________________________________________________________
void CbmTrdCluster::AddDigi(Int_t idx, Int_t channel, Bool_t terminator)
{
/** Extend basic functionality of CbmCluster::AddDigi(). 
 * If channel>=0 add this info to channel map.
 */

  CbmCluster::AddDigi(idx);
  if(channel<0) return;

  if(channel>=0xffff)
    LOG(WARNING) << GetName() << "::ReInit: pad-channel truncated to 2bytes." << FairLogger::endl;
  
  if(!fNCols || channel < fStartCh) fStartCh = channel;
  fNCols++;
  return;
}

//____________________________________________________________________
void CbmTrdCluster::Clear()
{
  CbmCluster::ClearDigis();
  fNCols = 0;
  fNRows = 0xff;
  fStartCh = 0xffff;
  fStartTime = 0xffff;
}

//____________________________________________________________________
void CbmTrdCluster::ReInit(Int_t address, Int_t row, Int_t time)
{
  SetAddress(address);
  fNCols = 0;
  fStartCh = 0xffff;
  // check truncation
  if(row>=0xff)
    LOG(WARNING) << GetName() << "::ReInit: pad-row truncated to 1byte." << FairLogger::endl;
  fNRows = row; 
  if(time>=0xffff)
    LOG(WARNING) << GetName() << "::ReInit: buffer time truncated to 2bytes." << FairLogger::endl;
  fStartTime = time; 
}

//____________________________________________________________________
Int_t CbmTrdCluster::IsChannelInRange(Int_t ch) const
{
  if(!fNCols) return -2;
//   if(IsTerminatedLeft() && fAddressCh[0]>ch) return -1;      
//   if(IsTerminatedRight() && fAddressCh[clSize-1]<ch) return 1;

  if(fStartCh>ch+1) return -1;      
  if(fStartCh+fNCols<ch) return 1;
  return 0;      
}

//____________________________________________________________________
Bool_t CbmTrdCluster::Merge(CbmTrdCluster *second)
{

  if(fNRows != second->fNRows) return kFALSE;
  if(TMath::Abs(second->fStartTime - fStartTime)>=10) return kFALSE; 
  // look before and after  
  if(second->fStartCh+second->fNCols == fStartCh){
    cout<<"Merge before with "<<second->ToString()<<endl;
    fStartCh = second->fStartCh;
    fNCols  += second->fNCols;
    fStartTime = TMath::Min(fStartTime, second->fStartTime);
    AddDigis(second->GetDigis());
    return kTRUE;
  }  
  if(fStartCh+fNCols == second->fStartCh) {
    cout<<"Merge after with "<<second->ToString()<<endl;
    fNCols  += second->fNCols;
    fStartTime = TMath::Min(fStartTime, second->fStartTime);
    AddDigis(second->GetDigis());
    return kTRUE;
  } 
  return kFALSE;
}

//____________________________________________________________________
string CbmTrdCluster::ToString() const
{
  //CbmCluster::ToString();
  stringstream ss;
  ss<<"Time "<<fStartTime<<" Chs: ";
  for (Int_t i(0); i < fNCols; i++) ss << fStartCh+i << " ";
  return ss.str();
}

ClassImp(CbmTrdCluster)
