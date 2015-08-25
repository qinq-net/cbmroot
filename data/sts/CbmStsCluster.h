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


    /** @brief Get cluster index
     ** @return Index of cluster in cluster array
     **/
    Int_t GetIndex() const { return fIndex; }


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


	private:

    Double_t fCharge;        ///< Total charge
    Double_t fChannelMean;   ///< Charge-weighted mean channel number
    Double_t fChannelMeanSq; ///< Charge-weighted mean square channel number
    Double_t fTime;          ///< Cluster time (average of digi times)
    Int_t    fIndex;         ///< Index of cluster in input array


    ClassDef(CbmStsCluster, 5);
};

#endif
