/** @file CbmStsSensorTypeDssd.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 02.05.2013
 **/

#include "digitize/CbmStsSensorTypeDssdReal.h"

#include <iomanip>
#include <fstream>
#include <cstring>
#include <time.h>

#include "TMath.h"
#include "TRandom3.h"
#include "TString.h"
#include "TSystem.h"
#include "TParticlePDG.h"
#include "TDatabasePDG.h"
#include "TParticle.h"

#include "FairLogger.h"

#include "digitize/CbmStsPhysics.h"
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensorPoint.h"
#include "setup/CbmStsSensor.h"


using std::fixed;
using std::setprecision;
using std::ifstream;
using std::vector;


// --- Energy for creation of an electron-hole pair in silicon [GeV]  ------
const double kPairEnergy = 3.57142e-9;

// -----   Constructor   ---------------------------------------------------
CbmStsSensorTypeDssdReal::CbmStsSensorTypeDssdReal()
: CbmStsSensorTypeDssd(), 
    fPhysics(CbmStsPhysics::Instance()),
	    fNonUniformity(1),
	    fDiffusion(1),
	    fCrossTalk(1),
	    fLorentzShift(1)
{
}
// -------------------------------------------------------------------------



// -----   Process an MC Point  --------------------------------------------
Int_t CbmStsSensorTypeDssdReal::ProcessPoint(CbmStsSensorPoint* point,
	const CbmStsSensor* sensor) const {


    // --- Catch if parameters are not set
    if ( ! fIsSet ) {
	LOG(FATAL) << fName << ": parameters are not set!"
	    << FairLogger::endl;
	return -1;
    }


    // --- Debug
    LOG(DEBUG4) << GetName() << ": processing sensor point at ( "
	<< point->GetX1() << ", " << point->GetX2()
	<< " ) cm at " << point->GetTime() << " ns, energy loss (Geant) "
	<< point->GetELoss() << ", PID " << point->GetPid()
	<< ", By = " << point->GetBy() << " T"
	<< FairLogger::endl;

    // --- Check for being in sensitive area
    // --- Note: No charge is produced if either entry or exit point
    // --- (or both) are outside the active area. This is not an exact
    // --- description since the track may enter the sensitive area
    // --- if not perpendicular to the sensor plane. The simplification
    // --- was chosen to avoid complexity. :-)
    if ( TMath::Abs(point->GetX1()) > fDx/2. ||
	    TMath::Abs(point->GetY1()) > fDy/2. ||
	    TMath::Abs(point->GetX2()) > fDx/2. ||
	    TMath::Abs(point->GetY2()) > fDy/2. ) return 0;

    // --- Created charge signals
    Int_t nSignals = 0;

    vector <Double_t> ELossLayerArray;
    // --- Produce charge on front and back side
    nSignals += 1000 * ProduceCharge(point, 0, sensor, ELossLayerArray);
    nSignals += ProduceCharge(point, 1, sensor, ELossLayerArray);

    return nSignals;
}
// -------------------------------------------------------------------------


// -----   Produce charge on the strips   ----------------------------------
Int_t CbmStsSensorTypeDssdReal::ProduceCharge(CbmStsSensorPoint* point,
	Int_t side,
	const CbmStsSensor* sensor, vector<Double_t> &ELossLayerArray)
