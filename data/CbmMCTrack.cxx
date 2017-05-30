// -------------------------------------------------------------------------
// -----                      CbmMCTrack source file                   -----
// -----                  Created 03/08/04  by V. Friese               -----
// -------------------------------------------------------------------------
#include "CbmMCTrack.h"

#include "FairLogger.h"

#include <sstream>

#include "TMCProcess.h"
#include "TParticle.h"
#ifndef ROOT_TParticlePDG
 #include "TParticlePDG.h"
#endif
#ifndef ROOT_TDatabasePDG
 #include "TDatabasePDG.h"
#endif

using std::stringstream;


// -----   Default constructor   -------------------------------------------
CbmMCTrack::CbmMCTrack() 
  : TObject(),
    fProcessId(kPNoProcess),
    fPdgCode(0),
    fMotherId(-1),
    fPx(0.),
    fPy(0.),
    fPz(0.),
    fE(-1.),
    fStartX(0.),
    fStartY(0.),
    fStartZ(0.),
    fStartT(0.),
    fNPoints(0)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMCTrack::CbmMCTrack(Int_t pdgCode, Int_t motherId, Double_t px,
		       Double_t py, Double_t pz, Double_t x, Double_t y,
		       Double_t z, Double_t t, Int_t nPoints = 0) 
  : TObject(),
    fProcessId(kPNoProcess),
    fPdgCode(pdgCode),
    fMotherId(motherId),
    fPx(px),
    fPy(py),
    fPz(pz),
    fE(-1.),
    fStartX(x),
    fStartY(y),
    fStartZ(z),
    fStartT(t),
    fNPoints(0)
{
  if (nPoints >= 0) fNPoints = nPoints;
  //  else              fNPoints = 0;
}
// -------------------------------------------------------------------------



// -----   Copy constructor   ----------------------------------------------
CbmMCTrack::CbmMCTrack(const CbmMCTrack& track) 
  : TObject(track),
    fProcessId(track.fProcessId),
    fPdgCode(track.fPdgCode),
    fMotherId(track.fMotherId),
    fPx(track.fPx),
    fPy(track.fPy),
    fPz(track.fPz),
    fE(track.GetEnergy()),
    fStartX(track.fStartX),
    fStartY(track.fStartY),
    fStartZ(track.fStartZ),
    fStartT(track.fStartT),
    fNPoints(track.fNPoints)
{ 
  //  *this = track;
}
// -------------------------------------------------------------------------



// -----   Constructor from TParticle   ------------------------------------
CbmMCTrack::CbmMCTrack(TParticle* part) 
  : TObject(),
    fProcessId(part->GetUniqueID()),
    fPdgCode(part->GetPdgCode()),
    fMotherId(part->GetMother(0)),
    fPx(part->Px()),
    fPy(part->Py()),
    fPz(part->Pz()),
    fE(part->Energy()),
    fStartX(part->Vx()),
    fStartY(part->Vy()),
    fStartZ(part->Vz()),
    fStartT(part->T()*1e09),
    fNPoints(0)
{
}
// -------------------------------------------------------------------------


  
// -----   Destructor   ----------------------------------------------------
CbmMCTrack::~CbmMCTrack() { }
// -------------------------------------------------------------------------



// -----   Public method GetMass   -----------------------------------------
Double_t CbmMCTrack::GetMass() const {
  if ( TDatabasePDG::Instance() ) {
    TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(fPdgCode);
    if ( particle ) return particle->Mass();
    else return 0.;
  }
  return 0.;
}
// -------------------------------------------------------------------------



// -----   Public method GetCharge   ---------------------------------------
Double_t CbmMCTrack::GetCharge() const {
  if ( TDatabasePDG::Instance() ) {
    TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(fPdgCode);
    if ( particle ) return particle->Charge();
    else return 0.;
  }
  return 0.;
}
// -------------------------------------------------------------------------

// -----   Public method GetRapidity   -------------------------------------
Double_t CbmMCTrack::GetRapidity() const {
  Double_t e = GetEnergy();
  Double_t y = 0.5 * TMath::Log( (e+fPz) / (e-fPz) );
  return y;
}
// -------------------------------------------------------------------------



