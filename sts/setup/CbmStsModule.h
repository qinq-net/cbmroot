/** @file CbmStsModule.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 14.05.2013
 **/

#ifndef CBMSTSMODULE_H
#define CBMSTSMODULE_H 1


#include <map>
#include <set>
#include <vector>
#include "TF1.h"
#include "TRandom.h"
#include "CbmStsCluster.h"
#include "CbmStsDigi.h"
#include "digitize/CbmStsSignal.h"
#include "setup/CbmStsElement.h"
#include "setup/CbmStsSensor.h"

class TClonesArray;
class CbmStsPhysics;


/** @class CbmStsModule
 ** @brief Class representing an instance of a readout unit in the CBM-STS.
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** The StsModule is the read-out unit in the CBM STS. It consists of one
 ** sensor or two or more daisy-chained sensors (CbmStsSensor), the analogue
 ** cable and the read-out electronics.
 **
 ** The module receives and stores the analogue signals from the sensor(s)
 ** in a buffer. It takes care of interference of signals in one and the
 ** same channel (two signals arriving within a given dead time).
 ** The module digitises the analogue signals and sends them to the
 ** CbmDaq when appropriate.
 **/
class CbmStsModule : public CbmStsElement
{

  public:


    /** Standard constructor
     ** @param address  Unique element address
     ** @param node     Pointer to geometry node
     ** @param mother   Pointer to mother element
     **/
    CbmStsModule(UInt_t address = 0, TGeoPhysicalNode* node = nullptr,
                 CbmStsElement* mother = nullptr);


    // Copy constructor and assignment operator not allowed
    CbmStsModule(const CbmStsModule&) = delete;
    CbmStsModule& operator=(const CbmStsModule&) = delete;


    /** Destructor **/
    virtual ~CbmStsModule();


    /** Convert ADC channel to charge
     ** @param adcChannel  ADC channel number
     ** @return analogue charge [e]
     **/
    Double_t AdcToCharge(UShort_t adcChannel);


    /** @brief Add a cluster to its array
     ** @param cluster Pointer to cluster object to be added
     **/
    void AddCluster(CbmStsCluster* cluster) { fClusters.push_back(cluster); }


    /** Add an analogue signal to the buffer
     **
     ** @param channel        channel number
     ** @param time           time of signal [ns]
     ** @param charge         analogue charge [e]
     ** @param index          index of CbmStsPoint in TClonesArray
     ** @param entry          MC entry (event number)
     ** @param file           MC input file number
     **
     ** The signal will be added to the buffer. Interference with
     ** previous signals within the same channels is checked and the
     ** proper action is executed.
     **/
    void AddSignal(UShort_t channel, Double_t time, Double_t charge,
                   Int_t index = 0, Int_t entry = 0, Int_t file = 0);


    /** Get status of the analogue buffer
     **
     ** @paramOut nofSignals Number of signals in buffer (active channels)
     ** @paramOut timeFirst  Time of first signal in buffer [ns]
     ** @paramOut timeLast   Time of last signal in buffer [ns]
     **/
    void BufferStatus(Int_t& nofSignals,
                      Double_t& timeFirst, Double_t& timeLast);


    /** Convert charge to ADC channel.
     ** @param charge  analogUE charge [e]
     ** @return  ADC channel number
     **
     ** This must be the inverse of AdcToCharge
     **/
    Int_t ChargeToAdc(Double_t charge);


    /** Clear the cluster vector **/
    void ClearClusters() { fClusters.clear(); }


    /** Find hits from clusters
     ** @param hitArray  Array where hits shall be registered
     ** @param event     Pointer to current event for registering of hits
     ** @return Number of created hits
     **/
    Int_t FindHits(TClonesArray* hitArray, CbmEvent* event = NULL);


    /** Dynamic range
     ** @value Dynamic range [e]
     **/
    Double_t GetDynamicRange() const { return fDynRange; }


    /** Electronic noise
     ** @value Electronic noise (r.m.s. in [e])
     **/
    Double_t GetNoise() const { return fNoise; }


    /** Number of ADC channels for charge discretisation
     ** @value Number of ADC channels
     **/
    Int_t GetNofAdcChannels() const { return fNofAdcChannels; }


    /** @brief Number of electronic channels
     ** @value Number of ADC channels
     **/
    UShort_t GetNofChannels() const { return fNofChannels; };


    /** @brief Current number of clusters
     ** @value Number of clusters in the buffer
     **/
    Int_t GetNofClusters() const { return fClusters.size(); }


    /** @brief Set of dead channels
     ** @value Set of dead channels
     **/
    std::set<UShort_t> GetSetOfDeadChannels() const {return fDeadChannels;}


    /** Time resolution
     ** @value Time resolution (r.m.s.) [ns]
     **/
    Double_t GetTimeResolution() const { return fTimeResolution; }


    /** @brief Zero threshold noise rate
     ** @value Noise rate at zero threshold [1/ns]
     **/
    Double_t GetZeroNoiseRate() const { return fZeroNoiseRate; }


    /** Initialise the analogue buffer
     ** The analogue buffer contains a std::multiset for each channel, to be
     ** filled with CbmStsSignal objects. Without this method, a channel
     ** multiset would be instantiated at run time when the first signal
     ** for this channel arrives. Depending on the occupancy of this channel,
     ** this may happen only after several hundreds of events. Consequently,
     ** the memory consumption will increase for the first events until
     ** each channel was activated at least once. This behaviour mimics a
     ** memory leak and makes it harder to detect a real one in other parts
     ** of the code. This is avoided by instantiating each channel multiset
     ** at initialisation time.
     **/
    void InitAnalogBuffer();


