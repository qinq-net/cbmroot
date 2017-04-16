// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmFlibTestSource                        -----
// -----                    Created 20.06.2016 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmFlibTestSource.h"

#include "CbmTbDaqBuffer.h"
 
#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "TimesliceSubscriber.hpp"
#include "MicrosliceContents.hpp"

#include "FairLogger.h"

#include "TSystem.h"
#include "TList.h"
#include "TObjString.h"
#include "TRegexp.h"
#include "TSystemDirectory.h"

#include <iostream>
#include <fstream>
#include <iomanip>
  
std::vector<CbmDigi*> vdigi;

CbmFlibTestSource::CbmFlibTestSource()
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
    fiReqDigiAddr(),
    fdMaxDeltaT(500.),
    fTimer(),
    fBufferFillNeeded(kTRUE),
    fSource(NULL)
{
}

CbmFlibTestSource::CbmFlibTestSource(const CbmFlibTestSource& source)
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
    fiReqDigiAddr(),
    fdMaxDeltaT(100.),
    fTimer(),
    fBufferFillNeeded(kTRUE),
    fSource(NULL)
{
}

CbmFlibTestSource::~CbmFlibTestSource()
{
}

Bool_t CbmFlibTestSource::Init()
{
  Int_t fNFiles = fInputFileList.GetEntries();
  if ( fiReqDigiAddr.size()>0 )
  for(Int_t i=0; i<fiReqDigiAddr.size(); i++)
  LOG(INFO) << Form("Look for 0x%08x digis in %d input files",fiReqDigiAddr[i],fNFiles) << FairLogger::endl;
  if ( 0 == fNFiles ) { //fFileName.Length() ) {
    TString connector = Form("tcp://%s:%i", fHost.Data(), fPort);
    LOG(INFO) << "Open TSPublisher at " << connector << FairLogger::endl;
    fInputFileList.Add(new TObjString(connector));
    fSource = new fles::TimesliceSubscriber(connector.Data());
    if ( !fSource) { 
      LOG(FATAL) << "Could not connect to publisher." << FairLogger::endl;
    } 
  } else {
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
  }

  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    LOG(INFO) << "Initialize " << it->second->GetName() << 
      " for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
    it->second->Init();
    //    it->second->Register();
  }
  
  return kTRUE;
}

void CbmFlibTestSource::AddPath(const TString& tFileDirectory,
                           const TString& tFileNameWildCard)
{

  FileStat_t tFileStat;
  if(1 == gSystem->GetPathInfo(tFileDirectory.Data(), tFileStat))
  {
    gLogger->Fatal(MESSAGE_ORIGIN,
                   TString::Format("\nInput data file directory %s does not "
                                   "exist.",
                                   tFileDirectory.Data()
                                  ).Data()
                  );
  }

  TRegexp* tRegexp = new TRegexp(tFileNameWildCard.Data(), kTRUE);

  TSystemDirectory* tSystemDirectory = 
         new TSystemDirectory("dir", tFileDirectory.Data());

  TString tDirectoryName(tFileDirectory);

  if(!tDirectoryName.EndsWith("/"))
  {
      tDirectoryName += "/";
  }
  TList* tList = tSystemDirectory->GetListOfFiles();

  for (Int_t iRun=0; iRun<100; iRun++) {
    
    Char_t* cRun = Form("_%04d.t",iRun);
    TIterator* tIter = tList->MakeIterator();
    TSystemFile* tSystemFile;
    TString tFileName;
    

    while(NULL != (tSystemFile = (TSystemFile*)tIter->Next()))
    {
      tFileName = tSystemFile->GetName();

      //LOG(INFO)<<" Check file  "<<tFileName.Data()<<" for "<<cRun<<FairLogger::endl;
 
      if( tFileName.Contains(*tRegexp) && tFileName.Contains(cRun) )
      {
	tFileName = tDirectoryName + tFileName;

	LOG(INFO)<<" Add file to input "<<tFileName.Data()<<FairLogger::endl; 

	AddFile(tFileName);
	break;
      }
    }
  }
  tList->Delete();
  if(0==fInputFileList.GetEntries())
  LOG(FATAL)<<" did not find any valid input file at "<<tFileDirectory.Data()<<FairLogger::endl; 
}

