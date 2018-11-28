#ifndef CBMTSUNPACKTESTMINIRICH_H
#define CBMTSUNPACKTESTMINIRICH_H

#include "CbmTSUnpack.h" // mother class

class TClonesArray;
class CbmTrbRawMessage;

class CbmTSUnpackTestMiniRich : public CbmTSUnpack
{
public:
	CbmTSUnpackTestMiniRich();
	virtual ~CbmTSUnpackTestMiniRich();

	virtual Bool_t Init();

#ifndef __CINT__
	virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif

	virtual void Reset();

	virtual void Finish();

	virtual void SetParContainers() {;}

	virtual void FillOutput(CbmDigi*) {;}

private:

	void ProcessMicroslice(size_t const size, void const * const ptr);

	/**
	 * Including header
	 */
	Int_t ProcessTRBevent(size_t const size, void const * const ptr);

	/**
	 * Including header
	 * Return number of processed bytes
	 */
	Int_t ProcessTRBsubevent(size_t const size, void const * const ptr);

	/**
	 * Including TDC header, but not including TRB subsubevent header
	 * Return number of processed bytes
	 */
	Int_t ProcessTRBsubsubevent(size_t const size, void const * const ptr);

	Int_t ProcessCTSsubevent(size_t const size, void const * const ptr);

	Int_t ProcessSKIPsubevent(size_t const size, void const * const ptr);

	void ProcessTDCword(void const * const ptr);

	/**
	 * Write unpacked edge into the output collection
	 */
	void WriteEdge(CbmTrbRawMessage* p_obj);

	/**
	 * Write unpacked edge into the output collection
	 */
	void WriteEdge(Int_t febID, Int_t ch, Int_t edge, Int_t epoch, Int_t coarse, Int_t fine);

private:

	Int_t fEventsCounter;

	Int_t fCurMSid;
	Int_t fGwordCnt; // global word counter within current microslice

	Bool_t fInSubSubEvent;

	UInt_t fCurEpochCounter;
	Int_t fTRBeventSize1;
	Int_t fTRBeventSize2;
	Int_t fTriggerType;
	Int_t fSubEvId;
	Int_t fTriggerNum;
	Int_t fSubEvSize;
	Int_t fHubId;
	Int_t fSubSubEvSize;
	Int_t fSubSubEvId;

	// Output collection
	TClonesArray* fTrbRawMessages;

	ClassDef(CbmTSUnpackTestMiniRich, 1);
};

#endif // CBMTSUNPACKTESTMINIRICH_H
