// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmFlibCern2016Source                        -----
// -----                    Created 20.06.2016 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmFlibCern2016Source.h"

#include "CbmTbDaqBuffer.h"
 
#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "TimesliceSubscriber.hpp"
#include "MicrosliceContents.hpp"

#include "FairLogger.h"
#include "FairRunOnline.h"

#include "TH1.h"
#include "THttpServer.h"

#include <iostream>
#include <fstream>
#include <iomanip>

CbmFlibCern2016Source::CbmFlibCern2016Source()
  : FairSource(),
    fFileName(""),
    fInputFileList(new TObjString()),
    fFileCounter(0),
    fHost("localhost"),
    fPort(5556),
    fUnpackers(),
    fDetectorSystemMap(),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fTSNumber(0),
    fTSCounter(0),
    fTimer(),
    fBufferFillNeeded(kTRUE),
    fHistoMissedTS(NULL),
    fNofTSSinceLastTS(0),
    fSource(NULL)
{
}

CbmFlibCern2016Source::CbmFlibCern2016Source(const CbmFlibCern2016Source& source)
  : FairSource(source),
    fFileName(""),
    fInputFileList(),
    fFileCounter(0),
    fHost("localhost"),
    fPort(5556),
    fUnpackers(),
    fDetectorSystemMap(),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fTSNumber(0),
    fTSCounter(0),
    fTimer(),
    fBufferFillNeeded(kTRUE),
    fHistoMissedTS(NULL),
    fNofTSSinceLastTS(),
    fSource(NULL)
{
}

CbmFlibCern2016Source::~CbmFlibCern2016Source()
{
}

Bool_t CbmFlibCern2016Source::Init()
{
  if ( 0 == fFileName.Length() && 0 == fInputFileList.GetSize()) {
    TString connector = Form("tcp://%s:%i", fHost.Data(), fPort);
    LOG(INFO) << "Open TSPublisher at " << connector << FairLogger::endl;
    fInputFileList.Add(new TObjString(connector));
    fSource = new fles::TimesliceSubscriber(connector.Data());
    if ( !fSource) { 
      LOG(FATAL) << "Could not connect to publisher." << FairLogger::endl;
    } 
  } else {
/*     
    // --- Open input file 
    TObjString* tmp =
      dynamic_cast<TObjString*>(fInputFileList.At(fFileCounter));
    fFileName = tmp->GetString();

    LOG(INFO) << "Open the Flib input file " << fFileName << FairLogger::endl;
    // Check if the input file exist
    FILE* inputFile = fopen(fFileName.Data(), "r");
    if ( ! inputFile )  {
      LOG(FATAL) << "Input file " << fFileName << " doesn't exist." << FairLogger::endl;
    }
    fclose(inputFile);
    fSource = new fles::TimesliceInputArchive(fFileName.Data());
    if ( !fSource) { 
      LOG(FATAL) << "Could not open input file." << FairLogger::endl;
    } 
*/ 
    if( kFALSE == OpenNextFile() )
    {
      LOG(ERROR) << "Could not open the first file in the list, Doing nothing!" << FairLogger::endl;
      return kFALSE;
    } // if( kFALSE == OpenNextFile() )
  }

  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    LOG(INFO) << "Initialize " << it->second->GetName() << 
      " for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
    it->second->Init();
    //    it->second->Register();
  }
  
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
//  server->SetJSROOT("https://root.cern.ch/js/latest");
#endif

  fHistoMissedTS = new TH1I("Missed TS", "Missed TS", 2, 0., 2.);

#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHistoMissedTS);
#endif


  return kTRUE;
}

void CbmFlibCern2016Source::SetParUnpackers()
{
	for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
		LOG(INFO) << "Set parameter container " << it->second->GetName() <<
				" for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
	    it->second->SetParContainers();
	  }

}

Bool_t CbmFlibCern2016Source::InitUnpackers()
{
	Bool_t result = kTRUE;
	for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
		LOG(INFO) << "Initialize parameter container " << it->second->GetName() <<
				" for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
	    result = result && it->second->InitContainers();
	}
    return result;
}

