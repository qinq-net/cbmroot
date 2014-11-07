// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmTSUnpackSpadic.h"

#include "CbmSpadicRawMessage.h"
#include "TimesliceReader.hpp"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iostream>

CbmTSUnpackSpadic::CbmTSUnpackSpadic()
  : CbmTSUnpack(),
    fSpadicRaw(new TClonesArray("CbmSpadicRawMessage", 10))
{
}

CbmTSUnpackSpadic::~CbmTSUnpackSpadic()
{
}

Bool_t CbmTSUnpackSpadic::Init()
{
  LOG(INFO) << "Initializing" << FairLogger::endl; 

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }
  ioman->Register("SpadicRawMessage", "spadic raw data", fSpadicRaw, kTRUE);

  return kTRUE;
}

Bool_t CbmTSUnpackSpadic::DoUnpack(const fles::Timeslice& ts, size_t component)
{
  LOG(INFO) << "Unpacking Spadic Data" << FairLogger::endl; 

  fSpadicRaw->Clear();

  spadic::TimesliceReader r;
  Int_t counter=0;
  
  r.add_component(ts, component);

  for (auto addr : r.sources()) {
    LOG(INFO) << "---- reader " << addr << " ----" << FairLogger::endl;
    while (auto mp = r.get_message(addr)) {
      print_message(*mp);
      Int_t link = ts.descriptor(component, 0).eq_id;
      Int_t address = addr;
      Int_t channel = mp->channel_id();
      Int_t epoch = -1;
      Int_t time = mp->timestamp();
      Int_t samples = mp->samples().size();
      Int_t* sample_values = new Int_t[samples];
      Int_t counter1=0;
      for (auto x : mp->samples()) {
        sample_values[counter1] = x;
        ++counter1;
      }
      new( (*fSpadicRaw)[counter] )
	CbmSpadicRawMessage(link, address, channel, epoch, time, samples, sample_values);
      ++counter;
      delete[] sample_values;
    }
  }
  return kTRUE;

}

void CbmTSUnpackSpadic::print_message(const spadic::Message& m)
{
  std::cout << "v: " << (m.is_valid() ? "o" : "x");
  std::cout << " / gid: " << static_cast<int>(m.group_id());
  std::cout << " / chid: " << static_cast<int>(m.channel_id());
  if ( m.is_hit() ) { 
    std::cout << " / ts: " << m.timestamp();
    std::cout << " / samples (" << m.samples().size() << "):";
    for (auto x : m.samples()) {
      std::cout << " " << x;
    }
    std::cout << std::endl;
  } else {
    if ( m.is_epoch_marker() ) { 
      std::cout << " This is an Epoch Marker" << std::endl; 
    } else if ( m.is_epoch_out_of_sync() ) { 
      std::cout << " This is an out of sync Epoch Marker" << std::endl; 
    } else {
      std::cout << " This is not known" << std::endl;
    }
  }
}

void CbmTSUnpackSpadic::Reset()
{
}

/*
void CbmTSUnpackSpadic::Register()
{
}
*/


ClassImp(CbmTSUnpackSpadic)
