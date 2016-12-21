// -------------------------------------------------------------------------
// -----                       CbmVertex source file                   -----
// -----                  Created 28/11/05  by V. Friese               -----
// -------------------------------------------------------------------------
#include "CbmVertex.h"

#include <iomanip>
#include <sstream>
#include "FairLogger.h"

using namespace std;


// -----   Default constructor   -------------------------------------------
CbmVertex::CbmVertex() 
  : TNamed("Vertex", "Global"),
    fX(0.),
    fY(0.),
    fZ(0.),
    fChi2(0.),
    fNDF(0),
    fNTracks(0),
    fCovMatrix()
{
}
// -------------------------------------------------------------------------

// -----   Constructor with name and title   -------------------------------
CbmVertex::CbmVertex(const char* name, const char* title) 
  : TNamed(name, title),
    fX(0.),
    fY(0.),
    fZ(0.),
    fChi2(0.),
    fNDF(0),
    fNTracks(0),
    fCovMatrix()
{
}
// -------------------------------------------------------------------------
  


// -----   Constructor with all parameters   -------------------------------
CbmVertex::CbmVertex(const char* name, const char* title,
		     Double_t x, Double_t y, Double_t z, Double_t chi2,
		     Int_t ndf, Int_t nTracks, 
		     const TMatrixFSym& covMat) 
  : TNamed(name, title),
    fX(x),
    fY(y),
    fZ(z),
    fChi2(chi2),
    fNDF(ndf),
    fNTracks(nTracks),
    fCovMatrix(covMat)
{
  /*
  fTitle   = title;
  fX       = x;
  fY       = y;
  fZ       = z;
  fChi2    = chi2;
  fNDF     = ndf;
  fNTracks = nTracks;
  */
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmVertex::~CbmVertex() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmVertex::Print(Option_t*) const {
  Double_t chi2ndf;
  if (fNDF) chi2ndf = fChi2 / Double_t(fNDF);
  else chi2ndf = 0.;	       
  LOG(INFO) << "Vertex coord. (" << fX << "," << fY << "," << fZ << ") cm, "
            << "chi2/ndf = " << chi2ndf << ", " << fNTracks
            << " tracks used" << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Accessor to covariance matrix    --------------------------------
void CbmVertex::CovMatrix(TMatrixFSym& covMat) const {
	covMat = fCovMatrix;
}
// -------------------------------------------------------------------------



// -----   Accessor to covariance matrix elements   ------------------------
/*Double_t CbmVertex::GetCovariance(Int_t i, Int_t j) const {
	return fCovMatrix(i,j);
}
*/
// -------------------------------------------------------------------------



// -----   Public method SetVertex   ---------------------------------------
void CbmVertex::SetVertex(Double_t x, Double_t y, Double_t z, Double_t chi2,
			  Int_t ndf, Int_t nTracks, 
			  const TMatrixFSym& covMat) {
  fX       = x;
  fY       = y;
  fZ       = z;
  fChi2    = chi2;
  fNDF     = ndf;
  fNTracks = nTracks;
  fCovMatrix = covMat;
}
// -------------------------------------------------------------------------



// -----   Public method Reset   -------------------------------------------
void CbmVertex::Reset() {
  fX = fY = fZ = fChi2 = 0.;
  fNDF = fNTracks = 0;
  fCovMatrix.Clear();
}  
// -------------------------------------------------------------------------



// --- String output  ------------------------------------------------------
string CbmVertex::ToString() const {

  Double_t chi2ndf = ( fNDF ? fChi2 / Double_t(fNDF) : 0.);
  stringstream ss;
  ss << "Vertex: position (" << fixed << setprecision(4) << fX << ", " << fY << ", " << fZ
	 << ") cm, chi2/ndf = " << chi2ndf << ", tracks used: " << fNTracks;
   return ss.str();
}
// -------------------------------------------------------------------------



ClassImp(CbmVertex)