Bool_t CbmFlibCern2016Source::ReInitUnpackers()
{
	Bool_t result = kTRUE;
	for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
		LOG(INFO) << "Initialize parameter container " << it->second->GetName() <<
				" for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
	    result = result && it->second->ReInitContainers();
	}
    return result;
}

Int_t CbmFlibCern2016Source::ReadEvent(UInt_t) 
{
//  LOG(INFO) << FairLogger::endl;
//  LOG(INFO) << "Before FillBuffer" << FairLogger::endl;
  Int_t retVal = -1;  
  if (fBufferFillNeeded) {
    FillBuffer();
  }
//  LOG(INFO) << "After FillBuffer" << FairLogger::endl;

  retVal = GetNextEvent();
//  LOG(INFO) << "After GetNextEvent: " << retVal << FairLogger::endl;
  
  Int_t bla = 0;
  // If no more data and file mode, try to read next file in List
  if( fSource->eos() && 0 < fFileName.Length() )
  {
    fFileCounter ++; // Increment file counter to go to next item in List
    bla = ( kFALSE == OpenNextFile() ? 1 : 0 );
  } // if( fSource->eos() && 0 < fFileName.Length() )
  
//  Int_t bla = fSource->eos(); // no more data; trigger end of run
//  LOG(INFO) << "After fSource->eos: " << bla << FairLogger::endl;
  return bla; // no more data; trigger end of run
}

void CbmFlibCern2016Source::PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc)
{
  LOG(INFO) << "Header ID: Ox" << std::hex << static_cast<int>(mdsc.hdr_id) 
            << std::dec << FairLogger::endl;
  LOG(INFO) << "Header version: Ox" << std::hex << static_cast<int>(mdsc.hdr_ver) 
            << std::dec << FairLogger::endl;
  LOG(INFO) << "Equipement ID: " << mdsc.eq_id << FairLogger::endl;
  LOG(INFO) << "Flags: " << mdsc.flags << FairLogger::endl;
  LOG(INFO) << "Sys ID: Ox" << std::hex << static_cast<int>(mdsc.sys_id) 
            << std::dec << FairLogger::endl;
  LOG(INFO) << "Sys version: Ox" << std::hex << static_cast<int>(mdsc.sys_ver) 
            << std::dec << FairLogger::endl;
  LOG(INFO) << "Microslice Idx: " << mdsc.idx << FairLogger::endl; 
  LOG(INFO) << "Checksum: " << mdsc.crc << FairLogger::endl;
  LOG(INFO) << "Size: " << mdsc.size << FairLogger::endl;
  LOG(INFO) << "Offset: " << mdsc.offset << FairLogger::endl;
}

Bool_t CbmFlibCern2016Source::CheckTimeslice(const fles::Timeslice& ts)
{
    if ( 0 == ts.num_components() ) {
      LOG(ERROR) << "No Component in TS " << ts.index() 
                 << FairLogger::endl;
      return 1;
    }
    LOG(INFO) << "Found " << ts.num_components() 
              << " different components in timeslice" << FairLogger::endl; 
    return kTRUE;
}

void CbmFlibCern2016Source::Close()
{
  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    LOG(INFO) << "Finish " << it->second->GetName() << " for systemID 0x" 
              << std::hex << it->first << std::dec << FairLogger::endl;
    it->second->Finish();
  }
  fHistoMissedTS->Write();
}

void CbmFlibCern2016Source::Reset()
{
  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    it->second->Reset();
  }
}

