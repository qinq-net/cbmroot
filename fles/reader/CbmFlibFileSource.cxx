// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmFlibFileSource                        -----
// -----                    Created 01.11.2013 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmFlibFileSource.h"

#include "FairLogger.h"

#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "MicrosliceContents.hpp"

#include "Message.hpp"
#include "message_reader.h"
#include "TimesliceReader.hpp"

#include <iostream>

CbmFlibFileSource::CbmFlibFileSource()
  : FairSource(),
    fFileName(""),
    fSource(NULL)
{
}



CbmFlibFileSource::CbmFlibFileSource(const CbmFlibFileSource& source)
  : FairSource(source),
    fFileName(""),
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
  Int_t counter=0;

  while (auto timeslice = fSource->get()) {
    const fles::Timeslice& ts = *timeslice;
    for (size_t c {0}; c < ts.num_components(); c++) {
      auto systemID = ts.descriptor(c, 0).sys_id;
      
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
  }

  return 1; // no more data; trigger end of run
}
    
void CbmFlibFileSource::UnpackSpadicCbmNetMessage(const fles::Timeslice& ts, size_t component)
{
  spadic::TimesliceReader r;

  r.add_timeslice_cbmroot(ts, component);

  for (auto addr : r.sources()) {
    std::cout << "---- reader " << addr << " ----" << std::endl;
    while (auto mp = r.get_message(addr)) {
      print_message(*mp);
    }
  }

}

void CbmFlibFileSource::print_message(const spadic::Message& m)
{
  std::cout << "v: " << (m.is_valid() ? "o" : "x");
  std::cout << " / ts: " << m.timestamp();
  std::cout << " / samples (" << m.samples().size() << "):";
  for (auto x : m.samples()) {
    std::cout << " " << x;
  }
  std::cout << std::endl;
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