const {

    // --- Protect against being called without parameters being set
    if ( ! fIsSet ) LOG(FATAL) << "Parameters of sensor " << fName
	<< " are not set!" << FairLogger::endl;

    // This implementation assumes a straight trajectory in the sensor
    // and a non-uniform charge distribution along it.

    // Check for side qualifier
    if ( side < 0 || side > 1 )  {
	LOG(ERROR) << "Illegal side qualifier!" << FairLogger::endl;
	return -1;
    }

    // Total produced charge
    Double_t qtot = point->GetELoss() / kPairEnergy;
   if (side == 0) LOG(DEBUG) << "Total produced charge (from Geant) =  " << 
	point -> GetELoss() * 1.e6 << " keV" << FairLogger::endl;

    // Stereo angle and strip pitch
    Double_t tanphi = fTanStereo[side];
    Double_t pitch  = fPitch[side];
    Int_t nStrips   = fNofStrips[side];

    // Debug output
    LOG(DEBUG4) << GetName() << ": Side " << side << ", dx = " << fDx
	<< " cm, dy = " << fDy << " cm, stereo " << fStereo[side]
	<< " degrees, strips " << fNofStrips[side] << ", pitch "
	<< pitch << " mum" << FairLogger::endl;

    //coordinates of point in local coordinate system of sensor
    Double_t locX1 = point->GetX1() + 0.5 * fDx;
    Double_t locX2 = point->GetX2() + 0.5 * fDx;
    Double_t locY1 = point->GetY1() + 0.5 * fDy;
    Double_t locY2 = point->GetY2() + 0.5 * fDy;
    Double_t locZ1 = point->GetZ1();// + 0.5 * fDz;
    Double_t locZ2 = point->GetZ2();// + 0.5 * fDz;
    // Debug output
    LOG(DEBUG4) << GetName() << ": Side " << side 
	<< ", x1 = " << locX1 << " cm, x2 = " << locX2 
	<< " cm, y1 = " << locY1 << " cm, y2 = " << locY2
	<< " cm, z1 = " << locZ1 << " cm, z2 = " << locZ2
	<< " cm." << FairLogger::endl;

    // Calculate the length of trajectiry inside sensor 
    // (without taking into account the band in magnetic field)
    Double_t trajLength = sqrt ((locX2 - locX1) * (locX2 - locX1) + 
	    (locZ2 - locZ1) * (locZ2 - locZ1) + 
	    (locY2 - locY1) * (locY2 - locY1));


    // shift in (or out) coordinate due to magnetic field      
    Double_t tanLor = 0.;
    Double_t field = point -> GetBy();
    if (!fLorentzShift)   field = 0.;
    if (side == 0) {// n-side
	tanLor = 1650e-4 * field;// electrons
	if (locZ1 < 0)	locX1 += fDz * tanLor;
	if (locZ2 < 0)	locX2 += fDz * tanLor;
    }
    if (side == 1) {// p-side
	tanLor = 310e-4 * field;// holes
	if (locZ1 > 0) locX1 += fDz * tanLor;
	if (locZ2 > 0) locX2 += fDz * tanLor;
    }


    // Project point coordinates (in / out) along strips to readout (top) edge
    // Keep in mind that the SensorPoint gives coordinates with
    // respect to the centre of the active area.
    Double_t x1 = locX1	- ( fDy - locY1 ) * tanphi;
    Double_t x2 = locX2	- ( fDy - locY2 ) * tanphi;
    LOG(DEBUG4) << GetName() << ": R/O x coordinates are " << x1 << " "
	<< x2 << FairLogger::endl;

    // Calculate corresponding strip numbers
    // This can be negative or larger than the number of channels if the
    // strip does not extend to the top edge.
    //Int_t i1 = TMath::FloorNint( x1 / pitch );
    //Int_t i2 = TMath::FloorNint( x2 / pitch );
    Int_t i1 = int( x1 / pitch );
    Int_t i2 = int( x2 / pitch );
    LOG(DEBUG4) << GetName() << ": R/O strips are " << i1 << " "
	<< i2 << FairLogger::endl;

    Bool_t invert = 0;
    // --- More than one strip: sort strips
    if ( i1 > i2 ) {
	invert = 1;
	Int_t tempI = i1;
	i1 = i2;
	i2 = tempI;
	Double_t tempX = x1;
	x1 = x2;
	x2 = tempX;
	Double_t templocX = locX1;
	locX1 = locX2;
	locX2 = templocX;
	Double_t templocY = locY1;
	locY1 = locY2;
	locY2 = templocY;
	Double_t templocZ = locZ1;
	locZ1 = locZ2;
	locZ2 = templocZ;
    }
    if (invert) LOG(DEBUG4) << "Coordinates are inverted." << FairLogger::endl;

    Double_t * stripCharge = new Double_t[nStrips]; 
    memset(stripCharge, 0, nStrips * sizeof(Double_t) );
    Double_t * stripChargeCT;
    Double_t totalLossCharge = 0.;

    // thickness of trajectory layer in cm, should be > 2 mum and < 10 mum. 
    // The trajectory is divided on whole number of steps with step length about 3 mum 
    Double_t delta = trajLength / Int_t(trajLength / 3.e-4);
    LOG(DEBUG4) << "delta = " << delta << " cm" << FairLogger::endl;

    // --- Get mass and charge
    Double_t mass   = 0.;
    Double_t charge = 0.;
    Int_t pid = point->GetPid();
    // First look in TDatabasePDG
    TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(pid);
    if ( particle ) {
    	mass   = particle->Mass();
    	charge = particle->Charge() / 3.;
    } //? found in database
    else if (pid > 1000000000 && pid < 1010000000 ) {  // ion
    	pid -= 1000000000;
    	Int_t iCharge = pid / 10000;
    	pid -= iCharge * 10000;
    	charge = Double_t (iCharge);
    	mass   = Double_t ( pid / 10 );
    } //? ion
    else    // not found
    	LOG(FATAL) << GetName() << ": could not determine properties for PID "
    			       << point->GetPid() << FairLogger::endl;

    // --- Calculate kinetic energy; determine whether it is an electron
    Double_t p = point->GetP();
    Double_t eKin = TMath::Sqrt( p*p + mass*mass ) - mass;
    Bool_t isElectron = ( point->GetPid() == 11 || point->GetPid() == -11 );

    // --- Get stopping power
    Double_t dedx = fPhysics->StoppingPower(eKin, mass, charge, isElectron);

    LOG(DEBUG3) << GetName() << ": PID " << point -> GetPid() << ", mass "
	<< mass << " GeV, charge " << charge << ", Ekin " << eKin
	<< " GeV, <dE/dx> = " << 1000. * dedx << " MeV/cm "
	<< FairLogger::endl;


    Double_t chargeLayerUni = qtot / (Int_t (trajLength / delta));
    //calculate some values for thermal diffusion
    Double_t kTq, Vdepl, Vbias;
    kTq = sqrt (2. * sensor -> GetConditions().GetTemperatur() * 1.38e-4 / 1.6);
    Vdepl = sensor -> GetConditions().GetVfd();
    Vbias = sensor -> GetConditions().GetVbias();

    Int_t layer_counter = 0;
    for (Int_t iLayer = 0; iLayer < Int_t(trajLength / delta); iLayer ++) {
	if (side == 0){
	    // energy loss calculation in eV; m, Ekin - in GeV, dEdx - in GeV/cm
	    if (fNonUniformity) { 
		Double_t lossInLayer = fPhysics -> EnergyLoss(delta, mass, eKin, dedx); 
		ELossLayerArray.push_back(lossInLayer);
	    }
	    else ELossLayerArray.push_back(chargeLayerUni);
	}

	layer_counter ++;
	totalLossCharge += ELossLayerArray[iLayer];// in eV 
	// thermal diffusion calculation, result in eV
	ThermalSharing(delta, stripCharge, ELossLayerArray[iLayer], 
	    locX1, locX2, locY1, locY2, locZ1, locZ2, i1, i2, trajLength, iLayer, side, kTq, Vdepl, Vbias); 
    }
if (side == 0) LOG(DEBUG4) << "Total produced charge after energy loss = " << totalLossCharge * 1e-3 << " keV" << FairLogger::endl;
    LOG(DEBUG4) << "number of layers = " << layer_counter << ", side =  " << side << FairLogger::endl;

    if (fCrossTalk) {
	Double_t CTcoef =  sensor -> GetConditions().GetCrossTalk();
	stripChargeCT = new Double_t[nStrips];
	memset(stripChargeCT, 0, nStrips * sizeof(Double_t) );
	CrossTalk (stripCharge, stripChargeCT, nStrips, tanphi, CTcoef);
    }
    else stripChargeCT = stripCharge;

    // --- Loop over fired strips
    Int_t nSignals = 0;
    for (Int_t iStrip = 0; iStrip < nStrips; iStrip++) {
	if (stripChargeCT[iStrip] > 0){//charge > 1 eV

	    // --- Calculate charge in strip
	    // normalize registered charge to total charge from simulation - qtot
	    Double_t chargeNorm = stripChargeCT[iStrip] * qtot / totalLossCharge;// in e
	    // --- Register charge to module
	    RegisterCharge(sensor, side, iStrip, chargeNorm, 
		    (point -> GetTime()));
	    nSignals++;
	}
    } // Loop over fired strips

       //deleting pointers
    delete stripCharge;
    delete stripChargeCT;

    
    return nSignals;
}
// -------------------------------------------------------------------------


