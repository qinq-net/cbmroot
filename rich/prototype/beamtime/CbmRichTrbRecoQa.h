#ifndef CBMRICHTRBRECOQA_H
#define CBMRICHTRBRECOQA_H

#include "FairTask.h"
#include "CbmRichRingLight.h"
#include <vector>

using namespace std;

class TClonesArray;
class TH1D;
class TH2D;

class CbmRichRingFitterCOP;
class CbmRichRingFitterEllipseTau;

class CbmRichTrbRecoQa : public FairTask
{
public:
   /*
    * Constructor.
    */
   CbmRichTrbRecoQa();

   /*
    * Destractor.
    */
   virtual ~CbmRichTrbRecoQa();

   /**
    * \brief Inherited from FairTask.
    */
   virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Exec(Option_t* option);

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Finish();

    /*
     * Histogram initialization.
     */
    void InitHist();

   /*
    * Draw histograms.
    */
   void DrawHist();

   /*
    * Save histograms to the file.
    */
   void SaveHist();

    /*
     * Draw current event (event display)
     * \param fitRingCircle Fitted rings with COP algorithm.
     * \param fitRingEllipse FittedRings with TAU ellipse fitter.
     */
    void DrawEvent(
    		const vector<CbmRichRingLight>& fitRingCircle,
    		const vector<CbmRichRingLight>& fitRingEllipse);

    /*
     * Fill histogramms for the circle fit.
     */
    void FillHistCircle(
          CbmRichRingLight* ring);

    /*
     * Fill histogramms for the ellipse fit.
     */
    void FillHistEllipse(
          CbmRichRingLight* ring);

private:

   TClonesArray* fRichHits; // Array of RICH hits
   TClonesArray* fRichRings; // Array of found RICH rings
   TClonesArray* fRichHitInfos; // Array of CbmRichHitInfo

   TH1D* fhNofHitsInEvent; // number of hits in event
   TH2D* fhHitsXYCm; // XY distribution of the hits in event in cm
   TH2D* fhHitsXYPixel; // XY distribution of the hits in event in pixels
   TH1D* fhNofRingsInEvent; // number of found rings per event
   TH1D* fhNofHitsInRing; // number of hits in found rings

   //Ellipse histograms
   TH1D* fhBoverAEllipse;
   TH2D* fhXcYcEllipse;
   TH1D* fhBaxisEllipse;
   TH1D* fhAaxisEllipse;
   TH1D* fhChi2Ellipse;

   //circle histograms
   TH2D* fhXcYcCircle;
   TH1D* fhRadiusCircle;
   TH1D* fhChi2Circle;
   TH1D* fhDrCircle;

   TH1D* fhEventsWithRings;
   TH1D* fhHitsPerPMT;

   UInt_t fEventNum; // Event counter
   UInt_t fNofDrawnEvents; // Number of drawn events

   CbmRichRingFitterCOP* fCopFit;
   CbmRichRingFitterEllipseTau* fTauFit;

   CbmRichTrbRecoQa(const CbmRichTrbRecoQa&){;}
   CbmRichTrbRecoQa operator=(const CbmRichTrbRecoQa&){;}

   ClassDef(CbmRichTrbRecoQa, 1);
};

#endif
