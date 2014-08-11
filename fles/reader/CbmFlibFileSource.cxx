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
  }
  
  return kTRUE;

}

Int_t CbmFlibFileSource::ReadEvent()
{
  Int_t counter=0;

  while (auto timeslice = fSource->get()) {
    const fles::Timeslice& ts = *timeslice;
    for (size_t c {0}; c < ts.num_components(); c++) {
      for (size_t m {0}; m < ts.num_microslices(c); m++) {
        auto& desc = ts.descriptor(c, m);
        auto p = ts.content(c, m);
        // TODO check ts.sys_id, ts.sys_version
        // TODO check same source address from different components
        flib_dpb::MicrosliceContents mc {p, desc.size};
	for (auto& dtm : mc.dtms()) {
	  uint16_t cbmnet_src_addr = dtm.data[0];
	  const uint16_t *spadic_buffer = dtm.data + 1;
	  size_t spadic_buffer_size = dtm.size - 1;
            // TODO use a separate spadic::MessageReader for each cbmnet_src_addr
          spadic::MessageReader reader;

	  reader.add_buffer(spadic_buffer, spadic_buffer_size);
	//            while (auto spadic_msg = reader.get_message()) {
	  // ???
	}
      }
    }
  }


//  while (auto timeslice = fSource->get()) {
//    CheckTimeslice(*timeslice);
//    ++counter;
//  }
//  LOG(INFO) << "Processed " << counter << " timeslices"<<FairLogger::endl;

  return 1; // no more data; trigger end of run
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