// -----   Thermal diffusion    -----------------------------
void CbmStsSensorTypeDssdReal::ThermalSharing (Double_t delta, Double_t * stripCharge, Double_t chargeLayer, 
	Double_t locX1, Double_t locX2, Double_t locY1, Double_t locY2, Double_t locZ1, Double_t locZ2,
	Int_t i1, Int_t i2, Double_t trajLength, Int_t iLayer, Int_t side, Double_t kTq, Double_t Vdepletion, Double_t Vbias) const {

    // Stereo angle and strip pitch
    Double_t tanphi = fTanStereo[side];
    Double_t pitch  = fPitch[side];
    Int_t nStrips   = fNofStrips[side];

    Double_t xLayer = locX1 + (iLayer + 0.5) * delta * (locX2 - locX1) / trajLength;
    Double_t yLayer = locY1 + (iLayer + 0.5) * delta * (locY2 - locY1) / trajLength;
    Double_t zLayer = locZ1 + (iLayer + 0.5) * delta * (locZ2 - locZ1) / trajLength;
    //Double_t zLayer = (iLayer + 0.5) * delta * fDz / trajLength;


    Double_t tau, sigmaLayer;
    if (fDiffusion) {
	if ( (side == 0 && locZ1 > 0) || (side == 1 && locZ1 < 0) ) zLayer = fDz - zLayer;  
	tau = fDz * fDz / (2. * Vdepletion) * log ((Vbias + Vdepletion) / (Vbias - Vdepletion) * 
		(1. - 2. * zLayer / fDz * Vdepletion / (Vbias + Vdepletion))); 
	sigmaLayer = kTq * sqrt(tau); 
    }

    Double_t * stripChargeSh = new Double_t[i2 - i1 + 3];
    memset(stripChargeSh, 0, (i2 - i1 + 3) * sizeof(Double_t) );
    if ( (xLayer >= 0) && (xLayer <= fDx) ){//layer inside sensor

	for (Int_t iStrip = i1; iStrip <= i2; iStrip++){

	    Int_t iStripShifted = iStrip - i1 + 1;// shift index to avoid negativ index of array 
	    Double_t chargePrev = 0., chargeNext = 0.; 

	    if ((xLayer >= iStrip * pitch + tanphi * (fDy - yLayer)) && 
		    (xLayer < (iStrip + 1) * pitch + tanphi * (fDy - yLayer))) {

		if (!fDiffusion) stripChargeSh[iStripShifted] += chargeLayer; 
		if (fDiffusion){
		    // calculate the coordianate of strip edge, where this layer is
		    Double_t xro = xLayer - (fDy - yLayer) * tanphi;//coordinate on the ro (top) edge of sensor
		    Int_t iro = TMath::FloorNint(xro / pitch);//number of ro strip
		    Double_t edgeLeft = iro * pitch + tanphi * (fDy - yLayer);
		    Double_t edgeRight = (iro + 1) * pitch + tanphi * (fDy - yLayer);

		   if ( (xLayer - 20. * 1e-4) < edgeLeft) {
			chargePrev = chargeLayer * 0.5 * (1. + TMath::Erf(( edgeLeft - xLayer) / (sqrt(2.) * sigmaLayer) ));
			if (edgeLeft >= pitch / 2. && chargePrev > 0) {stripChargeSh[iStripShifted - 1] += chargePrev;
			}
		    }
		    if ( (xLayer + 20. * 1e-4) > edgeRight ) {
			chargeNext = chargeLayer * 0.5 * TMath::Erfc( (edgeRight - xLayer) / (sqrt(2.) * sigmaLayer));
			if (edgeRight <= fDx - pitch / 2. && chargeNext > 0) {stripChargeSh[iStripShifted + 1] += chargeNext;
			}
		    } 
		    stripChargeSh[iStripShifted] += chargeLayer - chargePrev - chargeNext;
		}//if fDiffusion
	    }
	}//loop over strips
    }//layer inside sensor
    // move indexes back
    for (Int_t iStripShifted = 0; iStripShifted < (i2 - i1 + 3); iStripShifted ++){
	if (stripChargeSh[iStripShifted] > 0){
	    int iStrip = iStripShifted + i1 - 1;
	    if (iStrip < 0) iStrip += nStrips;
	    if (iStrip >= nStrips) iStrip -= nStrips;
	    stripCharge[iStrip] += stripChargeSh[iStripShifted];
	}
    }
    delete stripChargeSh;
}
// --------------------------------------------------------------------