// -----   Public method GetNPoints   --------------------------------------
Int_t CbmMCTrack::GetNPoints(ECbmModuleId detId) const {
  if      ( detId == kRef  ) return (  fNPoints &   1);
  else if ( detId == kMvd  ) return ( (fNPoints & ( 7 <<  1) ) >>  1);
  else if ( detId == kSts  ) return ( (fNPoints & (31 <<  4) ) >>  4);
  else if ( detId == kRich ) return ( (fNPoints & ( 1 <<  9) ) >>  9);
  else if ( detId == kMuch ) return ( (fNPoints & (31 << 10) ) >> 10);
  else if ( detId == kTrd  ) return ( (fNPoints & (31 << 15) ) >> 15);
  else if ( detId == kTof  ) return ( (fNPoints & (15 << 20) ) >> 20);
  else if ( detId == kEcal ) return ( (fNPoints & ( 1 << 24) ) >> 24);
  else if ( detId == kPsd  ) return ( (fNPoints & ( 1 << 25) ) >> 25);
  else {
    LOG(ERROR) << "GetNPoints: Unknown detector ID "
	       << detId << FairLogger::endl;
    return 0;
  }
}
// -------------------------------------------------------------------------



// -----   Public method SetNPoints   --------------------------------------
void CbmMCTrack::SetNPoints(Int_t iDet, Int_t nPoints) {

  if ( iDet == kRef ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 1 ) nPoints = 1;
    fNPoints = ( fNPoints & ( ~ 1 ) )  |  nPoints;
  }

  else if ( iDet == kMvd ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 7 ) nPoints = 7;
    fNPoints = ( fNPoints & ( ~ (  7 <<  1 ) ) )  |  ( nPoints <<  1 );
  }

  else if ( iDet == kSts ) {
    if      ( nPoints <  0 ) nPoints =  0;
    else if ( nPoints > 31 ) nPoints = 31;
    fNPoints = ( fNPoints & ( ~ ( 31 <<  4 ) ) )  |  ( nPoints <<  4 );
  }

  else if ( iDet == kRich ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 1 ) nPoints = 1;
    fNPoints = ( fNPoints & ( ~ (  1 <<  9 ) ) )  |  ( nPoints <<  9 );
  }

  else if ( iDet == kMuch ) {
    if      ( nPoints <  0 ) nPoints =  0;
    else if ( nPoints > 31 ) nPoints = 31;
    fNPoints = ( fNPoints & ( ~ ( 31 << 10 ) ) )  |  ( nPoints << 10 );
  }

  else if ( iDet == kTrd ) {
    if      ( nPoints <  0 ) nPoints =  0;
    else if ( nPoints > 31 ) nPoints = 31;
    fNPoints = ( fNPoints & ( ~ ( 31 << 15 ) ) )  |  ( nPoints << 15 );
  }

  else if ( iDet == kTof ) {
    if      ( nPoints <  0 ) nPoints =  0;
    else if ( nPoints > 15 ) nPoints = 15;
    fNPoints = ( fNPoints & ( ~ ( 15 << 20 ) ) )  |  ( nPoints << 20 );
  }

  else if ( iDet == kEcal ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 1 ) nPoints = 1;
    fNPoints = ( fNPoints & ( ~ (  1 << 24 ) ) )  |  ( nPoints << 24 );
  }

  else if ( iDet == kPsd ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 1 ) nPoints = 1;
    fNPoints = ( fNPoints & ( ~ (  1 << 25 ) ) )  |  ( nPoints << 25 );
  }

  else LOG(ERROR) << "Unknown detector ID "
		  << iDet << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   String output   -------------------------------------------------
std::string CbmMCTrack::ToString() const
{
   stringstream ss;
   ss << "MCTrack: mother  " << fMotherId
   	  << ", GeantProcess " << TMCProcessName[fProcessId]
      << ", Type " << fPdgCode << ", momentum (" << fPx << ", "
      << fPy << ", " << fPz << ") GeV" << std::endl;
   ss << "       Ref " << GetNPoints(kRef)
	  	<< ", MVD " << GetNPoints(kMvd)
	    << ", STS " << GetNPoints(kSts)
	    << ", RICH " << GetNPoints(kRich)
	    << ", MUCH " << GetNPoints(kMuch)
	    << ", TRD " << GetNPoints(kTrd)
	    << ", TOF " << GetNPoints(kTof)
	    << ", ECAL " << GetNPoints(kEcal)
	    << ", PSD " << GetNPoints(kPsd) << std::endl;
   return ss.str();
}
// -------------------------------------------------------------------------


ClassImp(CbmMCTrack)
