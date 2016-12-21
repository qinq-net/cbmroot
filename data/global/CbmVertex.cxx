/** @file CbmVertex.cxx
 ** @author V.Friese <v.friese@gsi.de>
 **/

#include "CbmVertex.h"

#include <iomanip>
#include <sstream>
#include "FairLogger.h"

using namespace std;

// -----   Default constructor   -------------------------------------------
CbmVertex::CbmVertex() :
        TNamed("Vertex", "Global"),
        fX(0.),
        fY(0.),
        fZ(0.),
        fChi2(0.),
        fNDF(0),
        fNTracks(0),
        fCovMatrix() {
}
// -------------------------------------------------------------------------

// -----   Constructor with name and title   -------------------------------
CbmVertex::CbmVertex(const char* name, const char* title) :
        TNamed(name, title),
        fX(0.),
        fY(0.),
        fZ(0.),
        fChi2(0.),
        fNDF(0),
        fNTracks(0),
        fCovMatrix() {
}
// -------------------------------------------------------------------------

// -----   Constructor with all parameters   -------------------------------
CbmVertex::CbmVertex(const char* name, const char* title, Double_t x,
                     Double_t y, Double_t z, Double_t chi2, Int_t ndf,
                     Int_t nTracks, const TMatrixFSym& covMat) :
        TNamed(name, title),
        fX(x),
        fY(y),
        fZ(z),
        fChi2(chi2),
        fNDF(ndf),
        fNTracks(nTracks),
        fCovMatrix(covMat) {
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmVertex::~CbmVertex() {
}
// -------------------------------------------------------------------------

// -----   Set vertex parameters   -----------------------------------------
void CbmVertex::SetVertex(Double_t x, Double_t y, Double_t z, Double_t chi2,
                          Int_t ndf, Int_t nTracks,
                          const TMatrixFSym& covMat) {
  fX = x;
  fY = y;
  fZ = z;
  fChi2 = chi2;
  fNDF = ndf;
  fNTracks = nTracks;
  fCovMatrix = covMat;
}
// -------------------------------------------------------------------------

// -----   Reset all parameters   ------------------------------------------
void CbmVertex::Reset() {
  fX = fY = fZ = fChi2 = 0.;
  fNDF = fNTracks = 0;
  fCovMatrix.Clear();
}
// -------------------------------------------------------------------------

// --- String output  ------------------------------------------------------
string CbmVertex::ToString() const {
  Double_t chi2ndf = ( fNDF ? fChi2 / Double_t(fNDF) : 0. );
  stringstream ss;
  ss << "Vertex: position (" << fixed << setprecision(4) << fX << ", " << fY
     << ", " << fZ << ") cm, chi2/ndf = " << chi2ndf << ", tracks used: "
     << fNTracks;
  return ss.str();
}
// -------------------------------------------------------------------------

ClassImp(CbmVertex)
