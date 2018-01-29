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
#include "CbmRichPmtType.h"

class TClonesArray;
class CbmRichDigi;
class CbmRichPoint;
class CbmLink;

using namespace std;

enum CbmRichDigitizerModeEnum { CbmRichDigitizerModeEvents = 0, CbmRichDigitizerModeTimeBased = 1 };


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
   void SetDetectorType(CbmRichPmtTypeEnum detType){ fPmt.SetDetectorType(detType);}

   /**
    * \brief Set noise hit rate in [%]. nofNoiseDigitsPerEvent = fNoiseHitRate * NofAllPixels / 100.
    */
   void SetNoiseHitRate(Double_t noise) {fNoiseHitRate = noise;}

   /**
    * \brief Set collection efficiency for photoelectrons in PMT optics.
    */
   void SetCollectionEfficiency(Double_t collEff){ fPmt.SetCollectionEfficiency(collEff); }

   /**
    * \brief Set additional smearing of MC Points due to light scattering in mirror.
    */
   //void SetSigmaMirror(Double_t sigMirror) {fSigmaMirror = sigMirror;}

   /**
    * \brief Set Rich digitizer mode (CbmRichDigitizerModeEnum).
    */
   void SetMode(CbmRichDigitizerModeEnum mode){ fMode = mode; }

   /**
    * \brief Set Time resolution.
    */
   void SetTimeResolution(Double_t dt){ fTimeResolution = dt; }

private:
   Int_t fEventNum;

   CbmRichDigitizerModeEnum fMode;

   TClonesArray* fRichPoints; // RICH MC points
   TClonesArray* fRichDigis; // RICH digis (output array)
   TClonesArray* fMcTracks; // Monte-Carlo tracks

   CbmRichPmt fPmt;
   Double_t fCrossTalkProbability; // probability of the crosstalk for direct neighbor for one pixel
   Double_t fNoiseHitRate; // in [%] nofNoiseDigitsPerEvent = fNoiseHitRate * NofAllPixels / 100.

   map<Int_t, CbmRichDigi*> fDigisMap; //map which contains all fired digis, one digi per pixel

   Double_t fEventTime; // time of the current event
   Double_t fTimeResolution; // in ns

   /*
    * \brief Add crasstalk digis to the output array for the digi assuming fCrossTalkProbability
    */
   void AddCrossTalkDigis(
         CbmRichDigi* digi);

   /*
    * \brief Add fNofNoiseDigis number of digis.
    */
   void AddNoiseDigis(
           Int_t eventNum,
           Int_t inputNum);

   /*
    * \brief Process CbmRichPoint. Main method which is calle dfor all CbmRichPoints.
    */
   void ProcessPoint(
		   CbmRichPoint* point,
		   Int_t pointId,
		   Int_t eventNum,
		   Int_t inputNum);

   /*
    * \brief Add all the fired digis to the output array
    */
   void AddDigisToOutputArray();

   /*
    * \brief Process current MC event.
    */
   void ProcessMcEvent();

   /*
    * \brief Generate noise between events.
    */
   void GenerateNoiseBetweenEvents(
         Double_t oldEventTime,
         Double_t newEventTime);

   /*
    *
    */
   void AddDigi(
         Int_t address,
         Double_t time,
         const CbmLink& link);


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
