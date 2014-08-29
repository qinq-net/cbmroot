// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmFlibFileSource                        -----
// -----                    Created 01.11.2013 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmFlibFileSource.h"
#include "CbmSpadicRawMessage.h"

#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "MicrosliceContents.hpp"

// note M. Krieger, 2014-08-15: these includes should not be needed, please test
#if 0
#include "Message.hpp"
#include "message_reader.h"
#endif
#include "TimesliceReader.hpp"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iostream>

CbmFlibFileSource::CbmFlibFileSource()
  : FairSource(),
    fFileName(""),
    fSpadicRaw(new TClonesArray("CbmSpadicRawMessage", 10)),
    fSource(NULL)
{
}



CbmFlibFileSource::CbmFlibFileSource(const CbmFlibFileSource& source)
  : FairSource(source),
    fFileName(""),
    fSpadicRaw(NULL),
    fSource(NULL)
{
}


CbmFlibFileSource::~CbmFlibFileSource()
{
}

Bool_t CbmFlibFileSource::Init()
{
  LOG(INFO) << "Open the Flib input file" << FairLogger::endl;
  if ( 0 == fFileName.Length() ) {
    LOG(FATAL) << "No input file defined." << FairLogger::endl;
  } else {
    // Open the input file
    fSource = new fles::TimesliceInputArchive(fFileName.Data());
    //fSource.reset(new fles::TimesliceInputArchive(fFileName.Data()));

  }
  if ( !fSource) { 
    LOG(FATAL) << "Could not open input file." << FairLogger::endl;
  } 

  FairRootManager* ioman = FairRootManager::Instance();
  ioman->Register("SpadicRawMessage", "spadic raw data", fSpadicRaw, kTRUE);

  /*
  auto timeslice = fSource->get();
  const fles::Timeslice& ts = *timeslice;
  for (size_t c {0}; c < ts.num_components(); c++) {
    auto systemID = ts.descriptor(c, 0).sys_id;
    auto& desc = ts.descriptor(c, 0);

    LOG(INFO) << "Found subsystem with ID: " << std::hex << (int)desc.sys_id 
	      << " and version " << (int)desc.sys_ver 
	      << " for component " << c << FairLogger::endl;
    
    //    switch (desc.sys_id) {
    switch (systemID) {
    case 0xFA:
      LOG(INFO) << "It is flesnet pattern generator data" << FairLogger::endl;
      break;
    case 0xF0:
      LOG(INFO) << "It is flib pattern generator data" << FairLogger::endl;
      break;
    case 0xBC:
      LOG(INFO) << "It is spadic data with wrong system ID" << FairLogger::endl;
      break;
    default:
      LOG(INFO) << "Not known now" << FairLogger::endl;
    }

  }
*/
  
  return kTRUE;

}

Int_t CbmFlibFileSource::ReadEvent()
{
  fSpadicRaw->Clear();

  while (auto timeslice = fSource->get()) {
    const fles::Timeslice& ts = *timeslice;
    for (size_t c {0}; c < ts.num_components(); c++) {
      auto systemID = ts.descriptor(c, 0).sys_id;
      
      PrintMicroSliceDescriptor(ts.descriptor(c, 0));

      switch (systemID) {
      case 0xFA:
	LOG(INFO) << "It is flesnet pattern generator data" << FairLogger::endl;
	break;
      case 0xF0:
	LOG(INFO) << "It is flib pattern generator data" << FairLogger::endl;
	break;
      case 0xBC:
	LOG(INFO) << "It is spadic data with wrong system ID" 
		  << FairLogger::endl;
        UnpackSpadicCbmNetMessage(ts, c);
	break;
      default:
	LOG(INFO) << "Not known now" << FairLogger::endl;
      }
    }
    return 0;
  }

  return 1; // no more data; trigger end of run
}

void CbmFlibFileSource::PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc)
{
  LOG(INFO) << "Header ID: Ox" << std::hex << static_cast<int>(mdsc.hdr_id) 
	    << FairLogger::endl;
  LOG(INFO) << "Header version: Ox" << std::hex << static_cast<int>(mdsc.hdr_ver) 
	    << std::dec << FairLogger::endl;
  LOG(INFO) << "Equipement ID: " << mdsc.eq_id << FairLogger::endl;
  LOG(INFO) << "Flags: " << mdsc.flags << FairLogger::endl;
  LOG(INFO) << "Sys ID: Ox" << std::hex << static_cast<int>(mdsc.sys_id) 
	    << FairLogger::endl;
  LOG(INFO) << "Sys version: Ox" << std::hex << static_cast<int>(mdsc.sys_ver) 
	    << std::dec << FairLogger::endl;
  LOG(INFO) << "Microslice Idx: " << mdsc.idx << FairLogger::endl; 
  LOG(INFO) << "Checksum: " << mdsc.crc << FairLogger::endl;
  LOG(INFO) << "Size: " << mdsc.size << FairLogger::endl;
  LOG(INFO) << "Offset: " << mdsc.offset << FairLogger::endl;
}

    
void CbmFlibFileSource::UnpackSpadicCbmNetMessage(const fles::Timeslice& ts, size_t component)
{
  spadic::TimesliceReader r;
  Int_t counter=0;

  r.add_component(ts, component);

  for (auto addr : r.sources()) {
    std::cout << "---- reader " << addr << " ----" << std::endl;
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

}

void CbmFlibFileSource::print_message(const spadic::Message& m)
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
    } else {
      std::cout << " This is not known" << std::endl;
    }
  }
}




Bool_t CbmFlibFileSource::CheckTimeslice(const fles::Timeslice& ts)
{
    if ( 0 == ts.num_components() ) {
      LOG(ERROR) << "No Component in TS " << ts.index() 
		 << FairLogger::endl;
      return 1;
    }
    LOG(INFO) << "Found " << ts.num_components() 
	      << " different components in tiemeslice" << FairLogger::endl; 
}

void CbmFlibFileSource::Close()
{
}

void CbmFlibFileSource::Reset()
{
}


ClassImp(CbmFlibFileSource)
