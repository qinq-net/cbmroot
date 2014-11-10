/** @file CbmStsDigitize.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.05.2014
 **/

#ifndef CBMSTSDIGITIZE_H
#define CBMSTSDIGITIZE_H 1

#include <map>
#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmMatch.h"

class TClonesArray;
class CbmStsPoint;
class CbmStsSetup;


/** @class CbmStsDigitize
 ** @brief Task class for simulating the detector response of the STS
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 23.05.2014
 ** @version 2.0
 **
 ** The STS digitiser task reads CbmStsPoint from the input and produces
 ** objects of type CbmStsDigi. The StsPoints are distributed to the
 ** respective sensors, where the analog response is calculated. This
 ** is buffered and digitised by the connected module.
 ** The digitiser task triggers the readout of each module after the end
 ** of each call to Exec(), i.e. after processing one input MC event. All
 ** buffered data prior to the MC time of the current event are read out
 ** and stored in the output.
 **/
class CbmStsDigitize : public FairTask
{

 public:

  /** Constructor
   ** @param digiModel  Charge creation model.
   **                   0 = ideal, all charge in one strip.
   **                   1 = uniform charge creation along trajectory.
   **/
  CbmStsDigitize(Int_t digiModel = 0);


  /** Destructor **/
  virtual ~CbmStsDigitize();


  /** Create a digi and send it for further processing
   ** @param address   Unique channel address
   ** @param time      Absolute time [ns]
   ** @param adc       Digitised charge [ADC channels]
   ** @param match    MC Match object
   **/
  void CreateDigi(UInt_t address, ULong64_t time, UShort_t adc,
  		            const CbmMatch& match);


  /** Execution **/
  virtual void Exec(Option_t* opt);


  /** Set the digitisation parameters (same for all modules)
   ** @param dynRagne   Dynamic range [e]
   ** @param threshold  Threshold [e]
   ** @param nAdc       Number of ADC channels
   **/
  void SetParameters(Double_t dynRange, Double_t threshold, Int_t nAdc,
  		               Double_t timeResolution, Double_t deadTime) {
 	 fDynRange       = dynRange;
 	 fThreshold      = threshold;
 	 fNofAdcChannels = nAdc;
 	 fTimeResolution = timeResolution;
 	 fDeadTime       = deadTime;
   }




 private:

  Int_t fDigiModel;  ///< Detector response model. 0 = ideal, 1 = real.

  // --- Digitisation parameters
  Double_t fDynRange;         ///< Dynamic range [e]
  Double_t fThreshold;        ///< Threshold [e]
  Int_t    fNofAdcChannels;   ///< Number of ADC channels
  Double_t fTimeResolution;   ///< Time resolution (sigma) [ns]
  Double_t fDeadTime;         ///< Single-channel dead time [ns]

  CbmStsSetup*   fSetup;        ///< STS setup interface
  TClonesArray*  fPoints;       ///< Input array of CbmStsPoint
  TClonesArray*  fDigis;        ///< Output array of CbmStsDigi
  TClonesArray*  fMatches;      ///< Output array of CbmMatch
  TStopwatch     fTimer;        ///< ROOT timer

  // --- Event counters
  Int_t          fNofPoints;    ///< Number of points processed in Exec
  Int_t          fNofSignalsF;  ///< Number of signals on front side
  Int_t          fNofSignalsB;  ///< Number of signals on back side
  Int_t          fNofDigis;     ///< Number of created digis in Exec

  // --- Run counters
  Int_t          fNofEvents;      ///< Total number of events processed
  Double_t       fNofPointsTot;   ///< Total number of points processed
  Double_t       fNofSignalsFTot; ///< Number of signals on front side
  Double_t       fNofSignalsBTot; ///< Number of signals on back side
  Double_t       fNofDigisTot;    ///< Total number of digis created
  Double_t       fTimeTot;        ///< Total execution time


  /** End-of-run action **/
  virtual void Finish();


  /** Initialisation **/
  virtual InitStatus Init();


  /** Process one MCPoint
   ** @param point  Pointer to CbmStsPoint to be processed
   ** @param link   Link to MCPoint
   **/
  void ProcessPoint(CbmStsPoint* point, CbmLink* link = NULL);


  /** Re-initialisation **/
  virtual InitStatus ReInit();


  /** Reset eventwise counters **/
  void Reset();


  /** Set the digitisation parameters in the modules **/
  void SetModuleParameters();


  /** Set types for the sensors in the setup **/
  void SetSensorTypes();


  /** Prevent usage of copy constructor and assignment operator **/
  CbmStsDigitize(const CbmStsDigitize&);
  CbmStsDigitize operator=(const CbmStsDigitize&);



  ClassDef(CbmStsDigitize,2);

};

#endif