// -----   Cross-talk   -----------------------------------------------
void CbmStsSensorTypeDssdReal::CrossTalk(Double_t * stripCharge, Double_t * stripChargeCT, 
	Int_t nStrips, Double_t tanphi, Double_t CTcoef) const {

    for (Int_t iStrip = 1; iStrip < nStrips - 1; iStrip ++) // loop over all strips except first and last one
	stripChargeCT[iStrip] = stripCharge[iStrip] * (1 - 2 * CTcoef) + 
				(stripCharge[iStrip - 1] + stripCharge[iStrip + 1]) * CTcoef;

    if (tanphi != 0){// there are cross-connections
	stripChargeCT[0] = stripCharge[0] * (1 - 2 * CTcoef) + 
			   (stripCharge[nStrips - 1] + stripCharge[1]) * CTcoef;//first strip
	stripChargeCT[nStrips - 1] = stripCharge[nStrips - 1] * (1 - 2 * CTcoef) + 
			    (stripCharge[0] + stripCharge[nStrips - 2]) * CTcoef; // last strip
    }
    if (tanphi == 0){// no cross-connections
	stripChargeCT[0]           = stripCharge[0] * (1 - CTcoef) 
				   + stripCharge[1] * CTcoef;//first strip
	stripChargeCT[nStrips - 1] = stripCharge[nStrips - 1] * (1 - CTcoef) 
		        	    + stripCharge[nStrips - 2] * CTcoef; // last strip
    }
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSensorTypeDssdReal)
