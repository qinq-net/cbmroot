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
   **                   1 = simple, uniform charge creation
   **                   2 = advanced
   **/
  CbmStsDigitize(Int_t digiModel = 2);


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


  /** Execution **/
  virtual void Exec(Option_t* opt);


  /** Percentage of dead channels **/
  Double_t GetDeadChannelFraction() const {
  	return fDeadChannelFraction;
  }

  /** Digitize model **/
  // TODO: can be removed after validation of SensorTypeDssd
  Int_t GetDigitizeModel() const { return fDigiModel;}


  /** Flag for energy loss model
   ** @value 0 = ideal, 1 = uniform, 2 = fluctuations
   **/
  static Int_t GetElossModel() { return fElossModel; }


  /** Get number of signals front side **/
  Int_t GetNofSignalsF() const {return fNofSignalsF;}

  /** Get number of signals back side **/
  Int_t GetNofSignalsB() const {return fNofSignalsB;}

  /** Get the digitisation parameters 
   ** @param dynRagne             Dynamic range [e]
   ** @param threshold            Threshold [e]
   ** @param nAdc                 Number of ADC channels
   ** @param timeResolution       Time resolution [ns]
   ** @param deadTimec            Single-channel dead time [ns]
   ** @param noise                Equivalent noise charge (sigma) [e]
   **/
   void GetParameters(Double_t& dynRange, Double_t& threshold, Int_t& nAdc,
  		               Double_t& timeResolution, Double_t& deadTime,
  		               Double_t& noise) {
 	 dynRange       = fDynRange;
 	 threshold      = fThreshold;
 	 nAdc           = fNofAdcChannels;
 	 timeResolution = fTimeResolution;
 	 deadTime       = fDeadTime;
 	 noise          = fNoise;
    }


  /** Initialise the STS setup and the parameters **/
  void InitSetup();


  /** Re-initialisation **/
  virtual InitStatus ReInit();


  /** Set percentage of dead channels in the modules **/
  void SetDeadChannelFraction(Double_t fraction = 0.);


   /** Set the digitisation parameters in the modules **/
  void SetModuleParameters();


  /** Set the digitisation parameters (same for all modules)
   ** @param dynRagne             Dynamic range [e]
   ** @param threshold            Threshold [e]
   ** @param nAdc                 Number of ADC channels
   ** @param timeResolution       Time resolution [ns]
   ** @param deadTimec            Single-channel dead time [ns]
   ** @param noise                Equivalent noise charge (sigma) [e]
   **/
  void SetParameters(Double_t dynRange = 75000., Double_t threshold = 3000., Int_t nAdc = 32,
  		               Double_t timeResolution = 10., Double_t deadTime = 800.,
  		               Double_t noise = 1000.) {
 	 fDynRange       = dynRange;
 	 fThreshold      = threshold;
 	 fNofAdcChannels = nAdc;
 	 fTimeResolution = timeResolution;
 	 fDeadTime       = deadTime;
 	 fNoise          = noise;
   }


  /** Set physics processes
   ** @param eLossModel       0 = ideal, 1 = uniform, 2 = fluctuations
   ** @param useLorentzShift  If kTRUE, activate Lorentz shift
   ** @param useDiffusion     If kTRUE, activate diffusion
   ** @param useCrossTalk     If kTRUE; activate cross talk
   **
   ** Changing the physics flags is only allowed before Init() is called.
   **/
  void SetProcesses(Int_t eLossModel,
  		              Bool_t useLorentzShift = kTRUE,
  		              Bool_t useDiffusion = kTRUE,
  		              Bool_t useCrossTalk = kTRUE);


  /** (De-)Activate processing of secondary tracks
   ** @param flag  kTRUE if secondaries shall be processed (default)
   **
   ** This flag enables the user to suppress the digitisation of
   ** StsPoints from secondary tracks for debug purposes. By default,
   ** points from all tracks are processed.
   **/
  void SetProcessSecondaries(Bool_t flag = kTRUE) {
  	fProcessSecondaries = flag;
  }


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
  void SetSensorStripPitch(Double_t pitch) { fStripPitch = pitch; }


  /** Set types for the sensors in the setup **/
  void SetSensorTypes();


  /** Flag for cross talk
   ** @value kTRUE if cross talk is activated
   **/
  static Bool_t UseCrossTalk() { return fUseCrossTalk; }


  /** Flag for diffusion
   ** @value kTRUE if diffusion is activated
   **/
  static Bool_t UseDiffusion() { return fUseDiffusion; }


   /** Flag for Lorentz shift
   ** @value kTRUE if Lorentz shift is activated
   **/
  static Bool_t UseLorentzShift() { return fUseLorentzShift; }



 private:

  Int_t fMode;       ///< Run mode. 0 = stream, 1 = event
  Int_t fDigiModel;  ///< Detector response model. 0 = ideal, 1 = simple, 2 = real
  Bool_t fProcessSecondaries;  ///< If kFALSE, only primaries will be digitised
  static Bool_t fIsInitialised;   ///< kTRUE if Init() was called

  // --- Digitisation parameters
  Double_t fDynRange;            ///< Dynamic range [e]
  Double_t fThreshold;           ///< Threshold [e]
  Int_t    fNofAdcChannels;      ///< Number of ADC channels
  Double_t fTimeResolution;      ///< Time resolution (sigma) [ns]
  Double_t fDeadTime;            ///< Single-channel dead time [ns]
  Double_t fNoise;               ///< equivalent noise charge (sigma) [ns]
  Double_t fDeadChannelFraction; ///< fraction of dead channels [%]

  // --- Strip pitch, in case it is explicitly set by the user and not taken from
  // --- the sensor DB
  Double_t fStripPitch;

  // --- Switches for physics process
  static Int_t  fElossModel;      ///< Energy loss model (0, 1 or 2)
  static Bool_t fUseLorentzShift; ///< Lorentz shift of charge carriers in magnetic field
  static Bool_t fUseDiffusion;    ///< Diffusion of charge carriers
  static Bool_t fUseCrossTalk;    ///< Cross talk due to inter-strip capacitance

  CbmStsSetup*   fSetup;        ///< STS setup interface
  TClonesArray*  fPoints;       ///< Input array of CbmStsPoint
  TClonesArray*  fTracks;       ///< Input array of CbmMCTrack
  TClonesArray*  fDigis;        ///< Output array of CbmStsDigi
  TClonesArray*  fMatches;      ///< Output array of CbmMatch
  TStopwatch     fTimer;        ///< ROOT timer

  // --- Time of last processed StsPoint (for stream mode)
  Double_t fTimePointLast;

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
  Double_t       fTimeTot;        ///< Total execution time


  /** Status of the analog buffers **/
  string BufferStatus() const;

  /** Status of the analog buffers **/
  string BufferStatus2() const;

  /** End-of-run action **/
  virtual void Finish();


  /** Get event information
   ** @param[out]  eventNumber  Number of MC event
   ** @param[out]  inputNumber  Number of input
   ** @param[out]  eventTime    Start time of event [ns]
   **
   ** In case of being run with FairRunAna, this information
   ** is taken from FairEventHeader. If the task is run with
   ** FairRunSim, the FairEventHeader is not filled, so the
   ** respective information is taken from FairMCEventHeader.
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


