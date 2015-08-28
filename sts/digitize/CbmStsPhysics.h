/** @file CbmStsPhysics.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 02.12.2014
 ** @date 18.08.2015
 **/

#ifndef CBMSTSPHYSICS_H
#define CBMSTSPHYSICS_H 1


#include "Rtypes.h"
#include "TObject.h"
#include <iostream>
#include <map>


using std::map;


/** @class CbmStsPhysics
 ** @brief Auxiliary class for simulating physics processes in Silicon
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 02.12.2014
 */
class CbmStsPhysics : public TObject {

	public:

		/** Destructor **/
		virtual ~CbmStsPhysics();



		/** Diffusion width as function of z
		 ** @param z           Distance from p side [cm]
		 ** @param d           Thickness of sensor [cm]
		 ** @param vBias       Bias voltage [V]
		 ** @param vFd         Full depletion voltage [V]
		 ** @param temperature Temperature [K]
		 ** @param chargeType  0 = electron, 1 = hole
		 ** @value Drift time [s]
		 **
		 ** Calculates the diffusion width (sigma) for a charge drifting
		 ** from z to the readout (z = 0 for hole, z = d for electrons).
		 **
		 ** For the reference to the formulae, see the STS digitiser note.
		 **/
		static Double_t DiffusionWidth(Double_t z, Double_t d, Double_t vBias,
				                           Double_t vFd, Double_t temperature,
				                           Int_t chargeType);


		/** Electric field in a silicon sensor as function of z
		 ** @param vBias  Bias voltage [V]
		 ** @param vFd    Full depletion voltage [V]
		 ** @param dZ     Thickness of sensor [cm]
		 ** @param z      z coordinate, measured from the front side [cm]
		 ** @value        z component of electric field [V/cm]
		 **/
		static Double_t ElectricField(Double_t vBias, Double_t vFd,
				                          Double_t dZ, Double_t z);


  	/** Energy loss in a Silicon layer
		 ** @param dz    Layer thickness [cm]
		 ** @param mass  Particle mass [GeV]
		 ** @param eKin  Kinetic energy [GeV]
		 ** @param dedx  Average specific energy loss [GeV/cm]
		 ** @value Energy loss in the layer [GeV]
		 **
		 ** The energy loss is sampled from the Urban fluctuation model
		 ** described in the GEANT3 manual (PHYS333 2.4, pp. 262-264).
		 */
		Double_t EnergyLoss(Double_t dz, Double_t mass,
				                Double_t eKin, Double_t dedx) const;


		/** Energy loss model
		 ** @value Flag for energy loss model
		 **
		 ** 0 = ideal (all energy loss in one point)
		 ** 1 = simple (uniform energy loss)
		 ** 2 = realistic (energy loss fluctuations)
		 **/
		Int_t EnergyLossModel() const { return fEnergyLossModel; }


		/** Atomic charge of Silicon
		 ** @value Atomic charge of Silicon [e]
		 **/
		static Double_t GetSiCharge() { return fgkSiCharge; }


		/** Accessor to singleton instance
		 ** @value  Pointer to singleton instance
		 ** Will instantiate a singleton object if not yet existing. **/
		static CbmStsPhysics* Instance();


		/** Particle charge from PDG particle ID
		 ** @param pid   PID (PDG code)
		 ** @value particle charge [e]
		 **
		 ** For particles in the TDataBasePDG, the charge is taken from there.
		 ** For ions, it is calculated following the PDG code convention.
		 ** If not found,  is returned.
		 **/
		static Double_t ParticleCharge(Int_t pid);


		/** Particle mass from PDG particle ID
		 ** @param pid   PID (PDG code)
		 ** @value particle mass [GeV]
		 **
		 ** For particles in the TDataBasePDG, the mass is taken from there.
		 ** For ions, it is calculated following the PDG code convention.
		 ** If not found, zero is returned.
		 **/
		static Double_t ParticleMass(Int_t pid);


		/** Set the physical processes
		 ** @param energyLoss    Energy loss model   (default: 2)
		 ** @param lorentzShift  LorentzShift        (default:kTRUE)
		 ** @param diffusion     Charge diffusion    (default: kTRUE)
		 ** @param crossTalk     Cross talk between strips (default: kTRUE)
		 **
		 ** For the energy loss model, 0 means ideal (all energy loss in the
		 ** midpoint of the trajectory), 1 is simple (uniform energy loss),
		 ** 2 is realistic (energy loss fluctuations).
		 ** By default, all processes are on, and the energy loss model is
		 ** realistic.
		 */
		void SetProcesses(Int_t energyLoss,
				              Bool_t lorentzShift = kTRUE,
				              Bool_t diffusion = kTRUE,
				              Bool_t crossTalk = kTRUE) {
			fEnergyLossModel = energyLoss;
			fUseLorentzShift = lorentzShift;
			fUseDiffusion    = diffusion;
			fUseCrossTalk    = crossTalk;
		}


