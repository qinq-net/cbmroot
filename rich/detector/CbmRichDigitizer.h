/**
* \file CbmRichDigitizer.h
*
* \brief Class for producing RICH digis from from MCPoints.
*
* \author S.Lebedev
* \date 2015
**/

#ifndef CBM_RICH_DIGITIZER
#define CBM_RICH_DIGITIZER

#include "FairTask.h"
#include <map>
#include "CbmRichPmt.h"

class TClonesArray;
class CbmRichDigi;
class CbmRichPoint;
class CbmLink;

using namespace std;


/**
* \class CbmRichDigitizer
*
* \brief Class for producing RICH digis from from MCPoints.
*
* \author S.Lebedev
* \date 2015
**/
class CbmRichDigitizer : public FairTask
{
public:
   /**
    * \brief Default constructor.
    */
	CbmRichDigitizer();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichDigitizer();


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

   /**
    * \brief Set crosstalk probability.
    */
   void SetCrossTalkProbability(Double_t crosstalk) {fCrossTalkProbability = crosstalk;}

   /**
    * \brief Set detector type
    */
   void SetDetectorType(Int_t detType){ fPmt.SetDetectorType(detType);}

   /**
    * \brief Set number of noise hits to be added.
    */
   void SetNofNoiseHits(Int_t noise) {fNofNoiseDigis = noise;}

   /**
    * \brief Set collection efficiency for photoelectrons in PMT optics.
    */
   void SetCollectionEfficiency(Double_t collEff){ fPmt.SetCollectionEfficiency(collEff); }

   /**
    * \brief Set additional smearing of MC Points due to light scattering in mirror.
    */
   //void SetSigmaMirror(Double_t sigMirror) {fSigmaMirror = sigMirror;}

private:
   Int_t fEventNum;
   TClonesArray* fRichPoints; // RICH MC points
   TClonesArray* fRichDigis; // RICH digis (output array)
   TClonesArray* fMcTracks; // Monte-Carlo tracks

   CbmRichPmt fPmt;
   Double_t fCrossTalkProbability; // probability of the crosstalk for direct neighbor for one pixel
   Int_t fNofNoiseDigis; // number of noise digis

   map<Int_t, CbmRichDigi*> fDigisMap; //map which contains all fired digis, one digi per pixel

   /*
    * \brief Add crasstalk digis to the output array for the digi assuming fCrossTalkProbability
    */
   void AddCrossTalkDigis(
         CbmRichDigi* digi);

   /*
    * \brief Add fNofNoiseDigis number of digis.
    */
   void AddNoiseDigis();

   /*
    * \brief Process CbmRichPoint. Main method which is calle dfor all CbmRichPoints.
    */
   void ProcessPoint(
		   CbmRichPoint* point,
		   Int_t pointId);

   /*
    * \brief Add all the fired digis to the output array
    */
   void AddDigisToOutputArray();

   /*
    *
    */
   void AddDigi(Int_t address, const CbmLink& link);


   /**
    * \brief Copy constructor.
    */
   CbmRichDigitizer(const CbmRichDigitizer&);

   /**
    * \brief Assignment operator.
    */
   CbmRichDigitizer& operator=(const CbmRichDigitizer&);

   ClassDef(CbmRichDigitizer,1)
};

#endif
