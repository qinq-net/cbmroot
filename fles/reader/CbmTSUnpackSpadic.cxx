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
    fSpadicRaw(new TClonesArray("CbmSpadicRawMessage", 10)),
    fEpochMarkerArray(),
    fSuperEpochCounter()
{
  for (Int_t i=0; i < NrOfSyscores; ++i) { 
    for (Int_t j=0; j < NrOfHalfSpadics; ++j) { 
      fEpochMarkerArray[i][j] = 0;
      fSuperEpochCounter[i][j] = 0;
    }
  }
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
  LOG(DEBUG) << "Unpacking Spadic Data" << FairLogger::endl; 

  fSpadicRaw->Clear();

  spadic::TimesliceReader r;
  Int_t counter=0;
  
  r.add_component(ts, component);

  for (auto addr : r.sources()) {
    LOG(DEBUG) << "---- reader " << addr << " ----" << FairLogger::endl;
    while (auto mp = r.get_message(addr)) {
      if(gLogger->IsLogNeeded(DEBUG)) {
	print_message(*mp);
      }

      Int_t link = ts.descriptor(component, 0).eq_id;
      Int_t address = addr;

      if ( mp->is_epoch_marker() ) { 
	switch (link) {
	case kMuenster:  // Muenster
          if ( mp->epoch_count() < fEpochMarkerArray[0][addr] ) {
            fSuperEpochCounter[0][addr]++;
	    LOG(INFO) << "Overflow of EpochCounter for Syscore0_Spadic" 
		      << addr << FairLogger::endl;
	  } else if ((mp->epoch_count() - fEpochMarkerArray[0][addr]) !=1 ) {
	    LOG(INFO) << "Missed epoch counter for Syscore0_Spadic" 
		      << addr << FairLogger::endl;
	  }
	  fEpochMarkerArray[0][addr] = mp->epoch_count(); 
	  break;
	case kFrankfurt: // Frankfurt
          if ( mp->epoch_count() < fEpochMarkerArray[1][addr] ) {
            fSuperEpochCounter[1][addr]++;
	    LOG(INFO) << "Overflow of EpochCounter for Syscore0_Spadic" 
		      << addr << FairLogger::endl;
	  }  else if ((mp->epoch_count() - fEpochMarkerArray[0][addr]) !=1 ) {
	    LOG(INFO) << "Missed epoch counter for Syscore0_Spadic" 
		      << addr << FairLogger::endl;
	  }
	  fEpochMarkerArray[1][addr] = mp->epoch_count(); 
	  break;
	case kBucarest: // Bucarest
          if ( mp->epoch_count() < fEpochMarkerArray[2][addr] ) {
            fSuperEpochCounter[2][addr]++;
	    LOG(INFO) << "Overflow of EpochCounter for Syscore0_Spadic" 
		      << addr << FairLogger::endl;
	  } else if ((mp->epoch_count() - fEpochMarkerArray[0][addr]) !=1 ) {
	    LOG(INFO) << "Missed epoch counter for Syscore0_Spadic" 
		      << addr << FairLogger::endl;
	  }
	  fEpochMarkerArray[2][addr] = mp->epoch_count(); 
	  break;
	default:
	  LOG(FATAL) << "EquipmentID " << link << "not known." << FairLogger::endl;
	  break;
	}     
        
      } 
      
      if ( mp->is_hit() ) { 
	Int_t channel = mp->channel_id();
	Int_t epoch;
	switch (link) {
	case kMuenster:  // Muenster
	  epoch = fEpochMarkerArray[0][addr]; 
	  break;
	case kFrankfurt: // Frankfurt
	  epoch = fEpochMarkerArray[1][addr];
	  break;
	case kBucarest: // Bucarest
	  epoch = fEpochMarkerArray[2][addr];
	  break;
	default:
	  LOG(FATAL) << "EquipmentID " << link << "not known." << FairLogger::endl;
	  break;
	}     
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
  }
  return kTRUE;
}

void CbmTSUnpackSpadic::print_message(const spadic::Message& m)
{
  LOG(DEBUG) << "v: " << (m.is_valid() ? "o" : "x");
  LOG(DEBUG) << " / gid: " << static_cast<int>(m.group_id());
  LOG(DEBUG) << " / chid: " << static_cast<int>(m.channel_id());
  if ( m.is_hit() ) { 
    LOG(DEBUG) << " / ts: " << m.timestamp();
    LOG(DEBUG) << " / samples (" << m.samples().size() << "):";
    for (auto x : m.samples()) {
      LOG(DEBUG) << " " << x;
    }
    LOG(DEBUG) << FairLogger::endl;
  } else {
    if ( m.is_epoch_marker() ) { 
      LOG(DEBUG) << " This is an Epoch Marker" << FairLogger::endl; 
    } else if ( m.is_epoch_out_of_sync() ) { 
      LOG(DEBUG) << " This is an out of sync Epoch Marker" << FairLogger::endl; 
    } else {
      LOG(DEBUG) << " This is not known" << FairLogger::endl;
    }
  }
}

void CbmTSUnpackSpadic::Reset()
{
}

void CbmTSUnpackSpadic::Finish()
{
  for (Int_t i=0; i < NrOfSyscores; ++i) { 
    for (Int_t j=0; j < NrOfHalfSpadics; ++j) { 
      LOG(INFO) << "There have been " << fSuperEpochCounter[i][j] 
		<< " SuperEpochs for Syscore" << i << "_Spadic" 
		<< j << " in this file" << FairLogger::endl;
    }
  }

}

/*
void CbmTSUnpackSpadic::Register()
{
}
*/


ClassImp(CbmTSUnpackSpadic)
