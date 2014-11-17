// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmFlibFileSourceNew                        -----
// -----                    Created 01.11.2013 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmFlibFileSourceNew.h"

#include "CbmDaqBuffer.h"

#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "TimesliceSubscriber.hpp"
#include "MicrosliceContents.hpp"

#include "FairLogger.h"

#include <iostream>

CbmFlibFileSourceNew::CbmFlibFileSourceNew()
  : FairSource(),
    fFileName(""),
    fHost("localhost"),
    fPort(5556),
    fUnpackers(),
    fBuffer(CbmDaqBuffer::Instance()),
    fSource(NULL)
{
}

CbmFlibFileSourceNew::CbmFlibFileSourceNew(const CbmFlibFileSourceNew& source)
  : FairSource(source),
    fFileName(""),
    fHost("localhost"),
    fPort(5556),
    fUnpackers(),
    fBuffer(CbmDaqBuffer::Instance()),
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
  
  return kTRUE;
}

Int_t CbmFlibFileSourceNew::ReadEvent()
{

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

ClassImp(CbmFlibFileSourceNew)