		/** Stopping power (average specific energy loss) in Silicon
		 ** @param eKin  Kinetic energy pf the particle [GeV]
		 ** @param pid   Particle ID (PDG code)
		 ** @value Stopping power [GeV/cm]
		 **
		 ** This function calculates the stopping power
		 ** (average specific energy loss) in Silicon of a particle
		 ** with given mass and charge. For an unknown pid, null is returned.
		 **/
		Double_t StoppingPower(Double_t eKin, Int_t pid);


		/** Stopping power in Silicon
		 ** @param energy      Energy of particle [GeV]
		 ** @param mass        Particle mass [GeV]
		 ** @param charge      Electric charge [e]
		 ** @param isElectron  kTRUE if electron, kFALSE else
		 ** @value             Stopping power [GeV/cm]
		 **
		 ** This function calculates the stopping power
		 ** (average specific energy loss) in Silicon of a particle
		 ** with given mass and charge.
		 **/
		Double_t StoppingPower(Double_t energy, Double_t mass,
				                   Double_t charge, Bool_t isElectron);


		/** Accessors to physics flags **/
		Bool_t UseLorentzShift() const { return fUseLorentzShift; }
		Bool_t UseDiffusion()    const { return fUseDiffusion; }
		Bool_t UseCrossTalk()    const { return fUseCrossTalk; }


	private:

		/** Constructor **/
		CbmStsPhysics();

		/** Copy constructor (not implemented to avoid being executed) **/
    CbmStsPhysics(const CbmStsPhysics&);

    /** Assignment operator (not implemented to avoid being executed) **/
    CbmStsPhysics operator=(const CbmStsPhysics&);

    /** Pointer to singleton instance **/
		static CbmStsPhysics* fgInstance;

		/** Flag for energy loss calculation
		 ** 0 = ideal (all charge is created in the mid-point of the trajectory)
		 ** 1 = simple (uniform charge distribution along the trajectory)
		 ** 2 = realistic (charge fluctuations along the trajectory) [default]
		 **/
		Int_t fEnergyLossModel;

		// --- Flag for physical processes
		Bool_t fUseLorentzShift;  ///< Lorentz Shift on/off [default: on]
		Bool_t fUseDiffusion;     ///< Charge diffusion on/off [default: on]
		Bool_t fUseCrossTalk;     ///< Cross talk on/off [default: on]

		// --- Physical constants
		static const Double_t fgkSiCharge;     ///< Silicon atomic charge number
		static const Double_t fgkSiDensity;    ///< Silicon density [g/cm^3]
		static const Double_t fgkProtonMass;   ///< proton mass [GeV]

		// --- Parameters for the Urban model
		Double_t fUrbanI;     ///< Urban model: mean ionisation potential of Si
		Double_t fUrbanE1;    ///< Urban model: first atomic energy level
		Double_t fUrbanE2;    ///< Urban model: second atomic energy level
		Double_t fUrbanF1;    ///< Urban model: oscillator strength first level
		Double_t fUrbanF2;    ///< Urban model: oscillator strength second level
		Double_t fUrbanEmax;  ///< Urban model: cut-off energy (delta-e threshold)
		Double_t fUrbanR;     ///< Urban model: weight parameter excitation/ionisation

		// --- Data tables for stopping power
		map<Double_t, Double_t> fStoppingElectron;  ///< E [GeV] -> <-dE/dx> [GeV*g/cm^2]
		map<Double_t, Double_t> fStoppingProton  ;  ///< E [GeV] -> <-dE/dx> [GeV*g/cm^2]


		/** Interpolate a stopping power value from the data table
		 ** @param eEquiv  Equivalent kinetic energy [GeV]
		 ** @param table   Reference to data map (fStoppingElectron or fStoppingProton)
		 ** @value Interpolated value from data table [MeV*cm^2/g]
		 **
		 ** The eEquiv is below the tabulated range, the first table value is
		 ** returned; if it is above the range, the last value is returned.
		 **/
		Double_t InterpolateDataTable(Double_t eKin, map<Double_t, Double_t>& table);


		/** Read data tables from files **/
		void ReadDataTables();


		/** Calculate the parameters for the Urban model
		 ** @param z  atomic charge of material element
		 **
		 ** The parameters are set according to the GEANT3 manual (PHYS332 and PHYS333)
		 **/
		void SetUrbanParameters(Double_t z);


		ClassDef(CbmStsPhysics,1);

};

#endif /* CBMSTSPHYSICS_H_ */
