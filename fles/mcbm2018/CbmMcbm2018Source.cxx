// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmMcbm2018Source                     -----
// -----                    Created 19.01.2018 by P.-A. Loizeau            -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018Source.h"

#include "CbmTbDaqBuffer.h"

#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "TimesliceSubscriber.hpp"
#include "MicrosliceContents.hpp"

#include "FairLogger.h"
#include "FairRunOnline.h"

#include "TH1.h"
#include "THttpServer.h"
#include "TProfile.h"

#include <iostream>
#include <fstream>
#include <iomanip>

CbmMcbm2018Source::CbmMcbm2018Source()
  : FairSource(),
    fFileName(""),
    fInputFileList(new TObjString()),
    fFileCounter(0),
    fHost("localhost"),
    fPort(5556),
    fbOutputData( kFALSE ),
    fUnpackers(),
    fDetectorSystemMap(),
    fUnpackersToRun(),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fTSNumber(0),
    fTSCounter(0),
    fTimer(),
    fBufferFillNeeded(kTRUE),
    fHistoMissedTS(NULL),
    fHistoMissedTSEvo(NULL),
    fNofTSSinceLastTS(0),
    fuTsReduction(1),
    fSource(NULL)
{
}

CbmMcbm2018Source::CbmMcbm2018Source(const CbmMcbm2018Source& source)
  : FairSource(source),
    fFileName(""),
    fInputFileList(),
    fFileCounter(0),
    fHost("localhost"),
    fPort(5556),
    fbOutputData( kFALSE ),
    fUnpackers(),
    fDetectorSystemMap(),
    fUnpackersToRun(),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fTSNumber(0),
    fTSCounter(0),
    fTimer(),
    fBufferFillNeeded(kTRUE),
    fHistoMissedTS(NULL),
    fHistoMissedTSEvo(NULL),
    fNofTSSinceLastTS(0),
    fuTsReduction(1),
    fSource(NULL)
{
}

CbmMcbm2018Source::~CbmMcbm2018Source()
{
}

Bool_t CbmMcbm2018Source::Init()
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

    if( kFALSE == OpenNextFile() )
    {
      LOG(ERROR) << "Could not open the first file in the list, Doing nothing!" << FairLogger::endl;
      return kFALSE;
    } // if( kFALSE == OpenNextFile() )
  }

  /// Build list of unpackers without multiples from unpacker dealing with 2 or more detectors
  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it)
    fUnpackersToRun.insert( it->second );

  for( auto itUnp = fUnpackersToRun.begin(); itUnp != fUnpackersToRun.end(); ++ itUnp ) {
    LOG(INFO) << "Initialize " << (*itUnp)->GetName() << FairLogger::endl;
    (*itUnp)->Init();
    //    it->second->Register();
  }

  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();

  fHistoMissedTS = new TH1I("Missed_TS", "Missed TS", 2, 0., 2.);
  fHistoMissedTSEvo  = new TProfile("Missed_TS_Evo", "Missed TS evolution; TS Idx []", 100000, 0., 10000000.);

  if (server)
  {
    server->Register("/TofRaw", fHistoMissedTS);
    server->Register("/TofRaw", fHistoMissedTSEvo);
  } // if (server)


  return kTRUE;
}

void CbmMcbm2018Source::SetParUnpackers()
{
	for( auto itUnp = fUnpackersToRun.begin(); itUnp != fUnpackersToRun.end(); ++ itUnp ) {
		LOG(INFO) << "Set parameter container " << (*itUnp)->GetName() << FairLogger::endl;
	    (*itUnp)->SetParContainers();
	  }

}

Bool_t CbmMcbm2018Source::InitUnpackers()
{
	Bool_t result = kTRUE;
	for( auto itUnp = fUnpackersToRun.begin(); itUnp != fUnpackersToRun.end(); ++ itUnp ) {
		LOG(INFO) << "Initialize parameter container " << (*itUnp)->GetName() << FairLogger::endl;
	    result = result && (*itUnp)->InitContainers();
	}
    return result;
}

Bool_t CbmMcbm2018Source::ReInitUnpackers()
{
	Bool_t result = kTRUE;
	for( auto itUnp = fUnpackersToRun.begin(); itUnp != fUnpackersToRun.end(); ++ itUnp ) {
		LOG(INFO) << "Initialize parameter container " << (*itUnp)->GetName() << FairLogger::endl;
	    result = result && (*itUnp)->ReInitContainers();
	}
    return result;
}

Int_t CbmMcbm2018Source::ReadEvent(UInt_t)
{
//  LOG(INFO) << FairLogger::endl;
//  LOG(INFO) << "Before FillBuffer" << FairLogger::endl;
  Int_t retVal = -1;
  if (fBufferFillNeeded) {
    FillBuffer();
  }
//  LOG(INFO) << "After FillBuffer" << FairLogger::endl;

  retVal = GetNextEvent();
  LOG(DEBUG) << "After GetNextEvent: " << retVal << FairLogger::endl;

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

void CbmMcbm2018Source::PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc)
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

Bool_t CbmMcbm2018Source::CheckTimeslice(const fles::Timeslice& ts)
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

void CbmMcbm2018Source::Close()
{
  for( auto itUnp = fUnpackersToRun.begin(); itUnp != fUnpackersToRun.end(); ++ itUnp )
  {
    LOG(INFO) << "Finish " << (*itUnp)->GetName() << FairLogger::endl;
    (*itUnp)->Finish();
  }
  fHistoMissedTS->Write();
  fHistoMissedTSEvo->Write();
}

void CbmMcbm2018Source::Reset()
{
  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    it->second->Reset();
  }
}

