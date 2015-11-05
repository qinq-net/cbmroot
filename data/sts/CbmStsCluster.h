/**
 ** \file CbmStsCluster.h
 ** \author V.Friese <v.friese@gsi.de>
 ** \since 28.08.06
 ** \brief Data class for STS clusters.
 **
 ** Updated 25/06/2008 by R. Karabowicz.
 ** Updated 04/03/2014 by A. Lebedev <andrey.lebedev@gsi.de>
 ** Updated 10/06/2014 by V.Friese <v.friese@gsi.de>
 **/

#ifndef CBMSTSCLUSTER_H
#define CBMSTSCLUSTER_H 1

#include "CbmCluster.h"

/**
 ** \class CbmStsCluster
 ** \brief Data class for STS clusters.
 **
 ** The CbmStsCluster is a collection of CbmStsDigis in neighbouring
 ** module channels. Apart from the indices of the contributing digis,
 ** it provides address, time, total charge, mean position in channel units
 ** and the error of the latter.
 **/
class CbmStsCluster : public CbmCluster
{

	public:

    /**
     * \brief Default constructor
     **/
    CbmStsCluster();


    /**
     * \brief Destructor
     **/
    virtual ~CbmStsCluster();


    /** @brief Get cluster centre
     ** @value Cluster centre in channel units
     **/
    Double_t GetCentre() const { return fChannelMean; }


    /** Get the channel mean square
     ** @value Mean square channel number, weighted with charge
     **/
    Double_t GetChannelMeanSquare() const { return fChannelMeanSq; }


    /** @brief Get cluster charge
     ** @value  Total cluster charge [e]
     **
     ** This is the sum of the charges of the contributing digis.
     **/
    Double_t GetCharge() const { return fCharge; }


    /** @brief Cluster position
     ** @value Cluster position in channel number units
     **/
    Double_t GetPosition() const { return fChannelMean; }


    /** @brief Cluster position error
     ** @value Error (r.m.s.) of cluster position in channel number units
     **/
    Double_t GetPositionError() const { return fChannelError; }


    /** @brief Get cluster index
     ** @return Index of cluster in cluster array
     **/
    Int_t GetIndex() const { return fIndex; }


    /** @brief Set size of the cluster (number of channels)
     ** @value size  Number of channels from first to last
     **
     ** Note that this can be different from the number of digis in the
     ** cluster in case there are gaps e.g. due to dead channels.
     **/
    Int_t GetSize() const { return fSize; }


    /** @brief Get cluster time
     ** @return Time of cluster [ns]
     **
     ** This is the average time of the contributing digis.
     **/
    Double_t GetTime() const { return fTime; }


    /** @brief Set cluster index
     ** To keep track of the input during hit finding
     ** @param index  Index of cluster in cluster array
     **/
    void SetIndex(Int_t index) { fIndex = index; }


    /** @brief Set the position error
     ** @param error  Position error (r.m.s.) in channel units
     **/
    void SetPositionError(Double_t error) { fChannelError = error; }


    /** Set cluster properties (time, charge, mean)
     ** @param charge         Total charge in cluster
     ** @param channelMean    Charge-weighted mean channel number
     ** @param channelMeanSq  Charge-weighted mean square channel number
     ** @param time           Cluster time [ns]
     **/
    void SetProperties(Double_t charge, Double_t channelMean,
    		               Double_t channelMeanSq, Double_t time = 0.) {
    	fCharge        = charge;
    	fChannelMean   = channelMean;
    	fChannelMeanSq = channelMeanSq;
    	fTime          = time;
    }


    /** @brief Set size of the cluster (number of channels)
     ** @param size  Number of channels from first to last
     **
     ** Note that this can be different from the number of digis in the
     ** cluster in case there are gaps e.g. due to dead channels.
     **/
    void SetSize(Int_t size) { fSize = size; }


	private:

    Double_t fCharge;        ///< Total charge
    Int_t    fSize;          ///< Difference between first and last channel
    Double_t fChannelMean;   ///< Cluster centre in channel number units
    Double_t fChannelError;  ///< Cluster centre error (r.m.s.) in channel number units
    Double_t fChannelMeanSq; ///< Charge-weighted mean square channel number
    Double_t fTime;          ///< Cluster time (average of digi times)
    Int_t    fIndex;         ///< Index of cluster in input array

    //TODO: fChannelMeanSq is probably obsolete, since the determination
    //      of the cluster position is not any longer done by centre of gravity.
    //      In that case, it shall be removed eventually.


    ClassDef(CbmStsCluster, 6);
};

#endif
