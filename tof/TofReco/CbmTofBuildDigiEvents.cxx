/**
 * @file
 * @author Christian Simon <csimon@physi.uni-heidelberg.de>
 * @since 2018-05-31
 */

#include "CbmTofBuildDigiEvents.h"


#include "CbmTimeSlice.h"
#include "CbmTofDigiExp.h"
#include "CbmMatch.h"
//#include "CbmTofDef.h" TODO

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairFileSource.h"

#include "TClonesArray.h"
#include "TMath.h"

#include <algorithm>


// ---------------------------------------------------------------------------
CbmTofBuildDigiEvents::CbmTofBuildDigiEvents() :
	FairTask("TofBuildDigiEvents"),
  fFileSource(NULL),
  fTimeSliceHeader(NULL),
  fTofTimeSliceDigis(NULL),
  fTofEventDigis(NULL),
  fdEventWindow(0.),
  fNominalTriggerCounterMultiplicity(),
  fiTriggerMultiplicity(0),
  fbPreserveMCBacklinks(kFALSE),
  fbMCEventBuilding(kFALSE),
  fdEventStartTime(DBL_MIN),
  fCounterMultiplicity(),
  fdIdealEventWindow(1000.),
  fProcessedIdealEvents(),
  fIdealEventStartTimes(),
  fIdealEventDigis(),
  fiNEvents(0),
  fdDigiToTOffset(0.),
  fInactiveCounterSides()
{
}
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
CbmTofBuildDigiEvents::~CbmTofBuildDigiEvents()
{
}
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
void CbmTofBuildDigiEvents::Exec(Option_t*)
{
  if(fbMCEventBuilding)
  {
    ProcessIdealEvents(fTimeSliceHeader->GetStartTime());

    for(Int_t iDigi = 0; iDigi < fTofTimeSliceDigis->GetEntriesFast(); iDigi++)
    {
      CbmTofDigiExp* tDigi = dynamic_cast<CbmTofDigiExp*>(fTofTimeSliceDigis->At(iDigi));

      Int_t iDigiAddress = tDigi->GetAddress();
      Double_t dDigiTime = tDigi->GetTime();
      Double_t dDigiToT  = tDigi->GetTot();


      Int_t iNMCLinks = tDigi->GetMatch()->GetNofLinks();

      for(Int_t iLink = 0; iLink < iNMCLinks; iLink++)
      {
        const CbmLink& tLink = tDigi->GetMatch()->GetLink(iLink);

        // Only consider digis that contain at least one contribution from a
        // primary source signal. If the digi contains primary signal contributions
        // from more than one MC event, assign the digi to all MC events found.
        // TODO: Replace '0' by 'tof::signal_SourcePrimary' upon inclusion of
        //       'tof/TofTools/CbmTofDef.h' into trunk!
        if(0 == tLink.GetUniqueID())
        {
          std::pair<Int_t, Int_t> EventID(tLink.GetFile(), tLink.GetEntry());

          // The MC event is already known.
		      if(fIdealEventStartTimes.find(EventID) != fIdealEventStartTimes.end())
          {
            auto & DigiVector = fIdealEventDigis.at(EventID);

            if(fbPreserveMCBacklinks)
            {
              // deep copy construction including 'CbmDigi::fMatch'
              DigiVector.push_back(new CbmTofDigiExp(*tDigi));
            }
            else
            {
              // shallow construction excluding 'CbmDigi::fMatch'
              DigiVector.push_back(new CbmTofDigiExp(iDigiAddress, dDigiTime, dDigiToT));
            }
		      }
          // The MC event is not known yet.
		      else
          {
            // Make sure that a late digi from an event that has already been
            // processed and written to disk (i.e. the time difference to the
            // earliest digi in the same event is larger than 'fdIdealEventWindow')
            // does not trigger separate event processing for itself only
            // (and possibly a few additional latecomers).
            if(fProcessedIdealEvents.find(EventID) == fProcessedIdealEvents.end())
            {
              fIdealEventStartTimes.emplace(EventID, dDigiTime);
              fIdealEventDigis.emplace(EventID, std::vector<CbmTofDigiExp*>());

              auto & DigiVector = fIdealEventDigis.at(EventID);

              if(fbPreserveMCBacklinks)
              {
                // deep copy construction including 'CbmDigi::fMatch'
                DigiVector.push_back(new CbmTofDigiExp(*tDigi));
              }
              else
              {
                // shallow construction excluding 'CbmDigi::fMatch'
                DigiVector.push_back(new CbmTofDigiExp(iDigiAddress, dDigiTime, dDigiToT));
              }
            }
          }
        }
      }
    }
  }
  else
  {
    for(Int_t iDigi = 0; iDigi < fTofTimeSliceDigis->GetEntriesFast(); iDigi++)
    {
      CbmTofDigiExp* tDigi = dynamic_cast<CbmTofDigiExp*>(fTofTimeSliceDigis->At(iDigi));

      Int_t iDigiModuleType   = tDigi->GetType();
      Int_t iDigiModuleIndex  = tDigi->GetSm();
      Int_t iDigiCounterIndex = tDigi->GetRpc();
      Int_t iDigiCounterSide  = tDigi->GetSide();
      Int_t iDigiAddress      = tDigi->GetAddress();
      Double_t dDigiTime      = tDigi->GetTime();
      Double_t dDigiToT       = tDigi->GetTot();


      if(dDigiTime - fdEventStartTime > fdEventWindow)
      {
        std::map<std::tuple<Int_t, Int_t, Int_t>, UChar_t> ActualTriggerCounterMultiplicity;

        std::set_intersection(fCounterMultiplicity.begin(), fCounterMultiplicity.end(),
                              fNominalTriggerCounterMultiplicity.begin(), fNominalTriggerCounterMultiplicity.end(),
                              std::inserter(ActualTriggerCounterMultiplicity, ActualTriggerCounterMultiplicity.begin()));

        if(ActualTriggerCounterMultiplicity.size() >= fiTriggerMultiplicity)
        {
          FairRootManager::Instance()->Fill();
          fiNEvents++;
          fTofEventDigis->Delete();
        }
        else
        {
          fTofEventDigis->Delete();
        }

        fCounterMultiplicity.clear();

        fdEventStartTime = dDigiTime;
      }


      fCounterMultiplicity[std::make_tuple(iDigiModuleType, iDigiModuleIndex, iDigiCounterIndex)] |= 1 << iDigiCounterSide;

      auto CounterSideTuple = std::make_tuple(iDigiModuleType, iDigiModuleIndex, iDigiCounterIndex, iDigiCounterSide);

      if(fInactiveCounterSides.find(CounterSideTuple) == fInactiveCounterSides.end())
      {
        CbmTofDigiExp* tEventDigi(NULL);

        if(fbPreserveMCBacklinks)
        {
          // deep copy construction including 'CbmDigi::fMatch'
          tEventDigi = new((*fTofEventDigis)[fTofEventDigis->GetEntriesFast()]) CbmTofDigiExp(*tDigi);
        }
        else
        {
          // shallow construction excluding 'CbmDigi::fMatch'
          tEventDigi = new((*fTofEventDigis)[fTofEventDigis->GetEntriesFast()]) CbmTofDigiExp(iDigiAddress, dDigiTime, dDigiToT);
        }

        tEventDigi->SetTot(tEventDigi->GetTot() + fdDigiToTOffset);
      }
    }
  }

}
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
InitStatus CbmTofBuildDigiEvents::Init()
{
  if(!FairRootManager::Instance())
  {
    LOG(ERROR)<<"FairRootManager not found."<<FairLogger::endl;
    return kFATAL;
  }

  fFileSource = dynamic_cast<FairFileSource*>(FairRootManager::Instance()->GetSource());
  if(!fFileSource)
  {
    LOG(ERROR)<<"Could not get pointer to FairFileSource."<<FairLogger::endl;
    return kFATAL;
  }

  fTimeSliceHeader = dynamic_cast<CbmTimeSlice*>(FairRootManager::Instance()->GetObject("TimeSlice."));
  if(!fTimeSliceHeader)
  {
    LOG(ERROR)<<"Could not retrieve branch 'TimeSlice.' from FairRootManager."<<FairLogger::endl;
    return kFATAL;
  }

  fTofTimeSliceDigis = dynamic_cast<TClonesArray*>(FairRootManager::Instance()->GetObject("TofDigiExp"));
  if(!fTofTimeSliceDigis)
  {
    LOG(ERROR)<<"Could not retrieve branch 'TofDigiExp' from FairRootManager."<<FairLogger::endl;
    return kFATAL;
  }

  fTofEventDigis = new TClonesArray("CbmTofDigiExp", 100);

  FairRootManager::Instance()->Register("CbmTofDigiExp", "TOF event digis", fTofEventDigis, IsOutputBranchPersistent("CbmTofDigiExp"));


  if(0. >= fdEventWindow)
  {
    fbMCEventBuilding = kTRUE;
  }

  fiTriggerMultiplicity = TMath::Abs(fiTriggerMultiplicity);

  if(fNominalTriggerCounterMultiplicity.size() < fiTriggerMultiplicity)
  {
    fiTriggerMultiplicity = fNominalTriggerCounterMultiplicity.size();
  }

  return kSUCCESS;
}
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
void CbmTofBuildDigiEvents::Finish()
{
  if(fbMCEventBuilding)
  {
    // With O(s) of off-spill noise (not eligible for MC event building) stored
    // in several timeslices (the processing of each causing an 'Exec' call by
    // the framework) following the final spill there should not be any digis
    // related to MC events left for processing at this point.  
    ProcessIdealEvents(DBL_MAX);
  }
  else
  {
    // The remaining digis in the buffer do not cover a time interval of
    // 'fdEventWindow' and, in consequence, do not qualify for event building.
    fTofEventDigis->Delete();
    fCounterMultiplicity.clear();
  }
}
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
void CbmTofBuildDigiEvents::SetTriggerCounter(Int_t iModuleType, Int_t iModuleIndex, Int_t iCounterIndex, Int_t iNCounterSides)
{
  fNominalTriggerCounterMultiplicity.emplace(std::make_tuple(iModuleType, iModuleIndex, iCounterIndex), (1 == iNCounterSides) ? 1 : 3);
}
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
void CbmTofBuildDigiEvents::ProcessIdealEvents(Double_t dProcessingTime)
{
  for(auto itEvent = fIdealEventStartTimes.cbegin(); itEvent != fIdealEventStartTimes.cend(); )
  {
    auto EventID = itEvent->first;
    Double_t dEventStartTime = itEvent->second;

    if(dProcessingTime - dEventStartTime > fdIdealEventWindow)
    {
      for(auto & tDigi : fIdealEventDigis.at(EventID))
      {
        Int_t iDigiModuleType   = tDigi->GetType();
        Int_t iDigiModuleIndex  = tDigi->GetSm();
        Int_t iDigiCounterIndex = tDigi->GetRpc();
        Int_t iDigiCounterSide  = tDigi->GetSide();

        fCounterMultiplicity[std::make_tuple(iDigiModuleType, iDigiModuleIndex, iDigiCounterIndex)] |= 1 << iDigiCounterSide;

        auto CounterSideTuple = std::make_tuple(iDigiModuleType, iDigiModuleIndex, iDigiCounterIndex, iDigiCounterSide);

        if(fInactiveCounterSides.find(CounterSideTuple) == fInactiveCounterSides.end())
        {
          // deep copy construction including 'CbmDigi::fMatch' (only if already deep-copied in 'Exec')
          CbmTofDigiExp* tEventDigi = new((*fTofEventDigis)[fTofEventDigis->GetEntriesFast()]) CbmTofDigiExp(*tDigi);
          tEventDigi->SetTot(tEventDigi->GetTot() + fdDigiToTOffset);
        }

        delete tDigi;
      }

      std::map<std::tuple<Int_t, Int_t, Int_t>, UChar_t> ActualTriggerCounterMultiplicity;

      std::set_intersection(fCounterMultiplicity.begin(), fCounterMultiplicity.end(),
                            fNominalTriggerCounterMultiplicity.begin(), fNominalTriggerCounterMultiplicity.end(),
                            std::inserter(ActualTriggerCounterMultiplicity, ActualTriggerCounterMultiplicity.begin()));

      if(ActualTriggerCounterMultiplicity.size() >= fiTriggerMultiplicity)
      {
        FairRootManager::Instance()->Fill();
        fiNEvents++;
        fTofEventDigis->Delete();
      }
      else
      {
        fTofEventDigis->Delete();
      }

      fCounterMultiplicity.clear();
      fIdealEventDigis.erase(EventID);
      fProcessedIdealEvents.emplace(EventID);

      itEvent = fIdealEventStartTimes.erase(itEvent);
    }
    else
    {
      ++itEvent;
    }
  }
}
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
void CbmTofBuildDigiEvents::SetIgnoreCounterSide(Int_t iModuleType, Int_t iModuleIndex, Int_t iCounterIndex, Int_t iCounterSide)
{
  fInactiveCounterSides.emplace(std::make_tuple(iModuleType, iModuleIndex, iCounterIndex, iCounterSide));
}
// ---------------------------------------------------------------------------


ClassImp(CbmTofBuildDigiEvents)
