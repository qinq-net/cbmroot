#include "CbmTrbRawMessage.h"

// -----   Default constructor   -------------------------------------------
CbmTrbRawMessage::CbmTrbRawMessage()
  : CbmRawMessage(),
    fTDCfine(0),
    fEdge(kFALSE),
    fCorrection(0.)
{
}

// ------ Constructor -----------------------------------------------------
CbmTrbRawMessage::CbmTrbRawMessage(Int_t FlibLink, Int_t FebId,
                                   Int_t ChannelId,
                                   Int_t EpochMarker, Int_t CoarseTime,
                                   Int_t FineTime, Bool_t Edge, Double_t Correction)
  : CbmRawMessage(FlibLink, FebId, ChannelId, EpochMarker, CoarseTime),
    fTDCfine(FineTime),
    fEdge(Edge),
    fCorrection(Correction)
{
}

// -------------------------------------------------------------------------

void CbmTrbRawMessage::CopyMessage(CbmTrbRawMessage* source)
{
  /* From CbmRawMessage */
  fEquipmentID = source->GetEquipmentID();
  fSourceAddress = source->GetSourceAddress();
  fChannelID = source->GetChannelID();
  fEpochMarker = source->GetEpochMarker();
  fTime = source->GetTime();

  /* From CbmTrbRawMessage */
  fTDCfine = source->GetTDCfine();
  fEdge = source->GetEdge();
  fCorrection = source->GetCorr();

}

/*
 * For information see comments in class CbmTrbCalibrator
 * There you can find the info about how to compute full time from
 * epoch, coarse, fine and correction.
 */
ULong_t CbmTrbRawMessage::GetFullTime()
{
  Double_t coarseUnit = 5.;
  Double_t epochUnit = coarseUnit * 0x800;

  ULong_t time = fEpochMarker * epochUnit + fTime * coarseUnit + fCorrection;

  return time;
}

ClassImp(CbmTrbRawMessage)
