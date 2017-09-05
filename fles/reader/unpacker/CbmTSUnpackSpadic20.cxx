// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic20                     -----
// -----                    Created 07.11.2014 by P. Munkes                -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmTSUnpackSpadic20.h"

#include "CbmSpadicRawMessage.h"

#include "TimesliceReader20.hpp"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iostream>

CbmTSUnpackSpadic20::CbmTSUnpackSpadic20 () :
    CbmTSUnpack (), fSpadicRaw (new TClonesArray ("CbmSpadicRawMessage", 10)), fEpochMarkerArray (), fPreviousEpochMarkerArray (), fSuperEpochArray (), fEpochMarker (
	0), fSuperEpoch (0), fNrExtraneousSamples
      { 0 }
{}

CbmTSUnpackSpadic20::~CbmTSUnpackSpadic20 ()
{
  LOG(INFO) << "Number of extraneous Samples " << fNrExtraneousSamples
	       << FairLogger::endl;
}

Bool_t
CbmTSUnpackSpadic20::Init ()
{
  LOG(INFO) << "Initializing" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance ();
  if (ioman == NULL)
    {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
    }
  ioman->Register ("SpadicRawMessage", "spadic raw data", fSpadicRaw, kTRUE);

  return kTRUE;
}

Bool_t
CbmTSUnpackSpadic20::DoUnpack (const fles::Timeslice& ts, size_t component)
{
  // compare to: https://github.com/spadic/spadic10-software/blob/master/lib/message/message.h
  // or fles/spadic/message/wrap/cpp/message.cpp
  LOG(DEBUG) << "Unpacking Spadic Data" << FairLogger::endl;

  spadic::TimesliceReader20 r;
  Int_t counter = 0;

  r.add_component (ts, component);

  for (uint16_t addr : r.sources ())
    {
      //    auto spadicLink=(addr);
      int address=2*(r.get_link(addr)/0x100)/8+(r.get_link(addr)/0x100)%8;
      //LOG(INFO) <<std::hex << "---- reader " << addr << " ----" << r.get_link(addr) << "--------------" << ts.descriptor(component,0).eq_id<<std::dec << FairLogger::endl;

      //LOG(INFO) << "---- reader " << std::hex << addr <<FairLogger::endl;
      while (std::unique_ptr<spadic::Message> mp = r.get_message (addr))
	{
	  if (gLogger->IsLogNeeded (DEBUG))
	    {
	      print_message (*mp);
	    }
	  counter++;
	  Int_t link = ts.descriptor(component, 0).eq_id-0xaa00;
	  //Int_t address = addr;
	  /*
	   if ( mp->is_valid() ){
	   LOG(DEBUG) << counter << " This message is valid" << FairLogger::endl;
	   } else {
	   LOG(ERROR) << counter <<  " This message is not valid" << FairLogger::endl;
	   continue;
	   }
	   */
	  //std::cout << std::hex << address << std::dec <<std::endl;
	  Bool_t isInfo (false), isHit (false), isEpoch (false),
	      isEpochOutOfSynch (false), isOverflow (false), isHitAborted (
		  false), isStrange (false);
	  if (mp->is_epoch_out_of_sync ())
	    {
	      isEpochOutOfSynch = true;
	      FillEpochInfo (link, address, mp->epoch_count ());
	      GetEpochInfo (link, address);
	      Int_t triggerType = -1;
	      Int_t infoType = -1;
	      Int_t stopType = -1;
	      Int_t groupId = mp->group_id ();
	      Int_t channel = mp->channel_id ();
	      Int_t time = -1;
	      Int_t bufferOverflowCounter = 0;
	      Int_t samples = 1;
	      Int_t* sample_values = new Int_t[samples];
	      sample_values[0] = -256;
	      LOG(DEBUG) << counter << " This is an out of sync Epoch Marker"
		  << FairLogger::endl;
	      LOG(DEBUG) << "   TimeStamp: " << mp->timestamp ()
				<< FairLogger::endl;
	      LOG(DEBUG) << "   Channel:   " << mp->channel_id ()
				<< FairLogger::endl;
	      LOG(DEBUG) << "   Group:     " << mp->group_id ()
				<< FairLogger::endl;
	      new ((*fSpadicRaw)[fSpadicRaw->GetEntriesFast ()]) CbmSpadicRawMessage (
		  link, address, channel, fEpochMarker, time, fSuperEpoch,
		  triggerType, infoType, stopType, groupId,
		  bufferOverflowCounter, samples, sample_values, isHit, isInfo,
		  isEpoch, isEpochOutOfSynch, isHitAborted, isOverflow,
		  isStrange);
	      delete[] sample_values;
	    }
	  else if (mp->is_epoch_marker ())
	    {
	      LOG(DEBUG) << counter << " This is an Epoch Marker"
		  << FairLogger::endl;
	      isEpoch = true;
	      FillEpochInfo (link, address, mp->epoch_count ());
	      GetEpochInfo (link, address);
	      Int_t triggerType = -1;
	      Int_t infoType = -1;
	      Int_t stopType = -1;
	      Int_t groupId = mp->group_id ();
	      Int_t channel = mp->channel_id ();
	      Int_t time = -1;
	      Int_t bufferOverflowCounter = 0;
	      Int_t samples = 1;
	      Int_t* sample_values = new Int_t[samples];
	      sample_values[0] = -256;
	      new ((*fSpadicRaw)[fSpadicRaw->GetEntriesFast ()]) CbmSpadicRawMessage (
		  link, address, channel, fEpochMarker, time, fSuperEpoch,
		  triggerType, infoType, stopType, groupId,
		  bufferOverflowCounter, samples, sample_values, isHit, isInfo,
		  isEpoch, isEpochOutOfSynch, isHitAborted, isOverflow,
		  isStrange);
	      delete[] sample_values;
	    }
	  else if (mp->is_buffer_overflow ())
	    {
	      LOG(DEBUG) << counter << " This is a buffer overflow message"
		  << FairLogger::endl;
	      isOverflow = true;
	      GetEpochInfo (link, address);
	      Int_t triggerType = -1;
	      Int_t infoType = -1;
	      Int_t stopType = -1;
	      Int_t groupId = mp->group_id ();
	      Int_t channel = mp->channel_id ();
	      Int_t time = mp->timestamp ();
	      Int_t bufferOverflowCounter =
		  static_cast<Int_t> (mp->buffer_overflow_count ());
	      Int_t samples = 1;
	      Int_t* sample_values = new Int_t[samples];
	      sample_values[0] = -256;
	      new ((*fSpadicRaw)[fSpadicRaw->GetEntriesFast ()]) CbmSpadicRawMessage (
		  link, address, channel, fEpochMarker, time, fSuperEpoch,
		  triggerType, infoType, stopType, groupId,
		  bufferOverflowCounter, samples, sample_values, isHit, isInfo,
		  isEpoch, isEpochOutOfSynch, isHitAborted, isOverflow,
		  isStrange);
	      delete[] sample_values;
	    }
	  else if (mp->is_info ())
	    {
	      LOG(DEBUG) << counter << " This is a info message"
		  << FairLogger::endl;
	      isInfo = true;
	      GetEpochInfo (link, address);

	      Int_t triggerType = -1;
	      Int_t infoType = static_cast<Int_t> (mp->info_type ());
	      Int_t stopType = -1;
	      Int_t groupId = mp->group_id ();
	      Int_t channel = mp->channel_id ();
	      Int_t time = mp->timestamp ();
	      Int_t bufferOverflowCounter = 0; //mp->buffer_overflow_count();// should be now obsolete
	      Int_t samples = 1;
	      Int_t* sample_values = new Int_t[samples];
	      sample_values[0] = -256;
	      new ((*fSpadicRaw)[fSpadicRaw->GetEntriesFast ()]) CbmSpadicRawMessage (
		  link, address, channel, fEpochMarker, time, fSuperEpoch,
		  triggerType, infoType, stopType, groupId,
		  bufferOverflowCounter, samples, sample_values, isHit, isInfo,
		  isEpoch, isEpochOutOfSynch, isHitAborted, isOverflow,
		  isStrange);
	      delete[] sample_values;
	    }
	  else if (mp->is_hit ())
	    {
	      LOG(DEBUG) << counter << " This is a hit message"
		  << FairLogger::endl;
	      isHit = true;
	      GetEpochInfo (link, address);
	      Int_t triggerType = static_cast<Int_t> (mp->hit_type ());
	      Int_t stopType = static_cast<Int_t> (mp->stop_type ());
	      Int_t time = mp->timestamp ();
	      Int_t infoType = -1;
	      Int_t groupId = mp->group_id ();
	      Int_t bufferOverflowCounter = 0;
	      Int_t samples = mp->samples ().size ();
	      if (samples > 32)
		{
		  fNrExtraneousSamples++;
		  samples = 32; //Suppress extraneous Samples, which cannot (!) occur in Raw Data Stream.
		}
	      Int_t* sample_values = new Int_t[samples];
	      Int_t channel = mp->channel_id ();
	      Int_t counter1 = 0;
	      for (auto x : mp->samples ())
		{
		  sample_values[counter1] = x;
		  ++counter1;
		}
	      //}
	      new ((*fSpadicRaw)[fSpadicRaw->GetEntriesFast ()]) CbmSpadicRawMessage (
		  link, address, channel, fEpochMarker, time, fSuperEpoch,
		  triggerType, infoType, stopType, groupId,
		  bufferOverflowCounter, samples, sample_values, isHit, isInfo,
		  isEpoch, isEpochOutOfSynch, isHitAborted, isOverflow,
		  isStrange);
	      //++counter;
	      delete[] sample_values;
	    }
	  else if (mp->is_hit_aborted ())
	    {
	      LOG(DEBUG) << counter << " This is a hit message was aborted"
		  << FairLogger::endl;
	      isHitAborted = true;
	      GetEpochInfo (link, address);
	      Int_t triggerType = -1;
	      Int_t stopType = -1;
	      Int_t time = mp->timestamp ();
	      Int_t infoType = -1;
	      Int_t groupId = -1; //mp->group_id();//???
	      Int_t bufferOverflowCounter = 0;
	      Int_t samples = 1;
	      Int_t* sample_values = NULL;
	      Int_t channel = mp->channel_id ();
	      infoType = static_cast<Int_t> (mp->info_type ()); // should here be stoptype instead???
	      sample_values = new Int_t[samples];
	      sample_values[0] = -256;
	      new ((*fSpadicRaw)[fSpadicRaw->GetEntriesFast ()]) CbmSpadicRawMessage (
		  link, address, channel, fEpochMarker, time, fSuperEpoch,
		  triggerType, infoType, stopType, groupId,
		  bufferOverflowCounter, samples, sample_values, isHit, isInfo,
		  isEpoch, isEpochOutOfSynch, isHitAborted, isOverflow,
		  isStrange);
	      //++counter;
	      delete[] sample_values;

	    }
	  else
	    {
	      isStrange = true;
	      GetEpochInfo(link, address);
	      Int_t triggerType = -1;
	      Int_t stopType = -1;
	      Int_t time = -1; //mp->timestamp();
	      Int_t infoType = -1;
	      Int_t groupId = -1; //mp->group_id();
	      Int_t bufferOverflowCounter = 0;
	      Int_t samples = 1;
	      Int_t* sample_values = NULL;
	      Int_t channel = -1; //mp->channel_id();
	      sample_values = new Int_t[samples];
	      sample_values[0] = -256;
	      new ((*fSpadicRaw)[fSpadicRaw->GetEntriesFast ()]) CbmSpadicRawMessage (
		  link, address, channel, fEpochMarker, time, fSuperEpoch,
		  triggerType, infoType, stopType, groupId,
		  bufferOverflowCounter, samples, sample_values, isHit, isInfo,
		  isEpoch, isEpochOutOfSynch, isHitAborted, isOverflow,
		  isStrange);
	      //++counter;
	      delete[] sample_values;

	      LOG(DEBUG) << counter
		  << " This message type is not hit, info, epoch or overflow and will not be stored in the TClonesArray"
		  << FairLogger::endl;
	      LOG(DEBUG) << " valide:" << mp->is_valid () << " epoch marker:"
		  << fEpochMarker << " super epoch marker:"
		  << fSuperEpoch << " time:" << time << " link:"
		  << link << " address:" << address
		  << FairLogger::endl;
	      LOG(DEBUG) << "Channel ID:" << mp->channel_id ()
				<< FairLogger::endl;
	      //if ( mp->is_hit_aborted() )
	      //LOG(INFO) << "hit is aborted" << FairLogger::endl;
	      //if ( mp->is_hit() )
	      LOG(DEBUG) << "GroupID:" << mp->group_id () << "hit: triggerType:"
		  << static_cast<Int_t> (mp->hit_type ())
		  << " stopType:"
		  << static_cast<Int_t> (mp->stop_type ())
		  << " Nr.of samples:" << mp->samples ().size ()
		  << FairLogger::endl;

	      //if ()
	      for (auto x : mp->samples ())
		{
		  LOG(DEBUG) << " " << x;
		}
	      LOG(DEBUG) << FairLogger::endl;
	      //if (mp->is_info())
	      LOG(DEBUG) << "InfoType:" << static_cast<Int_t> (mp->info_type ())
				<< FairLogger::endl;
	      //if ()
	      LOG(DEBUG) << "Nr. of overflows:"
		  << static_cast<Int_t> (mp->buffer_overflow_count ())
		  << FairLogger::endl;
	      //print_message(mp);
	    }
	}
    }
  return kTRUE;
}

