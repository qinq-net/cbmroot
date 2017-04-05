/*
 * CbmStsClusterFinderModule.cxx
 *
 *  Created on: 01.04.2017
 *      Author: vfriese
 */

#include "CbmStsClusterFinderModule.h"

#include <cassert>
#include "TClonesArray.h"
#include "FairLogger.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"


ClassImp(CbmStsClusterFinderModule)

Int_t CbmStsClusterFinderModule::fgFinishCluster = 0;
Int_t CbmStsClusterFinderModule::fgFinishClusterEnd = 0;

CbmStsClusterFinderModule::CbmStsClusterFinderModule() :
  TNamed(), fSize(0), fDeltaT(0.), fModule(NULL), fClusters(NULL),
  fIndex(), fTime()
{
}


CbmStsClusterFinderModule::CbmStsClusterFinderModule(Int_t nChannels,
                                                     Double_t deltaT,
                                                     const char* name,
                                                     CbmStsModule* module,
                                                     TClonesArray* output) :
  TNamed(name, "cluster finder module"),
  fSize(nChannels),
  fDeltaT(deltaT),
  fModule(module),
  fClusters(output),
  fIndex(fSize),
  fTime(fSize)
{
 // Number of channels must fit in UShort_t
 assert ( nChannels > 0 && nChannels <= 65536);
}


CbmStsClusterFinderModule::~CbmStsClusterFinderModule() {
  // TODO Auto-generated destructor stub
}




// ----- Process an input digi   -------------------------------------------
void CbmStsClusterFinderModule::ProcessDigi(UShort_t channel, Double_t time,
                                            Int_t index) {

  // Check for matching digi in the same channel (should not happen)
  assert ( ! CheckChannel(channel, time) );

  // Check for matching digi in the left neighbour channel
  Bool_t leftNeighbour =  ( channel != 0 && channel != fSize/2 &&
       CheckChannel(channel-1, time) );

  // Check for matching digi in the right neighbour channel
  Bool_t rightNeighbour = ( channel != fSize/2 - 1 && channel != fSize - 1 &&
       CheckChannel(channel+1, time) );

  // Set channel active
  fIndex[channel] = index;
  fTime[channel] = time;

}
// -------------------------------------------------------------------------



// ----- Search for a matching cluster for a given channel   ---------------
Bool_t CbmStsClusterFinderModule::CheckChannel(UShort_t channel,
                                               Double_t time) {

  // No match if no active digi in the channel
  if ( fIndex[channel] == -1 ) return kFALSE;

  // Check timing. Digis should be time-ordered.
  assert( time >= fTime[channel] );

  // Channel is active, but time is not matching: close cluster and return no match.
  if ( time - fTime[channel] > fDeltaT ) {
    FinishCluster(channel);
    fgFinishCluster++;
    return kFALSE;
  }

  // Matching digi found
  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Close a cluster   -----------------------------------------------
void CbmStsClusterFinderModule::FinishCluster(UShort_t channel) {

  // Find start and stop channel of cluster
  UShort_t start = channel;
  UShort_t stop = channel;
  if ( channel < fSize/2 ) {   // front side channels
    while ( start > 0 && fIndex[start-1] > -1 ) start--;
    while ( stop < fSize/2 - 1 && fIndex[stop+1] > -1 ) stop++;
  }
  else  {   // back side channels
    while ( start > fSize/2 && fIndex[start-1] > -1 ) start--;
    while ( stop < fSize - 1 && fIndex[stop+1] > -1 ) stop++;
  }

  // Create a cluster object
  CreateCluster(start, stop);

  // Reset channels
  for (UShort_t iChannel = start; iChannel <= stop; iChannel++) {
    assert( iChannel >= 0 && iChannel <= fSize );
    fIndex[iChannel] = -1;
    fTime[iChannel] = 0.;
  }

}
// -------------------------------------------------------------------------



// -----   Create a cluster object   ---------------------------------------
void CbmStsClusterFinderModule::CreateCluster(UShort_t first,
                                              UShort_t last) {

  // --- No action if no output array specified
  if ( ! fClusters ) return;

  Int_t index = fClusters->GetEntriesFast();
  CbmStsCluster* cluster = new ((*fClusters)[index]) CbmStsCluster();
  for (UShort_t channel = first; channel <= last; channel++) {
    assert( fIndex[channel] > -1 );
    cluster->AddDigi(fIndex[channel]);
  } //#  channels in cluster
  if ( fModule ) cluster->SetAddress(fModule->GetAddress());

}
// -------------------------------------------------------------------------



// -----   Process active clusters   ---------------------------------------
void CbmStsClusterFinderModule::ProcessBuffer() {

  for (UShort_t channel = 0; channel < fSize; channel++) {
    if ( fIndex[channel] == - 1 ) continue;
    FinishCluster(channel);
    fgFinishClusterEnd++;
  }

}
// -------------------------------------------------------------------------



// -----   Reset the channel vectors   -------------------------------------
void CbmStsClusterFinderModule::Reset() {

  fIndex.assign(fSize, -1);
  fTime.assign(fSize, 0.);

}
// -------------------------------------------------------------------------
