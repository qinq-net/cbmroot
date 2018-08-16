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

#include <map>
#include "CbmDigitize.h"
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
class CbmRichDigitizer : public CbmDigitize
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

   /** @brief Reset output arrays **/
   virtual void ResetArrays();

   /**
    * \brief Set crosstalk probability.
    */
   void SetCrossTalkProbability(Double_t crosstalk) {fCrossTalkProbability = crosstalk;}

   /**
    * \brief Set detector type
    */
   void SetDetectorType(CbmRichPmtTypeEnum detType){ fDetectorType = detType;}

   /**
    * \brief noise rate per McRichPoint / per  pixel / per second :
             hofNoiseDigis = nofRichPoints * nofPixels * dT(50 ns) * (fNoiseDigiRate / 1.e9);
    */
   void SetNoiseDigiRate(Double_t noise) {fNoiseDigiRate = noise;}

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


   /**
    * \brief Set Pixel dead time.
    */
   void SetPixelDeadTime(Double_t dt){ fPixelDeadTime = dt; }

   /** @brief Write a digi to the output
    ** @param digi  Pointer to digi object
    **/
   virtual void WriteDigi(CbmDigi* digi);


private:

   Int_t fEventNum;

   CbmRichDigitizerModeEnum fMode;

   TClonesArray* fRichPoints; // RICH MC points
   TClonesArray* fRichDigis; // RICH digis (output array)
   TClonesArray* fMcTracks; // Monte-Carlo tracks

   Double_t fNofPoints;  // total number of MCPoints processed
   Double_t fNofDigis;   // total number of digis created
   Double_t fTimeTot; // sum of execution time

   CbmRichPmt fPmt;
   Double_t fCrossTalkProbability; // probability of the crosstalk for direct neighbor for one pixel
   Double_t fNoiseDigiRate; // noise rate per McRichPoint / per  pixel / per second :
                            // hofNoiseDigis = nofRichPoints * nofPixels * dT(50 ns) * (fNoiseDigiRate / 1.e9);
   CbmRichPmtTypeEnum fDetectorType;

   map<Int_t, CbmRichDigi*> fDigisMap; //map which contains all fired digis, one digi per pixel

   Double_t fTimeResolution; // in ns
   Double_t fDarkRatePerPixel; // dark rate per pixel in Hz
   Double_t fPixelDeadTime; // in ns, during this time pixel can not be fired
   map<Int_t, Double_t> fFiredPixelsMap; // first: pixel address, second: last fired time.

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
    * \@value Number of digis written
    */
   Int_t AddDigisToOutputArray();

   /*
    * \brief Process current MC event.
    * \value Number of processed RichPoints
    */
   Int_t ProcessMcEvent();

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

   ClassDef(CbmRichDigitizer, 2)
};

#endif
