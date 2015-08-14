/**
 ** \file CbmStsHit.h
 ** \author Volker Friese <v.friese@gsi.de>
 ** \since 30.08.06
 ** \brief Data class for a reconstructed hit in the STS
 **
 ** Updated 14/03/2014 by Andrey Lebedev <andrey.lebedev@gsi.de>.
 ** Updated 15/08/2015 by Volker Friese <v.friese@gsi.de>.
 **/

#ifndef CBMSTSSHIT_H
#define CBMSTSSHIT_H 1

#include "CbmPixelHit.h"

class TVector3;


/** @class CbmStsHit
 ** @brief  data class for a reconstructed 3-d hit in the STS
 **
 ** A hit in the STS is a position measurement constructed from two clusters
 ** on the front and back side of the sensors, respectively, which have
 ** a geometric intersection. In addition to the base class, it provides
 ** indices of the contributing clusters and the measurement time.
 **/
class CbmStsHit : public CbmPixelHit
{

public:

		/** Default constructor **/
    CbmStsHit();


    /** Constructor with all parameters
     ** @param address  Unique detector address (see CbmStsAddress)
     ** @param pos      Hit coordinate vector [cm]
     ** @param dpos     Hit coordinate error vector [cm]
     ** @param dxy      x-y covariance [cm**2]
     ** @param frontClusterId  Index of front-side cluster
     ** @param backClusterId   Index of back-side cluster
     ** @param time            Hit time [ns]
     ** @param timeError       Hit time error [ns]
     */
    CbmStsHit(Int_t address, const TVector3& pos, const TVector3& dpos,
    		      Double_t dxy, Int_t frontClusterId, Int_t backClusterId,
     		      Double_t time = 0., Double_t timeError = 0.);


    /** Destructor **/
    virtual ~CbmStsHit();


    /** Index of cluster at the back side
     ** @value  Back-side cluster index
     **/
    Int_t GetBackClusterId() const { return fBackClusterId; }


    /** Index of cluster at the front side
     ** @value  Front-side cluster index
     **/
    Int_t GetFrontClusterId() const { return fFrontClusterId; }


    /** Time of hit
     ** @return Hit time [ns]
     **/
    Double_t GetTime() const { return fTime; }


    /** Time error of hit
     ** @return Hit time error [ns]
     **/
    Double_t GetTimeError() const { return fTimeError; }


   /** Set hit time
     ** @param time  Hit time [ns]
     **/
    void SetTime(Double_t time) { fTime = time; }


    /** Set hit time error
     ** @param time  Hit time error [ns]
     **/
    void SetTimeError(Double_t error) { fTimeError = error; }


private:

    Int_t fFrontClusterId; ///< Cluster index front side
    Int_t fBackClusterId;  ///< Cluster index back side
    Double_t fTime;        ///< Hit time [ns]
    Double_t fTimeError;   ///< Hit time error [ms]

    ClassDef(CbmStsHit, 6);
};

#endif