void
CbmTSUnpackSpadic20::print_message (const spadic::Message& m)
{
  LOG(INFO) << "v: " << (m.is_valid () ? "o" : "x");
  LOG(DEBUG) << " / gid: " << static_cast<int> (m.group_id ());
  LOG(DEBUG) << " / chid: " << static_cast<int> (m.channel_id ());
  if (m.is_hit ())
    {
      LOG(DEBUG) << " / ts: " << m.timestamp ();
      LOG(DEBUG) << " / samples (" << m.samples ().size () << "):";
      for (auto x : m.samples ())
	{
	  LOG(DEBUG) << " " << x;
	}
      LOG(DEBUG) << FairLogger::endl;
    }
  else
    {
      if (m.is_epoch_marker ())
	{
	  LOG(DEBUG) << " This is an Epoch Marker" << FairLogger::endl;
	}
      else if (m.is_epoch_out_of_sync ())
	{
	  LOG(INFO) << " This is an out of sync Epoch Marker"
		       << FairLogger::endl;
	}
      else
	{
	  LOG(INFO) << " This is not known" << FairLogger::endl;
	}
    }
}

void
CbmTSUnpackSpadic20::FillEpochInfo (Int_t link, Int_t addr, Int_t epoch_count)
{


      /* Check for repeated Epoch Messages, as the repeated Microslices
       are not captured by the CbmTsUnpacker. This is to ensure the 
       linearity of the GetFullTime() method.
       */
      // dirty workaround in the following line: only sufficiently big backsteps of Epoch will trigger an upcount of SuperEpoch. this avoids a high sensitivity of the SuperEpoch counting method to overlaps of some epochs, before the overlaps are understood better
      if (epoch_count < (fEpochMarkerArray[std::pair<int,int>(link,addr)] - 3000))
	{
	  if (SuppressMultipliedEpochMessages)
	    if (epoch_count != fPreviousEpochMarkerArray[std::pair<int,int>(link,addr)])
	      {
		fSuperEpochArray[std::pair<int,int>(link,addr)]++;
	      }
	    else
	      {
		LOG(ERROR) << "Multiply repeated Epoch Messages at Super Epoch "
			      << fSuperEpoch << " Epoch " << epoch_count
			      << " for AFCK" << link << "_Spadic" << addr
			      << FairLogger::endl;
	      }
	  else
	    fSuperEpochArray[std::pair<int,int>(link,addr)]++;

	  LOG(DEBUG) << "Overflow of EpochCounter for Afck" << link
			<< "_Spadic" << addr << FairLogger::endl;
	}
      else if ((epoch_count - fEpochMarkerArray[std::pair<int,int>(link,addr)]) != 1)
	{
	  LOG(INFO) <<std::hex<< "Missed epoch counter for Afck" << link
		       << "_Spadic" << addr <<std::dec<< FairLogger::endl;
	}
      else if (epoch_count == fEpochMarkerArray[std::pair<int,int>(link,addr)])
	{
	  LOG(ERROR) << "Identical Epoch Counters for Afck" << link
			<< "_Spadic" << addr << FairLogger::endl;
	}
      fPreviousEpochMarkerArray[std::pair<int,int>(link,addr)] =
	  fEpochMarkerArray[std::pair<int,int>(link,addr)];
      fEpochMarkerArray[std::pair<int,int>(link,addr)] = epoch_count;


}

void
CbmTSUnpackSpadic20::GetEpochInfo (Int_t link, Int_t addr)
{
  fEpochMarker = fEpochMarkerArray[std::pair<int,int>(link,addr)];
  fSuperEpoch = fSuperEpochArray[std::pair<int,int>(link,addr)];
}

void
CbmTSUnpackSpadic20::Reset ()
{
  fSpadicRaw->Clear ();
}

void
CbmTSUnpackSpadic20::Finish ()
{
  for (auto it = fSuperEpochArray.begin (); it != fSuperEpochArray.end (); it++)
    {
      LOG(DEBUG) << "There have been " << it->second << " SuperEpochs for Afck"
		    << it->first.first << "_Spadic" << it->first.second
		    << " in this file" << FairLogger::endl;

    }

}

/*
 void CbmTSUnpackSpadic20::Register()
 {
 }
 */

ClassImp(CbmTSUnpackSpadic20)