Int_t CbmFlibCern2016Source::FillBuffer()
{

    while (auto timeslice = fSource->get()) {
      fTSCounter++;
      if ( 0 == fTSCounter%10000 ) {
        LOG(INFO) << "Analyse Event " << fTSCounter << FairLogger::endl;
      }
            
      const fles::Timeslice& ts = *timeslice;
      auto tsIndex = ts.index();
      if( (tsIndex != (fTSNumber+1)) &&( fTSNumber != 0) ) {
        LOG(DEBUG) << "Missed Timeslices. Old TS Number was " << fTSNumber 
                     << " New TS Number is " << tsIndex << FairLogger::endl;
        fHistoMissedTS->Fill(1, tsIndex-fTSNumber);
        fNofTSSinceLastTS=tsIndex-fTSNumber;
      } else {
        fHistoMissedTS->Fill(0);
        fNofTSSinceLastTS=1;
      }
      fTSNumber=tsIndex;

      if ( 0 ==fTSNumber%1000 ) {
        LOG(INFO) << "Reading Timeslice " << fTSNumber
                  << FairLogger::endl;    
      }

      for (size_t c {0}; c < ts.num_components(); c++) {
        auto systemID = static_cast<int>(ts.descriptor(c, 0).sys_id);

        LOG(DEBUG) << "Found systemID: " << std::hex 
                  << systemID << std::dec << FairLogger::endl;
        
        if(gLogger->IsLogNeeded(DEBUG1)) {
          PrintMicroSliceDescriptor(ts.descriptor(c, 0));
        }

        auto it=fUnpackers.find(systemID);
        if (it == fUnpackers.end()) {
          LOG(FATAL) << "Could not find unpacker for system id 0x" 
                     << std::hex << systemID << std::dec 
                     << FairLogger::endl;
        } else {
          it->second->DoUnpack(ts, c);
        }
      }
      return 0;
    }

    return 1;
}

Int_t CbmFlibCern2016Source::GetNextEvent()
{

  Double_t fTimeBufferOut = fBuffer->GetTimeLast();
  LOG(DEBUG) << "Timeslice contains data from "
            << std::setprecision(9) << std::fixed 
            << static_cast<Double_t>(fBuffer->GetTimeFirst()) * 1.e-9 << " to "
            << std::setprecision(9) << std::fixed 
            << static_cast<Double_t>(fBuffer->GetTimeLast()) * 1.e-9 << " s" << FairLogger::endl;

  LOG(DEBUG) << "Buffer has " << fBuffer->GetSize() << " entries." << FairLogger::endl;


  CbmDigi* digi = fBuffer->GetNextData(fTimeBufferOut);

//  LOG(INFO) << "Before if" << FairLogger::endl;
  if (NULL == digi) return 1; 
//  LOG(INFO) << "After if" << FairLogger::endl;

  while(digi) {
    Int_t detId = digi->GetSystemId();
    Int_t flibId = fDetectorSystemMap[detId];
    LOG(DEBUG) << "Digi has system ID " << detId 
              << " which maps to FlibId "<< flibId << FairLogger::endl;
    std::map<Int_t, CbmTSUnpack*>::iterator it=fUnpackers.find(flibId);
    if (it == fUnpackers.end()) {
      LOG(ERROR) << "Skipping digi with unknown id " 
                 << detId << FairLogger::endl;
      continue;
    } else {
      it->second->FillOutput(digi);
    }
    digi = fBuffer->GetNextData(fTimeBufferOut);
  }; 

  return 0;
}

Bool_t CbmFlibCern2016Source::OpenNextFile()
{
   // First Close and delete existing source
   if( NULL != fSource )
      delete fSource;

   if( fFileCounter < fInputFileList.GetSize() )
   {
      // --- Open current input file 
      TObjString* tmp =
      dynamic_cast<TObjString*>(fInputFileList.At(fFileCounter));
      fFileName = tmp->GetString();

      LOG(INFO) << "Open the Flib input file " << fFileName << FairLogger::endl;
      // Check if the input file exist
      FILE* inputFile = fopen(fFileName.Data(), "r");
      if ( ! inputFile )  {
         LOG(ERROR) << "Input file " << fFileName << " doesn't exist." << FairLogger::endl;
         return kFALSE;
      }
      fclose(inputFile);
      fSource = new fles::TimesliceInputArchive(fFileName.Data());
      if ( !fSource) { 
         LOG(ERROR) << "Could not open input file." << FairLogger::endl;
         return kFALSE;
      } 
   } // if( fFileCounter < fInputFileList.GetSize() )
      else
      {
         LOG(INFO) << "End of files list reached: file counter is " << fFileCounter
                   << " for " << fInputFileList.GetSize() << " entries in the file list."
                   << FairLogger::endl;
         return kFALSE;
      } // else of if( fFileCounter < fInputFileList.GetSize() )
   
   return kTRUE;
}


ClassImp(CbmFlibCern2016Source)
