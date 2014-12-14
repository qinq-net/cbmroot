// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                            CbmSourceLmdNew                        -----
// -----                    Created 10.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMSOURCELMDNEW_H
#define CBMSOURCELMDNEW_H

#include "FairSource.h"
#include "FairLogger.h"

#include "CbmROCUnpack.h"
#include "CbmDetectorList.h"

#include "TList.h"
#include "TString.h"
#include "TObjString.h"

#include "roc/Message.h"
#include "roc/Iterator.h"

#include <map>
#include <set>

class CbmDigi;
class CbmDaqMap;
class CbmTbDaqBuffer;
class CbmTbEvent;
class TClonesArray;
class CbmTbEventBuilder;

class CbmSourceLmdNew : public FairSource
{
  public:
    CbmSourceLmdNew();
    CbmSourceLmdNew(const char *);
    CbmSourceLmdNew(const CbmSourceLmdNew& source);
    virtual ~CbmSourceLmdNew();

    virtual Bool_t Init();
    virtual Int_t ReadEvent();
    virtual void Close();

    virtual void Reset();

    void SetReadInTimeStep(ULong_t step) { fReadInTimeStep = step;  }
    void SetPersistence(Bool_t persistence = kTRUE) { fPersistence = persistence; }

    void SetTriggeredMode(Bool_t mode) { fTriggeredMode = mode; }

    void AddFile(const char * fname) 
    { fInputFileList.Add(new TObjString(fname)); }

    void AddUnpacker(CbmROCUnpack* unpacker, Int_t messageType)
    { fUnpackers.insert(std::pair<Int_t,CbmROCUnpack*>(messageType,unpacker));}

    void SetEventBuilder(CbmTbEventBuilder* eventBuilder)
    { fEventBuilder = eventBuilder; }

    //    CbmTbDaqBuffer* GetBuffer() { return fBuffer; }

    void SetBaseline(Bool_t baseline) { fBaselineData = baseline; } 
 
    Bool_t IsBaseline() { return fBaselineData; }
    Bool_t IsBaseline(Int_t rocNr);

    void AddBaselineRoc(Int_t rocNr);
    Bool_t RemoveBaselineRoc(Int_t rocNr);

    void SetEpoch(Int_t rocNr, uint32_t epochNr) 
    {  fCurrentEpoch[rocNr] = epochNr; }
    
    void CheckCurrentEpoch(Int_t rocNr) 
    { 
      if ( fCurrentEpoch.find(rocNr) == fCurrentEpoch.end() ) {
	LOG(FATAL) << GetName()
		   << ": Hit message without previous epoch marker for ROC "
		   << rocNr << FairLogger::endl;
      }
    }

    void SetStartTime(ULong_t time) 
    { fTimeStart = time; }

    void SetStopTime(ULong_t time) 
    { fTimeStop = time; }

    ULong_t GetStartTime() 
    { return fTimeStart; }

    ULong_t GetStopTime() 
    { return fTimeStop; }

 private:

    // --- Settings
    TString  fInputFileName;    ///< Name of lmd input file
    TList    fInputFileList;    ///< List of input files
    Int_t    fFileCounter;      ///< Counter the actual file in list
    ULong_t  fReadInTimeStep;   ///< Time step in which data from input are read
    Bool_t   fPersistence;      ///< Flag for file storage of output arrays

    // --- Auxiliary classes
    CbmTbDaqBuffer* fBuffer;        ///< Digi buffer instance
    CbmDaqMap* fDaqMap;             ///< Mapping from electronics to detectors
    roc::Iterator* fRocIter;        ///< ROC iterator
    CbmTbEventBuilder* fEventBuilder; ///< EventBuilder

    // --- Status
    ULong_t fTimeStart;
    ULong_t fTimeStop;
    ULong_t fCurrentEpochTime; ///< Time stamp of current epoch **/
    UInt_t fStartEpoch;             ///< Number of first epoch

    /** Current epoch marker for each ROC **/
    std::map<Int_t, uint32_t> fCurrentEpoch;  

    ULong_t fCurrentBufferTime;     ///< Time limit of current buffer
    roc::Message*  fCurrentMessage;      //! Current ROC message
    ULong_t  fTimeBufferFill;   ///< Time up to which the buffer was filled
    Double_t fTimeBufferOut;    ///< Time up to which data from the buffer can be retrieved

    // --- Event status
    CbmDigi* fCurrentDigi;        ///< Pointer to current digi read from buffer

    // --- Counters
    Int_t fNofMessages;          ///< Number of processed ROC messages
    Int_t fNofEpochs;            ///< Number of processed ROC epochs messages
    Int_t fNofEvents;            ///< Number of built events
    Int_t fNofMessType[8];       ///< Number of message per message type
    Int_t fNofMessRoc[20][5];    ///< Number of messages per ROC and NXYTER
    Int_t fNofHitMsg[kNOFDETS];  ///< Number of hit messages per detector system
    Int_t fNofDigis[kNOFDETS];   ///< Number of created digis per detector system
    Int_t fNofAux;               ///< Number of AUX messages

    Bool_t fBaselineData;   ///< Flag if the data is for baseline calibration
    std::set<Int_t> fBaselineRoc; ///< List of RocIds which already signaled changin of readout status
    Bool_t fTriggeredMode; ///< Flag if data is taken in triggered mode

    std::map<Int_t, CbmROCUnpack*> fUnpackers;

    Int_t fNofBaselineDigis;  ///< 

    /** Fill the buffer from the input
     ** @param time  Maximal time of input data
     ** @return kFALSE if end of input is reached�
     **/
    Bool_t FillBuffer(ULong_t time);


    /** Get next digi from the buffer.
     ** If necessary, refill the buffer.
     ** @return Pointer to next digi object. NULL if no more available.
     **/
    CbmDigi* GetNextData();

    /** Open a new input file and 
     ** read the first message from the file
     ** @return ????
     **/
    Bool_t OpenInputFileAndGetFirstMessage();

    ClassDef(CbmSourceLmdNew, 0)
};


#endif
