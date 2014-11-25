#ifndef CBMTRBEDGEMATCHER_H
#define CBMTRBEDGEMATCHER_H

#include <map>

#include "FairTask.h"

//#include "PossibleLeadingEdges.h"

class TH1D;
class TClonesArray;

class CbmTrbEdgeMatcher : public FairTask
{
public: // methods

   CbmTrbEdgeMatcher();
   ~CbmTrbEdgeMatcher();

   virtual InitStatus Init();
   virtual void Exec(Option_t* option);
   virtual void FinishEvent();
   virtual void FinishTask();

private: // methods

   void AddPossibleLeadingEdge(unsigned int tdcId, unsigned int lChannel, unsigned int lEpoch, unsigned int lDataWord);

   // return true if the corresponding leading edge found
   bool FindLeadingEdge(unsigned int tdcId, unsigned int lChannel,
                                 unsigned int tEpoch, unsigned int tCoarse,
                                 unsigned int* lEpoch, unsigned int* lWord);

   void CreateLeadingEdgeOnlyDigi(UInt_t tdcId, UInt_t lepoch, UInt_t lword);

   Double_t GetFullTime(UInt_t tdcId, UInt_t channel, UInt_t epoch, UInt_t coarse, UInt_t fine);

private: // data members

   TClonesArray* fTrbRawHits; // Input raw hits from the unpacker
   TClonesArray* fRichTrbDigi; // Output pairs (if possible) of leading and trailing edges


   unsigned int BUFSIZE;

   // 33 channels for each tdc * BUFSIZE edges stored for each channel * 2 (e+tw)
   std::map<unsigned int, unsigned int*> tdcIdToStoredEdges;
   std::map<unsigned int, unsigned int> circularCounters;


   TH1D* fhTtimeMinusLtime[68]; // for each possible TDCid

   ClassDef(CbmTrbEdgeMatcher,1)
};

#endif // CBMTRBEDGEMATCHER_H
