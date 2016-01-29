/**
* \file CbmRichHitProducer.h
*
* \brief Class for producing RICH hits directly from MCPoints.
*
* \author B. Polichtchouk
* \date 2004
**/

#ifndef CBM_RICH_HIT_PRODUCER
#define CBM_RICH_HIT_PRODUCER

#include "FairTask.h"
#include "CbmRichRecGeoPar.h"

//class CbmGeoRichPar;
class TClonesArray;
class TVector3;

/**
* \class CbmRichHitProducer
*
* \brief Class for producing RICH hits directly from MCPoints.
*
* \author B. Polichtchouk
* \date 2004
**/
class CbmRichHitProducer : public FairTask
{
public:
   /**
    * \brief Default constructor.
    */
   CbmRichHitProducer();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichHitProducer();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void SetParContainers();

   /**
    * \brief Inherited from FairTask.
    */
   virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Exec(
        Option_t* option);

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Finish();



private:

   TClonesArray* fRichDigis; // RICH digis
   TClonesArray* fRichHits; // RICH hits

   Int_t fEventNum; // event number

    /**
     * \brief Add hit to the output array
     */
     
   void AddHit(
         TVector3 &posHit,
   		Int_t index);

   /**
    * \brief Copy constructor.
    */
   CbmRichHitProducer(const CbmRichHitProducer&);

   /**
    * \brief Assignment operator.
    */
   CbmRichHitProducer& operator=(const CbmRichHitProducer&);

   ClassDef(CbmRichHitProducer,1)
};

#endif