void CbmFlibTestSource::SetParUnpackers()
{
	for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
		LOG(INFO) << "Set parameter container " << it->second->GetName() <<
				" for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
	    it->second->SetParContainers();
	  }

}

Bool_t CbmFlibTestSource::InitUnpackers()
{
	Bool_t result = kTRUE;
	for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
		LOG(INFO) << "Initialize parameter container " << it->second->GetName() <<
				" for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
	    result = result && it->second->InitContainers();
	}
    return result;
}

Bool_t CbmFlibTestSource::ReInitUnpackers()
{
	Bool_t result = kTRUE;
	for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
		LOG(INFO) << "Initialize parameter container " << it->second->GetName() <<
				" for systemID 0x" << std::hex << it->first << std::dec << FairLogger::endl;
	    result = result && it->second->ReInitContainers();
	}
    return result;
}

Int_t CbmFlibTestSource::ReadEvent(UInt_t iEv) 
{

  LOG(DEBUG) << "Request received for "<<iEv<<". event "
             << FairLogger::endl;

    
  while ( 1 ==  GetNextEvent()){   
      if (fBufferFillNeeded) {
	Int_t iRet = FillBuffer();
	if (iRet>0) break;  // no more data
      }
  }

  if ( fSource->eos() && fFileCounter < fInputFileList.GetEntries()){	  
    fFileCounter++;
    TObjString* tmp =
      dynamic_cast<TObjString*>(fInputFileList.At(fFileCounter));
    if(NULL != tmp) {
      fFileName = tmp->GetString();
      LOG(INFO) << "Open the "<<fFileCounter<<". ("<<fInputFileList.GetEntries()
		<<") Flib input file " << fFileName << FairLogger::endl;
      delete(fSource);
      fSource = new fles::TimesliceInputArchive(fFileName.Data());
      if ( !fSource) { 
	LOG(FATAL) << "Could not open input file." << FairLogger::endl;
      } 
    }
  }
  return fSource->eos(); // no more data; trigger end of run
}

void CbmFlibTestSource::PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc)
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

Bool_t CbmFlibTestSource::CheckTimeslice(const fles::Timeslice& ts)
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

void CbmFlibTestSource::Close()
{
  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    LOG(INFO) << "Finish " << it->second->GetName() << " for systemID 0x" 
              << std::hex << it->first << std::dec << FairLogger::endl;
    it->second->Finish();
  }

}

void CbmFlibTestSource::Reset()
{
  for (auto it=fUnpackers.begin(); it!=fUnpackers.end(); ++it) {
    it->second->Reset();
  }
}

