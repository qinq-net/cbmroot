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
#include "CbmStsDigitizeSettings.h"

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

  /** Constructor **/
  CbmStsDigitize();


  /** Destructor **/
  virtual ~CbmStsDigitize();


  /** Create a digi and send it for further processing
   ** @param address   Unique channel address
   ** @param time      Absolute time [ns]
   ** @param adc       Digitised charge [ADC channels]
   ** @param match    MC Match object
   **/
  void CreateDigi(UInt_t address, Long64_t time, UShort_t adc,
  		            const CbmMatch& match);


  /** @brief Discard processing of secondary tracks
   ** @param flag  kTRUE if secondaries shall be discarded
   **
   ** This flag enables the user to suppress the digitisation of
   ** StsPoints from secondary tracks for debug purposes. By default,
   ** points from all tracks are processed.
   **/
  void DiscardSecondaries(Bool_t flag = kTRUE) {
    fSettings->SetDiscardSecondaries(flag);
  }


  /** Execution **/
  virtual void Exec(Option_t* opt);


  /** Get energy loss model
  ** @param eLossModel       0 = ideal, 1 = uniform, 2 = fluctuations
  **/
  Int_t GetELossModel() const{return fSettings->GetELossModel();}


  /** Get current event time
   ** @value Current event time [ns]
   **/
  Double_t GetEventTime() { return fEventTimeCurrent; }


  /** Get number of signals front side **/
  Int_t GetNofSignalsF() const {return fNofSignalsF;}


  /** Get number of signals back side **/
  Int_t GetNofSignalsB() const {return fNofSignalsB;}


  /** @brief Get the digitisation settings
   ** @value Pointer to digitisation settings
   **/
  CbmStsDigitizeSettings* GetSettings() const { return fSettings; }


  /** Initialise the STS setup and the parameters **/
  void InitSetup();


  /** Re-initialisation **/
  virtual InitStatus ReInit();


  /** @brief Activate noise generation
   ** @param choice If kTRUE, noise will be generated (only in stream mode).
   **
   ** By default, noise is not generated.
   ** Changing the physics flags is only allowed before Init() is called.
   **/
  void SetGenerateNoise(Bool_t choise = kTRUE);


   /** Set the digitisation parameters in the modules **/
  void SetModuleParameters();


  /** Set the digitisation parameters (same for all modules)
   ** @param dynRagne             Dynamic range [e]
   ** @param threshold            Threshold [e]
   ** @param nAdc                 Number of ADC channels
   ** @param timeResolution       Time resolution [ns]
   ** @param deadTimec            Single-channel dead time [ns]
   ** @param noise                Equivalent noise charge (sigma) [e]
   ** @param zeroNoiseRate        Zero-threshold noise rate [1/ns]
   **
   ** Changing the parameters is only allowed before Init() is called.
   **/
  void SetParameters(Double_t dynRange, Double_t threshold, Int_t nAdc,
  		               Double_t timeResolution, Double_t deadTime,
  		               Double_t noise, Double_t zeroNoiseRate,
  		               Double_t deadChannelFrac = 0.);


  /** Set physics processes
   ** @param eLossModel       0 = ideal, 1 = uniform, 2 = fluctuations
   ** @param useLorentzShift  If kTRUE, activate Lorentz shift
   ** @param useDiffusion     If kTRUE, activate diffusion
   ** @param useCrossTalk     If kTRUE, activate cross talk
   ** @param generateNoise    If kTRUE, noise will be generated
   **
   ** Changing the physics flags is only allowed before Init() is called.
   **/
  void SetProcesses(Int_t eLossModel,
  		            Bool_t useLorentzShift = kTRUE,
  		            Bool_t useDiffusion = kTRUE,
  		            Bool_t useCrossTalk = kTRUE,
  		            Bool_t generateNoise = kFALSE);


  /** Set the operating parameters in the sensors **/
  void SetSensorConditions();


  /** Set the sensor strip pitch
   ** @param  pitch  Strip pitch [cm]
   **
   ** The internal sensor parameters like pitch, stereo angle etc. are normally taken
   ** from a sensor database. This method allows to override the value for the strip
   ** pitch defined there, in order to easily test different sensor layout options
   ** without defining new types in the database. It has effect only for strip sensor types.
   ** The specified strip pitch will be applied for all sensors in the setup.
   **/
  void SetSensorStripPitch(Double_t pitch) {
    fSettings->SetStripPitch(pitch);
  }




 private:

  Int_t  fMode;       ///< Run mode. 0 = stream, 1 = event
  Bool_t fIsInitialised;   ///< kTRUE if Init() was called

  CbmStsDigitizeSettings* fSettings;  ///< Digitisation parameters and settings
  CbmStsSetup*   fSetup;        ///< STS setup interface
  TClonesArray*  fPoints;       ///< Input array of CbmStsPoint
  TClonesArray*  fTracks;       ///< Input array of CbmMCTrack
  TClonesArray*  fDigis;        ///< Output array of CbmStsDigi
  TClonesArray*  fMatches;      ///< Output array of CbmMatch
  TStopwatch     fTimer;        ///< ROOT timer

  // --- Time of last processed StsPoint (for stream mode)
  Double_t fTimePointLast;

  // --- Time of current event (for stream mode)
  Double_t fEventTimeCurrent;   ///< Time of current event [ns]

  // --- Digi times (for stream mode, in each step)
  Double_t fTimeDigiFirst;      ///< Time of first digi sent to DAQ
  Double_t fTimeDigiLast;       ///< Time of last digi sent to DAQ

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
  Double_t       fNofNoiseTot;    ///< Total number of noise digis
  Double_t       fTimeTot;        ///< Total execution time


  /** Status of the analog buffers **/
  std::string BufferStatus() const;

  /** Status of the analog buffers **/
  std::string BufferStatus2() const;

  /** End-of-run action **/
  virtual void Finish();


  /** Get event information
   ** @param[out]  eventNumber  Number of MC event
   ** @param[out]  inputNumber  Number of input
   ** @param[out]  eventTime    Start time of event [ns]
   **
   ** In case of being run with FairRunAna, input number and time
   ** are taken from FairEventHeader. If the task is run with
   ** FairRunSim, the FairEventHeader is not filled, so input number
   ** and event time are zero.
   ** The event number is always taken from the FairRootManager, because the
   ** event number from FaurEventHeader::GetMCEntryNumber() is
   ** generator-dependent.
   **/
  void GetEventInfo(Int_t& inputNr, Int_t& eventNr, Double_t& eventTime);


  /** Initialisation **/
  virtual InitStatus Init();


  /** Process the analog buffers of all modules
   ** @param readoutTime  Time of readout [ns]
   **/
  void ProcessAnalogBuffers(Double_t readoutTime);


  /** Process StsPoints from MCEvent **/
  void ProcessMCEvent();


  /** Process one MCPoint
   ** @param point  Pointer to CbmStsPoint to be processed
   ** @param link   Link to MCPoint
   **/
  void ProcessPoint(const CbmStsPoint* point, Double_t eventTime = 0.,
  		              CbmLink* link = NULL);


  /** Reset step-wise counters **/
  void Reset();


  /** Prevent usage of copy constructor and assignment operator **/
  CbmStsDigitize(const CbmStsDigitize&);
  CbmStsDigitize operator=(const CbmStsDigitize&);



  ClassDef(CbmStsDigitize,3);

};

#endif


