/** @file CbmStsClusterFindeModule.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 05.04.20174
 **/

#ifndef CBMSTSCLUSTERFINDERMODULE_H
#define CBMSTSCLUSTERFINDERMODULE_H 1

#include <vector>
#include "TNamed.h"
#include "CbmStsModule.h"

class TClonesArray;

/** @class CbmStsClusterFinderModule
 ** @brief Class for finding clusters in one STS module
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 05.04.2017
 ** @version 1.0
 **
 ** A cluster is defined by a group of matching digis. Two digis
 ** are considered matching if they are in neighbouring channels
 ** and their time difference is less than the chosen limit.
 **
 ** Clustering is performed in a streaming way, i.e. on adding
 ** of each digi by the method ProcessDigi. If a cluster in the same
 ** or in a neighbour channel is found which does not match the digi,
 ** it is written to the output. This makes the runtime of the
 ** algorithm independent of input data size, but has as a consequence
 ** that the output clusters are not sorted w.r.t. time.
 **
 ** This implementation is suited for modules connected to
 ** double-sided strip sensors. It is assumed that the first half
 ** of module channels is connected to the front side of the sensor,
 ** the second half of the channels to the back side.
 ** Thus, digis in the channel nChannels/2-1 and nChannels/2 are
 ** never connected to a cluster.
 **
 ** In case of strips with stereo angle and cross-connection by a
 ** double-metal layer, the first and last strip on the respective
 ** sensor side are geometric neighbours. Clustering "round the edge"
 ** (connecting the first and last channel for this side) can be
 ** enabled by the methods ConnectEdgeFront and ConnectEdgeBack.
 **
 ** The digis are connected to the cluster in the order left to right,
 ** i.e. with ascending channel number. In case of clustering round
 ** the edge, the channels at the right edge are considered left neighbours
 ** of the first channel, i.e. the cluster starts with high channel
 ** number.
 **/
class CbmStsClusterFinderModule : public TNamed
{
  public:

    /** Default constructor **/
    CbmStsClusterFinderModule();


    /** Constructor with parameters
     ** @param nChannels  Number of channels in the module
     ** @param deltaT     Max. time difference for digis in one cluster
     ** @param name       Class instance name
     ** @param module     Pointer to CbmModule object (in CbmStsSetup)
     ** @param output     Pointer to output array of CbmStsClusters
     */
    CbmStsClusterFinderModule(Int_t nChannels, Double_t deltaT,
                              const char* name, CbmStsModule* module = NULL,
                              TClonesArray* output = NULL);


    /** Destructor **/
    virtual ~CbmStsClusterFinderModule();


    /** @brief Allow connection of first and last channel on back side
     ** @param connect  If kTRUE, allow connection
     **
     ** If this flag is set, the first and last channel of the back side
     ** can be part of the same cluster. This is e.g. the case for finite
     ** stereo angle of the strips with cross-connection by a
     ** double metal layer.
     **/
    void ConnectEdgeBack(Bool_t connect = kTRUE) {
      fConnectEdgeBack = connect;
    }


    /** @brief Allow connection of first and last channel on front side
     ** @param connect  If kTRUE, allow connection
     **
     ** If this flag is set, the first and last channel of the back side
     ** can be part of the same cluster. This is e.g. the case for finite
     ** stereo angle of the strips with cross-connection by a
     ** double metal layer.
      **/
    void ConnectEdgeFront(Bool_t connect = kTRUE) {
      fConnectEdgeFront = connect;
    }


    /** @brief Time difference allowed for digis to belong to one cluster
     ** @return time difference [ns]
     **/
    Double_t GetDeltaT() const { return fDeltaT; }


    /** @brief Get number of channels
     ** @return Number of channels
     **/
    Int_t GetSize() const { return fSize; }


    /** @brief Process the buffer of active channels
     **
     ** At the end of the time slice / event, the remaining active channels
     ** in the buffers have to be processed.
     **/
    void ProcessBuffer();


    /** Process an input digi
     ** @param channel   Channel number
     ** @param time      Digi time [ns]
     ** @param index     Index of digi object in its TClonesArray
     **/
    void ProcessDigi(UShort_t channel, Double_t time, Int_t index);


    /** Reset the internal bookkeeping **/
    void Reset();


  private:

    UShort_t fSize;               /// Number of channels
    Double_t fDeltaT;             /// Time window for joining digis
    Bool_t fConnectEdgeFront;     /// Round-the edge clustering front side
    Bool_t fConnectEdgeBack;      /// Round-the edge clustering back side
    CbmStsModule* fModule;        //! Pointer to STS module
    TClonesArray* fClusters;      //! Output array for clusters
    std::vector<Int_t> fIndex;    //! Channel -> digi index
    std::vector<Double_t> fTime;  //! Channel -> digi time


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


    /** Create a cluster object
     ** @param first  First channel of cluster
     ** @param last   Last channel of cluster
     **/
    void CreateCluster(UShort_t first, UShort_t last);


    /** Close an active cluster
     ** @param channel  Channel number
     **/
    void FinishCluster(UShort_t channel);



   ClassDef(CbmStsClusterFinderModule, 1);

};

#endif /* CBMSTSCLUSTERFINDERMODULE_H */
