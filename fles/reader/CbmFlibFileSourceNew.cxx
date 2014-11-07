// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmFlibFileSourceNew                        -----
// -----                    Created 01.11.2013 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmFlibFileSourceNew.h"
#include "CbmSpadicRawMessage.h"

#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "TimesliceSubscriber.hpp"
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

CbmFlibFileSourceNew::CbmFlibFileSourceNew()
  : FairSource(),
    fFileName(""),
    fHost("localhost"),
    fPort(5556),
    fSpadicRaw(new TClonesArray("CbmSpadicRawMessage", 10)),
    fSource(NULL)
{
}



CbmFlibFileSourceNew::CbmFlibFileSourceNew(const CbmFlibFileSourceNew& source)
  : FairSource(source),
    fFileName(""),
    fHost("localhost"),
    fPort(5556),
    fSpadicRaw(NULL),
    fSource(NULL)
{
}


CbmFlibFileSourceNew::~CbmFlibFileSourceNew()
{
}

Bool_t CbmFlibFileSourceNew::Init()
{
  if ( 0 == fFileName.Length() ) {
    TString connector = Form("tcp://%s:%i", fHost.Data(), fPort);
    LOG(INFO) << "Open TSPublisher at " << connector << FairLogger::endl;
    fSource = new fles::TimesliceSubscriber(connector.Data());
    if ( !fSource) { 
      LOG(FATAL) << "Could not connect to publisher." << FairLogger::endl;
    } 
  } else {
    LOG(INFO) << "Open the Flib input file " << fFileName << FairLogger::endl;
    fSource = new fles::TimesliceInputArchive(fFileName.Data());
    if ( !fSource) { 
      LOG(FATAL) << "Could not open input file." << FairLogger::endl;
    } 
  }

  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    LOG(INFO) << "Initialize " << it->second->GetName() << 
      " for systemID 0x" << std::hex << it->first << FairLogger::endl;
    it->second->Init();
    //    it->second->Register();
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

Int_t CbmFlibFileSourceNew::ReadEvent()
{
  fSpadicRaw->Clear();

  while (auto timeslice = fSource->get()) {
    const fles::Timeslice& ts = *timeslice;
    for (size_t c {0}; c < ts.num_components(); c++) {
      auto systemID = ts.descriptor(c, 0).sys_id;
      
          PrintMicroSliceDescriptor(ts.descriptor(c, 0));

      auto it=fUnpackers.find(systemID);
      if (it == fUnpackers.end()) {
	LOG(FATAL) << "Could not find unpacker for system id 0x" << 
	  std::hex << systemID << FairLogger::endl;
      } else {
        LOG(INFO) << "I am here." << FairLogger::endl;
	it->second->DoUnpack(ts, c);
      }

      /*
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
      case 0x40:
	LOG(INFO) << "It is spadic data with correct system ID" 
		  << FairLogger::endl;
        UnpackSpadicCbmNetMessage(ts, c);
	break;
      default:
	LOG(INFO) << "Not known now" << FairLogger::endl;
      }
      */
    }
    return 0;
  }

  return 1; // no more data; trigger end of run
}

void CbmFlibFileSourceNew::PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc)
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

    
void CbmFlibFileSourceNew::UnpackSpadicCbmNetMessage(const fles::Timeslice& ts, size_t component)
{
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

}

void CbmFlibFileSourceNew::print_message(const spadic::Message& m)
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




Bool_t CbmFlibFileSourceNew::CheckTimeslice(const fles::Timeslice& ts)
{
    if ( 0 == ts.num_components() ) {
      LOG(ERROR) << "No Component in TS " << ts.index() 
		 << FairLogger::endl;
      return 1;
    }
    LOG(INFO) << "Found " << ts.num_components() 
	      << " different components in tiemeslice" << FairLogger::endl; 
    return kTRUE;
}

void CbmFlibFileSourceNew::Close()
{
}

void CbmFlibFileSourceNew::Reset()
{
}
//void CbmFlibFileSourceNew::AddUnpacker(CbmTSUnpack* unpacker, Int_t id)
//{
//}

ClassImp(CbmFlibFileSourceNew)
