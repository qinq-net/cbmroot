/*
 * CbmStsClusterFinderModule.h
 *
 *  Created on: 01.04.2017
 *      Author: vfriese
 */

#ifndef CBMSTSCLUSTERFINDERMODULE_H
#define CBMSTSCLUSTERFINDERMODULE_H 1

#include <vector>
#include "TNamed.h"
#include "CbmStsModule.h"

class TClonesArray;


class CbmStsClusterFinderModule : public TNamed
{
  public:
    CbmStsClusterFinderModule();

    CbmStsClusterFinderModule(Int_t nChannels, Double_t deltaT,
                              const char* name, CbmStsModule* module = NULL,
                              TClonesArray* output = NULL);

    virtual ~CbmStsClusterFinderModule();


    Double_t GetDeltaT() const { return fDeltaT; }

    static Int_t GetNofClusters() { return fgFinishCluster; }
    static Int_t GetNofClustersEnd() { return fgFinishClusterEnd; }

    Int_t GetSize() const { return fSize; }

    /** Process an input digi
     ** @param channel   Channel number
     ** @param time      Digi time [ns]
     ** @param index     Index of digi object in its TClonesArray
     **/
    void ProcessDigi(UShort_t channel, Double_t time, Int_t index);


    /** @brief Process the buffer of active channels
     **
     ** At the end of the time slice / event, the remaining active channels
     ** in the buffers have to be processed.
     **/
    void ProcessBuffer();

    /** Reset the internal bookkeeping **/
    void Reset();




  private:

    UShort_t fSize;         /// Number of channels
    Double_t fDeltaT;       /// Time window for joining digis
    CbmStsModule* fModule;    // Pointer to STS module
    TClonesArray* fClusters;  //! Output array for clusters
    std::vector<Int_t> fIndex;  //! Channel -> digi index
    std::vector<Double_t> fTime;  //! Channel -> digi time

    static Int_t fgFinishCluster;
    static Int_t fgFinishClusterEnd;


    /** Check for a matching digi in a given channel
     ** @param channel  Channel number
     ** @param time     Time [ns]
     ** @value          kTRUE if matching digi found
     **
     ** The digi is considered matching if the time difference between
     ** the time argument and the time of the active digi in the channel
     ** is within the time window defined by the resolution of the module.
     **/
    Bool_t CheckChannel(UShort_t channel, Double_t time);


    /** Close an active cluster
     ** @param channel  Channel number
     **/
    void FinishCluster(UShort_t channel);


    /** Create a cluster object
     ** @param first  First channel of cluster
     ** @param last   Last channel of cluster
     **/
    void CreateCluster(UShort_t first, UShort_t last);


   ClassDef(CbmStsClusterFinderModule, 1);

};

#endif /* CBMSTSCLUSTERFINDERMODULE_H */