Int_t CbmFlibTestSource::FillBuffer()
{

    while (auto timeslice = fSource->get()) {
      fTSCounter++;
      if ( 0 == fTSCounter%10000 ) {
        LOG(INFO) << "Analyse Event " << fTSCounter << FairLogger::endl;
      }
            
      const fles::Timeslice& ts = *timeslice;
      auto tsIndex = ts.index();
      if( (tsIndex != (fTSNumber+1)) &&( fTSNumber != 0) ) {
        LOG(WARNING) << "Missed Timeslices. Old TS Number was " << fTSNumber 
                     << " New TS Number is " << tsIndex << FairLogger::endl;
      }
      fTSNumber=tsIndex;

      if ( 0 ==fTSNumber%10000 ) {
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
          LOG(DEBUG) << "Could not find unpacker for system id 0x" 
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

Int_t CbmFlibTestSource::GetNextEvent()
{
 const Int_t AddrMask=0x0001FFFF;
 Int_t nDigi=0;
 Bool_t bOut=kFALSE;
 Double_t fTimeBufferOut=0.;
 //Double_t dTLast=0.;
 while (!bOut)
 {
  fTimeBufferOut = fBuffer->GetTimeLast(); 
  LOG(DEBUG) << "Buffer has " << fBuffer->GetSize() << " entries and contains data from " 
            << std::setprecision(9) << std::fixed 
            << static_cast<Double_t>(fBuffer->GetTimeFirst()) * 1.e-9 << " to "
            << std::setprecision(9) << std::fixed 
            << static_cast<Double_t>(fBuffer->GetTimeLast()) * 1.e-9 << " s" << FairLogger::endl;

  if(fTimeBufferOut - fBuffer->GetTimeFirst()<1.E6) {fBufferFillNeeded=kTRUE; return 1;} // store 1 msec of data in memory

  CbmDigi* digi = fBuffer->GetNextData(fTimeBufferOut);
  if (NULL == digi)         {fBufferFillNeeded=kTRUE; return 1;}

  LOG(DEBUG) << "Buffer has " << fBuffer->GetSize() << " entries left for digitime = " <<digi->GetTime()<< FairLogger::endl;

  Double_t dTEnd = digi->GetTime() + fdMaxDeltaT;

  if(dTEnd > fTimeBufferOut)  {
    LOG(WARNING) <<Form("Remaining buffer < %f with %d entries is not sufficient for digi ending at %f -> skipped ",
			fTimeBufferOut, fBuffer->GetSize(), dTEnd ) 
		 <<  FairLogger::endl; 
    fBufferFillNeeded=kTRUE; return 1;
  }
  
  Bool_t bDet[fiReqDigiAddr.size()];
  for(Int_t i=0; i<fiReqDigiAddr.size(); i++) bDet[i]=kFALSE; //initialize

  nDigi=0;
  while(digi) { // build digi array
    if (nDigi == vdigi.size()) vdigi.resize(nDigi+100); 
    vdigi[nDigi++]=digi;
    for(Int_t i=0; i<fiReqDigiAddr.size(); i++)
    if( (digi->GetAddress() & AddrMask) == fiReqDigiAddr[i]) bDet[i]=kTRUE;
    //if(bOut) LOG(INFO)<<Form("Found 0x%08x, Req 0x%08x ", digi->GetAddress(), fiReqDigiAddr)<<FairLogger::endl;
    digi = fBuffer->GetNextData(dTEnd);
  }

  LOG(DEBUG) << nDigi << " digis associated to dTEnd = " <<dTEnd<< FairLogger::endl;
  if( fiReqDigiAddr.size() > 1)
    LOG(DEBUG) << "Found Req coinc in event with " <<nDigi << " digis, dTEnd = " <<dTEnd<< FairLogger::endl;

  //dTLast = vdigi[nDigi-1]->GetTime();

  for(Int_t i=0; i<fiReqDigiAddr.size(); i++) 
    if(bDet[i]==kFALSE) break;
    else if( i == fiReqDigiAddr.size()-1 ) bOut=kTRUE;

  if(fiReqDigiAddr.size()==0) bOut=kTRUE;

  for(Int_t iDigi=0; iDigi<nDigi; iDigi++){
    digi=vdigi[iDigi];
    Int_t detId = digi->GetSystemId();
    Int_t flibId = fDetectorSystemMap[detId];
    LOG(DEBUG1) << "Digi has system ID " << detId 
               << " which maps to FlibId "<< flibId
              //<< ", T "<<digi->GetTime()<<" < "<<dTEnd
              << FairLogger::endl;
    std::map<Int_t, CbmTSUnpack*>::iterator it=fUnpackers.find(flibId);
    if (it == fUnpackers.end()) {
      LOG(ERROR) << "Skipping digi with unknown id " 
                 << detId << FairLogger::endl;
      continue;
    } else {
      //nDigi++;
      //LOG(DEBUG) << "Found unpacker " <<  it->second << FairLogger::endl;
      if(bOut)      it->second->FillOutput(digi);
      else          digi->Delete();
    }
    //    digi = fBuffer->GetNextData(dTEnd);
  }; 
  vdigi.clear();
 }  // end of bOut condition
 LOG(DEBUG) << "Buffer has " << fBuffer->GetSize() << " entries left, "
            << nDigi <<" digis in current event. "<< FairLogger::endl;
 //( fBuffer->GetSize() <= 100000 ) ? fBufferFillNeeded=kTRUE : fBufferFillNeeded=kFALSE; 
 //((dTLast + 1.E10) >  fTimeBufferOut) ? fBufferFillNeeded=kTRUE : fBufferFillNeeded=kFALSE;  // store 10 sec in memory 
 return 0;
}

void CbmFlibTestSource::AddReqDigiAddr(Int_t iAddr)
{
  Int_t iNReq=fiReqDigiAddr.size();
  fiReqDigiAddr.resize(iNReq+1); // hopefully the old entries are preserved ...
  fiReqDigiAddr[iNReq]=iAddr;
}
ClassImp(CbmFlibTestSource)