Int_t CbmMcbm2018Source::FillBuffer()
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
        fHistoMissedTSEvo->Fill( tsIndex, 1, tsIndex-fTSNumber);
        fNofTSSinceLastTS=tsIndex-fTSNumber;
      } else {
        fHistoMissedTS->Fill(0);
        fHistoMissedTSEvo->Fill( tsIndex, 0, 1);
        fNofTSSinceLastTS=1;
      }
      fTSNumber=tsIndex;

      if ( 0 ==fTSNumber%1000 ) {
        LOG(INFO) << "Reading Timeslice " << fTSNumber
                  << FairLogger::endl;
      }

      if( 1 == fTSCounter )
      {
         for (size_t c {0}; c < ts.num_components(); c++)
         {
            auto systemID = static_cast<int>(ts.descriptor(c, 0).sys_id);

            LOG(DEBUG) << "Found systemID: " << std::hex
                       << systemID << std::dec << FairLogger::endl;
/*
            auto it=fUnpackers.find(systemID);
            if( it == fUnpackers.end() )
            {
                LOG(INFO) << "Could not find unpacker for system id 0x"
                          << std::hex << systemID << std::dec
                          << FairLogger::endl;
            } // if( it == fUnpackers.end() )
               else
               {
                  it->second->AddMsComponentToList( c, systemID );
                  it->second->SetNbMsInTs( ts.num_core_microslices(),
                                           ts.num_microslices( c ) - ts.num_core_microslices() );
               } // else of if( it == fUnpackers.end() )
*/
            /// Get range of all unpackers matching this system ID <= Trick for STS + MUCH
            auto it_list = fUnpackers.equal_range(systemID);
            if( it_list.first == it_list.second )
            {
               LOG(INFO) << "Could not find unpacker for system id 0x"
                         << std::hex << systemID << std::dec
                         << FairLogger::endl;
            } // if( it == fUnpackers.end() )
               else
               {
                  for( auto it = it_list.first; it != it_list.second; ++it )
                  {
                     it->second->AddMsComponentToList( c, systemID );
                     it->second->SetNbMsInTs( ts.num_core_microslices(),
                                             ts.num_microslices( c ) - ts.num_core_microslices() );
                  } // for( auto it = it_list.first; it != it_list.second; ++it )
               } // else of if( it == fUnpackers.end() )
         } // for (size_t c {0}; c < ts.num_components(); c++)
      } // if( 1 == fTSCounter )

      /// Apply TS throttling as set by user (default = 1 => no throttling)
      if( 0 == tsIndex % fuTsReduction )
      {
         for( auto itUnp = fUnpackersToRun.begin(); itUnp != fUnpackersToRun.end(); ++ itUnp )
         {
            (*itUnp)->DoUnpack(ts, 0);
         } // for( auto itUnp = fUnpackersToRun.begin(); itUnp != fUnpackersToRun.end(); ++ itUnp )
      } // if( 0 == tsIndex % fuTsReduction )
/*
      for (size_t c {0}; c < ts.num_components(); c++) {
        auto systemID = static_cast<int>(ts.descriptor(c, 0).sys_id);

        LOG(DEBUG) << "Found systemID: " << std::hex
                  << systemID << std::dec << FairLogger::endl;

        auto it=fUnpackers.find(systemID);
        if (it == fUnpackers.end()) {
          LOG(DEBUG) << "Could not find unpacker for system id 0x"
                     << std::hex << systemID << std::dec
                     << FairLogger::endl;
        } else {
           if( 0 == tsIndex%fuTsReduction ||  systemID != 0x10 )
             it->second->DoUnpack(ts, c);
        }
      }
*/
      return 0;
    }

    return 1;
}

Int_t CbmMcbm2018Source::GetNextEvent()
{

  if( kTRUE == fbOutputData )
  {
    Double_t fTimeBufferOut = fBuffer->GetTimeLast();
    LOG(DEBUG) << "Timeslice contains data from "
              << std::setprecision(9) << std::fixed
              << static_cast<Double_t>(fBuffer->GetTimeFirst()) * 1.e-9 << " to "
              << std::setprecision(9) << std::fixed
              << static_cast<Double_t>(fBuffer->GetTimeLast()) * 1.e-9 << " s" << FairLogger::endl;

    LOG(DEBUG) << "Buffer has " << fBuffer->GetSize() << " entries." << FairLogger::endl;


    CbmDigi* digi = fBuffer->GetNextData(fTimeBufferOut);

//    LOG(INFO) << "Before if" << FairLogger::endl;
    if (NULL == digi) return 1;
//    LOG(INFO) << "After if" << FairLogger::endl;

    while(digi) {
      Int_t detId = digi->GetSystemId();
/*
      Int_t flibId = fDetectorSystemMap[detId];
      LOG(DEBUG) << "Digi has system ID " << detId
                << " which maps to FlibId "<< flibId << FairLogger::endl;
*/
      //std::map<Int_t, CbmTSUnpack*>::iterator it=fUnpackers.find(flibId);
      std::map<Int_t, CbmMcbmUnpack*>::iterator it=fDetectorSystemMap.find(detId);

      if (it == fUnpackers.end()) {
        LOG(ERROR) << "Skipping digi with unknown id "
                   << detId << FairLogger::endl;
        continue;
      } else {
        it->second->FillOutput(digi);
      }
      digi = fBuffer->GetNextData(fTimeBufferOut);
    }
  } // if( kTRUE == fbOutputData )

  return 0;
}

Bool_t CbmMcbm2018Source::OpenNextFile()
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


ClassImp(CbmMcbm2018Source)