    /** Check if a channel is active or deactivated.
     ** @param channel  Channel number
     ** @value kTRUE if channel is active
     **/
    Bool_t IsChannelActive(UShort_t channel) {
      return ( fDeadChannels.find(channel) == fDeadChannels.end() );
    }


    /** Check whether module parameters are set
     ** @value kTRUE if parameters are set
     **/
    Bool_t IsSet() const { return fIsSet; }


    /** Digitise signals in the analogue buffer
     ** @param time  readout time [ns]
     ** @return Number of created digis
     **
     ** All signals with time less than the readout time minus a
     ** safety margin (taking into account dead time and time resolution)
     ** will be digitised and removed from the buffer.
     **/
    Int_t ProcessAnalogBuffer(Double_t readoutTime);


    /** Create list of inactive channels
     ** @param percentOfDeadChannels  Percentage of dead channels
     ** @value Number of dead channels in the sensor
     **
     ** The specified fraction of randomly chosen channels are set inactive.
     **/
    Int_t SetDeadChannels(Double_t percentOfDeadCh);


    /** Set the digitisation parameters
     ** @param dynRagne        Dynamic range [e]
     ** @param threshold       Threshold [e]
     ** @param nAdc            Number of ADC channels
     ** @param timeResolution  Time resolution [ns]
     ** @param deadTime        Single-channel dead time [ns]
     ** @param noise           Equivalent noise charge [e]
     ** @param zeroNoiseRate   Zero threshold noise rate [1/ns]
     **/
    void SetParameters(Int_t nChannels, Double_t dynRange,
                       Double_t threshold, Int_t nAdc,
                       Double_t timeResolution, Double_t deadTime,
                       Double_t noise, Double_t zeroNoiseRate) {
      fNofChannels    = nChannels;
      fDynRange       = dynRange;
      fThreshold      = threshold;
      fNofAdcChannels = nAdc;
      fTimeResolution = timeResolution;
      fDeadTime       = deadTime;
      fNoise          = noise;
      fZeroNoiseRate  = zeroNoiseRate;
      fIsSet          = kTRUE;
      fNoiseRate = 0.5 * fZeroNoiseRate
          * TMath::Exp( -0.5 * fThreshold * fThreshold / (fNoise * fNoise) );
      InitAnalogBuffer();
      fNoiseCharge = new TF1("Noise Charge", "TMath::Gaus(x, [0], [1])",
                             threshold, 10. * noise);
      fNoiseCharge->SetParameters(0., noise);
    }


    /** @brief Generate noise
     ** @param t1  Start time [ns]
     ** @param t2  Stop time [n2]
     **
     ** This method will generate noise digis in the time interval [t1, t2]
     ** according to Rice's formula. The number of noise digis in this
     ** interval is sampled from a Poissonian with mean calculated from
     ** the single-channel noise rate, the number of channels and the
     ** length of the time interval. The noise hits are randomly distributed
     ** to the channels. The time of each noise digi is sampled from a flat
     ** distribution, its charge from a Gaussian with sigma = noise,
     ** truncated at threshold.
     **/
    Int_t GenerateNoise(Double_t t1, Double_t t2);


    /** Set dead time for time-based hit finding
     ** @param dTime  Max. time difference for two cluster to make a hit
     **/
    void SetDeadTime(Double_t dTime) { fDeadTime = dTime; }



  private:

    UShort_t fNofChannels;       ///< Number of electronic channels
    Double_t fDynRange;          ///< dynamic range [e]
    Double_t fThreshold;         ///< threshold [e]
    Int_t    fNofAdcChannels;    ///< Number of ADC channels
    Double_t fTimeResolution;    ///< Time resolution (sigma) [ns]
    Double_t fDeadTime;          ///< Channel dead time [ns]
    Double_t fNoise;             ///< Equivalent noise charge (sigma) [e]
    Double_t fZeroNoiseRate;     ///< Zero-threshold noise rate [1/ns]
    Double_t fNoiseRate;         ///< Noise rate [1/ns]
    Bool_t   fIsSet;             ///< Flag whether parameters are set
    std::set <UShort_t> fDeadChannels;    ///< List of inactive channels
    CbmStsPhysics* fPhysics;  //!  Pointer to CbmStsPhysics instance
    TF1* fNoiseCharge;        //! Histogram to sample the noise charge

    /** Buffer for analog signals, key is channel number.
     ** Because signals do not, in general, arrive time-sorted,
     ** the buffer must hold a (multi)set of signals for each channel
     ** (multiset allowing for different signals at the same time).
     ** Sorting in the multiset is with the less operator of CbmStsSignal,
     ** which compares the time of the signals.
     **/
    typedef std::multiset<CbmStsSignal*, CbmStsSignal::Before> sigset;
    std::map<UShort_t, sigset> fAnalogBuffer;


    /** Vector of clusters. Used for hit finding. **/
    std::vector<CbmStsCluster*> fClusters;


    /** Digitise an analogue charge signal
     ** @param channel Channel number
     ** @param signal  Pointer to signal object
     **/
    void Digitize(UShort_t channel, CbmStsSignal* signal);


    /** Initialise daughters from geometry **/
    virtual void InitDaughters();



    ClassDef(CbmStsModule, 2);

};

#endif /* CBMSTSMODULE_H */
