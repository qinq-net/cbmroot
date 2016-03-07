/*
 * CbmRichPmt.cxx
 *
 *  Created on: Dec 16, 2015
 *      Author: slebedev
 */

#include "CbmRichPmt.h"
#include "FairLogger.h"
#include "TRandom.h"
#include "CbmRichGeoManager.h"
#include "FairLogger.h"

const Double_t CbmRichPmt::c = 2.998E8; // speed of light
const Double_t CbmRichPmt::h = 6.626E-34; // Planck constant
const Double_t CbmRichPmt::e = 1.6022E-19; // elementary charge

CbmRichPmt::CbmRichPmt()
 : fCollectionEfficiency(1.),
   fDetectorType(6),
   fLambdaMin(0.),
   fLambdaMax(0.),
   fLambdaStep(0.),
   fEfficiency(),
   fRefractiveIndex(-1.)
{
    LOG(INFO) << "CbmRichPmt init QE" << FairLogger::endl;
	InitQE();
}

CbmRichPmt::~CbmRichPmt() {

}

Bool_t CbmRichPmt::isPhotonDetected(Double_t momentum)
{
    if (fRefractiveIndex < 0.) fRefractiveIndex = CbmRichGeoManager::GetInstance().fGP->fNRefrac;
    Double_t lambda=c/fRefractiveIndex*h/e/momentum;// wavelength in nm
    if (lambda >= fLambdaMin && lambda < fLambdaMax) {
       Int_t ilambda=(Int_t)((lambda-fLambdaMin)/fLambdaStep);
       Double_t rand = gRandom->Rndm();
       if (fEfficiency[ilambda]*fCollectionEfficiency > rand ) return true;
    }
    return false;
}

void CbmRichPmt::InitQE()
{
	if (fDetectorType == 1) {
		// PMT efficiencies for Protvino-type PMT
		// corresponding range in lambda: (100nm)120nm - 700nm in steps of 20nm

		fLambdaMin = 120.;
		fLambdaMax = 700.;
		fLambdaStep = 20.;

		fEfficiency[0] = 0.216;
		fEfficiency[1] = 0.216;
		fEfficiency[2] = 0.216;
		fEfficiency[3] = 0.216;
		fEfficiency[4] = 0.216;
		fEfficiency[5] = 0.216;
		fEfficiency[6] = 0.216;
		fEfficiency[7] = 0.216;
		fEfficiency[8] = 0.216;
		fEfficiency[9] = 0.216;
		fEfficiency[10] = 0.216;
		fEfficiency[11] = 0.227;
		fEfficiency[12] = 0.23;
		fEfficiency[13] = 0.227;
		fEfficiency[14] = 0.216;
		fEfficiency[15] = 0.2;
		fEfficiency[16] = 0.176;
		fEfficiency[17] = 0.15;
		fEfficiency[18] = 0.138;
		fEfficiency[19] = 0.1;
		fEfficiency[20] = 0.082;
		fEfficiency[21] = 0.06;
		fEfficiency[22] = 0.044;
		fEfficiency[23] = 0.032;
		fEfficiency[24] = 0.022;
		fEfficiency[25] = 0.015;
		fEfficiency[26] = 0.01;
		fEfficiency[27] = 0.006;
		fEfficiency[28] = 0.004;

		fLambdaMin = 100.;
		fLambdaMax = 700.;
		fLambdaStep = 20.;

	} else if (fDetectorType == 3) {
		// quantum efficiency for CsI photocathode
		// approximately read off from fig.3 in NIM A 433 (1999) 201 (HADES)

		fLambdaMin = 130.;
		fLambdaMax = 210.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.45;
		fEfficiency[1] = 0.4;
		fEfficiency[2] = 0.35;
		fEfficiency[3] = 0.32;
		fEfficiency[4] = 0.25;
		fEfficiency[5] = 0.2;
		fEfficiency[6] = 0.1;
		fEfficiency[7] = 0.03;

	} else if (fDetectorType == 2) {
		// PMT efficiencies for Hamamatsu H8500
		// (Flat type Multianode Photomultiplier)
		// corresponding range in lambda: 260nm - 740nm in steps of 20nm

		fLambdaMin = 260.;
		fLambdaMax = 740.;
		fLambdaStep = 20.;

		fEfficiency[0] = 0.06;
		fEfficiency[1] = 0.12;
		fEfficiency[2] = 0.2;
		fEfficiency[3] = 0.22;
		fEfficiency[4] = 0.22;
		fEfficiency[5] = 0.22;
		fEfficiency[6] = 0.21;
		fEfficiency[7] = 0.2;
		fEfficiency[8] = 0.18;
		fEfficiency[9] = 0.16;
		fEfficiency[10] = 0.14;
		fEfficiency[11] = 0.11;
		fEfficiency[12] = 0.1;
		fEfficiency[13] = 0.06;
		fEfficiency[14] = 0.047;
		fEfficiency[15] = 0.03;
		fEfficiency[16] = 0.021;
		fEfficiency[17] = 0.012;
		fEfficiency[18] = 0.006;
		fEfficiency[19] = 0.0023;
		fEfficiency[20] = 0.0008;
		fEfficiency[21] = 0.00022;
		fEfficiency[22] = 0.00007;
		fEfficiency[23] = 0.00002;

	} else if (fDetectorType == 4) {
		// PMT efficiencies for Hamamatsu H8500-03
		// (Flat type Multianode Photomultiplier with UV window)
		// corresponding range in lambda: 200nm - 640nm in steps of 20nm

		fLambdaMin = 200.;
		fLambdaMax = 640.;
		fLambdaStep = 20.;

		fEfficiency[0] = 0.095;
		fEfficiency[1] = 0.13;
		fEfficiency[2] = 0.16;
		fEfficiency[3] = 0.2;
		fEfficiency[4] = 0.23;
		fEfficiency[5] = 0.24;
		fEfficiency[6] = 0.25;
		fEfficiency[7] = 0.25;
		fEfficiency[8] = 0.24;
		fEfficiency[9] = 0.24;
		fEfficiency[10] = 0.23;
		fEfficiency[11] = 0.22;
		fEfficiency[12] = 0.2;
		fEfficiency[13] = 0.16;
		fEfficiency[14] = 0.14;
		fEfficiency[15] = 0.1;
		fEfficiency[16] = 0.065;
		fEfficiency[17] = 0.045;
		fEfficiency[18] = 0.02;
		fEfficiency[19] = 0.017;
		fEfficiency[20] = 0.007;
		fEfficiency[21] = 0.0033;

	} else if (fDetectorType == 5) {
		// PMT efficiencies for Hamamatsu H8500 + WLS film
		// (Flat type Multianode Photomultiplier with UV window)
		// corresponding range in lambda: 150nm - 650nm in steps of 20nm

		fLambdaMin = 160.;
		fLambdaMax = 640.;
		fLambdaStep = 20.;

		fEfficiency[0] = 0.1;
		fEfficiency[1] = 0.2;
		fEfficiency[2] = 0.2;
		fEfficiency[3] = 0.2;
		fEfficiency[4] = 0.2;
		fEfficiency[5] = 0.2;
		fEfficiency[6] = 0.23;
		fEfficiency[7] = 0.24;
		fEfficiency[8] = 0.25;
		fEfficiency[9] = 0.25;
		fEfficiency[10] = 0.24;
		fEfficiency[11] = 0.24;
		fEfficiency[12] = 0.23;
		fEfficiency[13] = 0.22;
		fEfficiency[14] = 0.2;
		fEfficiency[15] = 0.16;
		fEfficiency[16] = 0.14;
		fEfficiency[17] = 0.1;
		fEfficiency[18] = 0.065;
		fEfficiency[19] = 0.045;
		fEfficiency[20] = 0.02;
		fEfficiency[21] = 0.017;
		fEfficiency[22] = 0.007;
		fEfficiency[23] = 0.0033;
	}

// -------------------------------------------------------------------------------------------
//   QE measured at Wuppertal University (BUW), spring 2011
// -------------------------------------------------------------------------------------------

	else if (fDetectorType == 6) {
		/**  Measured PMT efficiencies for MAPMTs (H8500D-03) at posF (BUW measurement) ##### CernOct2011 #####
		 (Flat type Multianode Photomultiplier with BA cathode + UV window)
		 corresponding range in lambda: 160nm  - 700nm in steps of 10nm */

		fLambdaMin = 160.;
		fLambdaMax = 700.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.0;
		fEfficiency[1] = 0.0;
		fEfficiency[2] = 0.0324;
		fEfficiency[3] = 0.0586;
		fEfficiency[4] = 0.0945;
		fEfficiency[5] = 0.1061;
		fEfficiency[6] = 0.1265;
		fEfficiency[7] = 0.1482;
		fEfficiency[8] = 0.1668;
		fEfficiency[9] = 0.1887;
		fEfficiency[10] = 0.2093;
		fEfficiency[11] = 0.2134;
		fEfficiency[12] = 0.2303;
		fEfficiency[13] = 0.2482;
		fEfficiency[14] = 0.2601;
		fEfficiency[15] = 0.2659;
		fEfficiency[16] = 0.2702;
		fEfficiency[17] = 0.283;
		fEfficiency[18] = 0.2863;
		fEfficiency[19] = 0.2863;
		fEfficiency[20] = 0.2884;
		fEfficiency[21] = 0.286;
		fEfficiency[22] = 0.2811;
		fEfficiency[23] = 0.2802;
		fEfficiency[24] = 0.272;
		fEfficiency[25] = 0.2638;
		fEfficiency[26] = 0.2562;
		fEfficiency[27] = 0.2472;
		fEfficiency[28] = 0.2368;
		fEfficiency[29] = 0.2218;
		fEfficiency[30] = 0.2032;
		fEfficiency[31] = 0.186;
		fEfficiency[32] = 0.1735;
		fEfficiency[33] = 0.1661;
		fEfficiency[34] = 0.1483;
		fEfficiency[35] = 0.121;
		fEfficiency[36] = 0.0959;
		fEfficiency[37] = 0.0782;
		fEfficiency[38] = 0.0647;
		fEfficiency[39] = 0.0538;
		fEfficiency[40] = 0.0372;
		fEfficiency[41] = 0.0296;
		fEfficiency[42] = 0.0237;
		fEfficiency[43] = 0.0176;
		fEfficiency[44] = 0.0123;
		fEfficiency[45] = 0.0083;
		fEfficiency[46] = 0.005;
		fEfficiency[47] = 0.003;
		fEfficiency[48] = 0.0017;
		fEfficiency[49] = 0.0008;
		fEfficiency[50] = 0.0006;
		fEfficiency[51] = 0.0003;
		fEfficiency[52] = 0.0003;
		fEfficiency[53] = 0.0002;
		fEfficiency[54] = 0.0001;

	} else if (fDetectorType == 10) {
		/**  Measured PMT efficiencies for MAPMTs (H8500D-03) at posH (BUW measurement) --dipcoated WLS film -- ##### CernOct2011 #####
		 (Flat type Multianode Photomultiplier with UV window)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.178;
		fEfficiency[1] = 0.200;
		fEfficiency[2] = 0.218;
		fEfficiency[3] = 0.222;
		fEfficiency[4] = 0.226;
		fEfficiency[5] = 0.228;
		fEfficiency[6] = 0.214;
		fEfficiency[7] = 0.210;
		fEfficiency[8] = 0.229;
		fEfficiency[9] = 0.231;
		fEfficiency[10] = 0.244;
		fEfficiency[11] = 0.253;
		fEfficiency[12] = 0.259;
		fEfficiency[13] = 0.263;
		fEfficiency[14] = 0.266;
		fEfficiency[15] = 0.277;
		fEfficiency[16] = 0.280;
		fEfficiency[17] = 0.274;
		fEfficiency[18] = 0.275;
		fEfficiency[19] = 0.270;
		fEfficiency[20] = 0.264;
		fEfficiency[21] = 0.263;
		fEfficiency[22] = 0.254;
		fEfficiency[23] = 0.246;
		fEfficiency[24] = 0.239;
		fEfficiency[25] = 0.229;
		fEfficiency[26] = 0.219;
		fEfficiency[27] = 0.207;
		fEfficiency[28] = 0.193;
		fEfficiency[29] = 0.179;
		fEfficiency[30] = 0.161;
		fEfficiency[31] = 0.149;
		fEfficiency[32] = 0.135;
		fEfficiency[33] = 0.117;
		fEfficiency[34] = 0.103;
		fEfficiency[35] = 0.082;
		fEfficiency[36] = 0.065;
		fEfficiency[37] = 0.056;
		fEfficiency[38] = 0.036;
		fEfficiency[39] = 0.030;
		fEfficiency[40] = 0.024;
		fEfficiency[41] = 0.018;
		fEfficiency[42] = 0.013;
		fEfficiency[43] = 0.009;
		fEfficiency[44] = 0.006;
		fEfficiency[45] = 0.004;
		fEfficiency[46] = 0.002;

	} else if (fDetectorType == 11) {
		/**  Measured PMT efficiencies for MAPMTs (H8500D-03) at posH (BUW measurement) --dipcoated WLS film -- ##### CernOct2011 #####
		 (Flat type Multianode Photomultiplier with UV window)		   ##### -3.8 % due to inhomogeneity #####
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.202;
		fEfficiency[1] = 0.207;
		fEfficiency[2] = 0.210;
		fEfficiency[3] = 0.214;
		fEfficiency[4] = 0.218;
		fEfficiency[5] = 0.219;
		fEfficiency[6] = 0.206;
		fEfficiency[7] = 0.202;
		fEfficiency[8] = 0.220;
		fEfficiency[9] = 0.222;
		fEfficiency[10] = 0.235;
		fEfficiency[11] = 0.243;
		fEfficiency[12] = 0.249;
		fEfficiency[13] = 0.253;
		fEfficiency[14] = 0.256;
		fEfficiency[15] = 0.266;
		fEfficiency[16] = 0.270;
		fEfficiency[17] = 0.264;
		fEfficiency[18] = 0.265;
		fEfficiency[19] = 0.260;
		fEfficiency[20] = 0.254;
		fEfficiency[21] = 0.253;
		fEfficiency[22] = 0.244;
		fEfficiency[23] = 0.237;
		fEfficiency[24] = 0.229;
		fEfficiency[25] = 0.221;
		fEfficiency[26] = 0.210;
		fEfficiency[27] = 0.199;
		fEfficiency[28] = 0.186;
		fEfficiency[29] = 0.172;
		fEfficiency[30] = 0.155;
		fEfficiency[31] = 0.143;
		fEfficiency[32] = 0.129;
		fEfficiency[33] = 0.113;
		fEfficiency[34] = 0.099;
		fEfficiency[35] = 0.079;
		fEfficiency[36] = 0.063;
		fEfficiency[37] = 0.054;
		fEfficiency[38] = 0.035;
		fEfficiency[39] = 0.028;
		fEfficiency[40] = 0.023;
		fEfficiency[41] = 0.018;
		fEfficiency[42] = 0.013;
		fEfficiency[43] = 0.009;
		fEfficiency[44] = 0.006;
		fEfficiency[45] = 0.004;
		fEfficiency[46] = 0.002;

	} else if (fDetectorType == 12) {
		/**  Measured PMT efficiencies for MAPMTs (H8500D-03) at posD (BUW measurement) ##### CernOct2011 #####
		 (Flat type Multianode Photomultiplier with UV window)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.060;
		fEfficiency[1] = 0.080;
		fEfficiency[2] = 0.096;
		fEfficiency[3] = 0.109;
		fEfficiency[4] = 0.130;
		fEfficiency[5] = 0.152;
		fEfficiency[6] = 0.172;
		fEfficiency[7] = 0.194;
		fEfficiency[8] = 0.214;
		fEfficiency[9] = 0.218;
		fEfficiency[10] = 0.235;
		fEfficiency[11] = 0.253;
		fEfficiency[12] = 0.265;
		fEfficiency[13] = 0.271;
		fEfficiency[14] = 0.275;
		fEfficiency[15] = 0.288;
		fEfficiency[16] = 0.291;
		fEfficiency[17] = 0.292;
		fEfficiency[18] = 0.294;
		fEfficiency[19] = 0.292;
		fEfficiency[20] = 0.287;
		fEfficiency[21] = 0.286;
		fEfficiency[22] = 0.278;
		fEfficiency[23] = 0.269;
		fEfficiency[24] = 0.262;
		fEfficiency[25] = 0.252;
		fEfficiency[26] = 0.242;
		fEfficiency[27] = 0.227;
		fEfficiency[28] = 0.208;
		fEfficiency[29] = 0.178;
		fEfficiency[30] = 0.170;
		fEfficiency[31] = 0.155;
		fEfficiency[32] = 0.129;
		fEfficiency[33] = 0.102;
		fEfficiency[34] = 0.083;
		fEfficiency[35] = 0.069;
		fEfficiency[36] = 0.058;
		fEfficiency[37] = 0.041;
		fEfficiency[38] = 0.033;
		fEfficiency[39] = 0.027;
		fEfficiency[40] = 0.020;
		fEfficiency[41] = 0.015;
		fEfficiency[42] = 0.010;
		fEfficiency[43] = 0.006;
		fEfficiency[44] = 0.004;
		fEfficiency[45] = 0.002;
		fEfficiency[46] = 0.001;

	} else if (fDetectorType == 13) {
		/**  Measured PMT efficiencies for R11265 (BUW measurement)
		 (Flat type Multianode Photomultiplier with UV window, SBA, 1 square inch)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.043;
		fEfficiency[1] = 0.078;
		fEfficiency[2] = 0.123;
		fEfficiency[3] = 0.146;
		fEfficiency[4] = 0.173;
		fEfficiency[5] = 0.202;
		fEfficiency[6] = 0.225;
		fEfficiency[7] = 0.253;
		fEfficiency[8] = 0.281;
		fEfficiency[9] = 0.290;
		fEfficiency[10] = 0.315;
		fEfficiency[11] = 0.344;
		fEfficiency[12] = 0.366;
		fEfficiency[13] = 0.378;
		fEfficiency[14] = 0.384;
		fEfficiency[15] = 0.400;
		fEfficiency[16] = 0.403;
		fEfficiency[17] = 0.404;
		fEfficiency[18] = 0.407;
		fEfficiency[19] = 0.403;
		fEfficiency[20] = 0.396;
		fEfficiency[21] = 0.395;
		fEfficiency[22] = 0.383;
		fEfficiency[23] = 0.370;
		fEfficiency[24] = 0.359;
		fEfficiency[25] = 0.347;
		fEfficiency[26] = 0.331;
		fEfficiency[27] = 0.310;
		fEfficiency[28] = 0.285;
		fEfficiency[29] = 0.263;
		fEfficiency[30] = 0.244;
		fEfficiency[31] = 0.232;
		fEfficiency[32] = 0.213;
		fEfficiency[33] = 0.182;
		fEfficiency[34] = 0.151;
		fEfficiency[35] = 0.126;
		fEfficiency[36] = 0.106;
		fEfficiency[37] = 0.092;
		fEfficiency[38] = 0.069;
		fEfficiency[39] = 0.060;
		fEfficiency[40] = 0.051;
		fEfficiency[41] = 0.042;
		fEfficiency[42] = 0.034;
		fEfficiency[43] = 0.026;
		fEfficiency[44] = 0.019;
		fEfficiency[45] = 0.014;
		fEfficiency[46] = 0.009;

	} else if (fDetectorType == 14) {
		/**  Measured PMT efficiencies for R11265 -- dipcoated WLS film -- (BUW measurement)
		 (Flat type Multianode Photomultiplier with UV window, SBA, 1 square inch)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.239;
		fEfficiency[1] = 0.294;
		fEfficiency[2] = 0.332;
		fEfficiency[3] = 0.351;
		fEfficiency[4] = 0.352;
		fEfficiency[5] = 0.338;
		fEfficiency[6] = 0.303;
		fEfficiency[7] = 0.286;
		fEfficiency[8] = 0.307;
		fEfficiency[9] = 0.307;
		fEfficiency[10] = 0.324;
		fEfficiency[11] = 0.340;
		fEfficiency[12] = 0.354;
		fEfficiency[13] = 0.364;
		fEfficiency[14] = 0.371;
		fEfficiency[15] = 0.390;
		fEfficiency[16] = 0.389;
		fEfficiency[17] = 0.392;
		fEfficiency[18] = 0.395;
		fEfficiency[19] = 0.393;
		fEfficiency[20] = 0.388;
		fEfficiency[21] = 0.388;
		fEfficiency[22] = 0.378;
		fEfficiency[23] = 0.367;
		fEfficiency[24] = 0.358;
		fEfficiency[25] = 0.347;
		fEfficiency[26] = 0.333;
		fEfficiency[27] = 0.310;
		fEfficiency[28] = 0.384;
		fEfficiency[29] = 0.265;
		fEfficiency[30] = 0.248;
		fEfficiency[31] = 0.238;
		fEfficiency[32] = 0.220;
		fEfficiency[33] = 0.188;
		fEfficiency[34] = 0.150;
		fEfficiency[35] = 0.123;
		fEfficiency[36] = 0.104;
		fEfficiency[37] = 0.089;
		fEfficiency[38] = 0.068;
		fEfficiency[39] = 0.058;
		fEfficiency[40] = 0.050;
		fEfficiency[41] = 0.041;
		fEfficiency[42] = 0.033;
		fEfficiency[43] = 0.025;
		fEfficiency[44] = 0.018;
		fEfficiency[45] = 0.013;
		fEfficiency[46] = 0.008;

	} else if (fDetectorType == 15) {
		/**  Measured PMT efficiencies for MAPMTs (H8500D-03) at posC (BUW measurement) ##### CernOct2012 #####
		 (Flat type Multianode Photomultiplier with UV window)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.037;
		fEfficiency[1] = 0.063;
		fEfficiency[2] = 0.103;
		fEfficiency[3] = 0.110;
		fEfficiency[4] = 0.131;
		fEfficiency[5] = 0.153;
		fEfficiency[6] = 0.172;
		fEfficiency[7] = 0.195;
		fEfficiency[8] = 0.215;
		fEfficiency[9] = 0.217;
		fEfficiency[10] = 0.232;
		fEfficiency[11] = 0.249;
		fEfficiency[12] = 0.261;
		fEfficiency[13] = 0.267;
		fEfficiency[14] = 0.271;
		fEfficiency[15] = 0.285;
		fEfficiency[16] = 0.286;
		fEfficiency[17] = 0.285;
		fEfficiency[18] = 0.287;
		fEfficiency[19] = 0.285;
		fEfficiency[20] = 0.280;
		fEfficiency[21] = 0.279;
		fEfficiency[22] = 0.272;
		fEfficiency[23] = 0.264;
		fEfficiency[24] = 0.256;
		fEfficiency[25] = 0.248;
		fEfficiency[26] = 0.239;
		fEfficiency[27] = 0.223;
		fEfficiency[28] = 0.204;
		fEfficiency[29] = 0.189;
		fEfficiency[30] = 0.177;
		fEfficiency[31] = 0.170;
		fEfficiency[32] = 0.155;
		fEfficiency[33] = 0.130;
		fEfficiency[34] = 0.105;
		fEfficiency[35] = 0.087;
		fEfficiency[36] = 0.073;
		fEfficiency[37] = 0.060;
		fEfficiency[38] = 0.041;
		fEfficiency[39] = 0.033;
		fEfficiency[40] = 0.027;
		fEfficiency[41] = 0.020;
		fEfficiency[42] = 0.015;
		fEfficiency[43] = 0.010;
		fEfficiency[44] = 0.006;
		fEfficiency[45] = 0.004;
		fEfficiency[46] = 0.003;

	} else if (fDetectorType == 16) {
		/**  Measured PMT efficiencies for MAPMTs (H8500D-03) at posC -- optimized dipcoated WLS film -- (BUW measurement) ##### CernOct2012 #####
		 (Flat type Multianode Photomultiplier with UV window)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.202;
		fEfficiency[1] = 0.240;
		fEfficiency[2] = 0.269;
		fEfficiency[3] = 0.277;
		fEfficiency[4] = 0.279;
		fEfficiency[5] = 0.273;
		fEfficiency[6] = 0.245;
		fEfficiency[7] = 0.228;
		fEfficiency[8] = 0.243;
		fEfficiency[9] = 0.243;
		fEfficiency[10] = 0.253;
		fEfficiency[11] = 0.259;
		fEfficiency[12] = 0.262;
		fEfficiency[13] = 0.263;
		fEfficiency[14] = 0.265;
		fEfficiency[15] = 0.278;
		fEfficiency[16] = 0.279;
		fEfficiency[17] = 0.281;
		fEfficiency[18] = 0.283;
		fEfficiency[19] = 0.281;
		fEfficiency[20] = 0.277;
		fEfficiency[21] = 0.275;
		fEfficiency[22] = 0.267;
		fEfficiency[23] = 0.260;
		fEfficiency[24] = 0.253;
		fEfficiency[25] = 0.245;
		fEfficiency[26] = 0.234;
		fEfficiency[27] = 0.219;
		fEfficiency[28] = 0.201;
		fEfficiency[29] = 0.187;
		fEfficiency[30] = 0.175;
		fEfficiency[31] = 0.167;
		fEfficiency[32] = 0.150;
		fEfficiency[33] = 0.124;
		fEfficiency[34] = 0.098;
		fEfficiency[35] = 0.080;
		fEfficiency[36] = 0.066;
		fEfficiency[37] = 0.055;
		fEfficiency[38] = 0.040;
		fEfficiency[39] = 0.033;
		fEfficiency[40] = 0.026;
		fEfficiency[41] = 0.020;
		fEfficiency[42] = 0.014;
		fEfficiency[43] = 0.010;
		fEfficiency[44] = 0.006;
		fEfficiency[45] = 0.004;
		fEfficiency[46] = 0.002;

	} else if (fDetectorType == 17) {
		/**  Measured PMT efficiencies for one MAPMT (H10966A-103) at posE (BUW measurement) ##### CernOct2012 #####
		 (Flat type Multianode Photomultiplier with UV window)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.007;
		fEfficiency[1] = 0.040;
		fEfficiency[2] = 0.085;
		fEfficiency[3] = 0.103;
		fEfficiency[4] = 0.130;
		fEfficiency[5] = 0.160;
		fEfficiency[6] = 0.186;
		fEfficiency[7] = 0.215;
		fEfficiency[8] = 0.244;
		fEfficiency[9] = 0.256;
		fEfficiency[10] = 0.281;
		fEfficiency[11] = 0.310;
		fEfficiency[12] = 0.332;
		fEfficiency[13] = 0.344;
		fEfficiency[14] = 0.355;
		fEfficiency[15] = 0.376;
		fEfficiency[16] = 0.382;
		fEfficiency[17] = 0.386;
		fEfficiency[18] = 0.390;
		fEfficiency[19] = 0.390;
		fEfficiency[20] = 0.387;
		fEfficiency[21] = 0.386;
		fEfficiency[22] = 0.376;
		fEfficiency[23] = 0.365;
		fEfficiency[24] = 0.356;
		fEfficiency[25] = 0.345;
		fEfficiency[26] = 0.328;
		fEfficiency[27] = 0.302;
		fEfficiency[28] = 0.278;
		fEfficiency[29] = 0.257;
		fEfficiency[30] = 0.241;
		fEfficiency[31] = 0.227;
		fEfficiency[32] = 0.191;
		fEfficiency[33] = 0.153;
		fEfficiency[34] = 0.128;
		fEfficiency[35] = 0.112;
		fEfficiency[36] = 0.098;
		fEfficiency[37] = 0.085;
		fEfficiency[38] = 0.064;
		fEfficiency[39] = 0.055;
		fEfficiency[40] = 0.047;
		fEfficiency[41] = 0.039;
		fEfficiency[42] = 0.030;
		fEfficiency[43] = 0.023;
		fEfficiency[44] = 0.017;
		fEfficiency[45] = 0.011;
		fEfficiency[46] = 0.007;

	} else if (fDetectorType == 18) {
		/**  Measured PMT efficiencies for one MAPMT (H10966A-103) at posE -- optimized dipcoated WLS film -- (BUW measurement) ##### CernOct2012 #####
		 (Flat type Multianode Photomultiplier with UV window)
		 corresponding range in lambda: 180nm  - 640nm in steps of 10nm */

		fLambdaMin = 180.;
		fLambdaMax = 640.;
		fLambdaStep = 10.;

		fEfficiency[0] = 0.241;
		fEfficiency[1] = 0.304;
		fEfficiency[2] = 0.351;
		fEfficiency[3] = 0.364;
		fEfficiency[4] = 0.368;
		fEfficiency[5] = 0.357;
		fEfficiency[6] = 0.311;
		fEfficiency[7] = 0.279;
		fEfficiency[8] = 0.299;
		fEfficiency[9] = 0.304;
		fEfficiency[10] = 0.321;
		fEfficiency[11] = 0.329;
		fEfficiency[12] = 0.336;
		fEfficiency[13] = 0.342;
		fEfficiency[14] = 0.350;
		fEfficiency[15] = 0.370;
		fEfficiency[16] = 0.374;
		fEfficiency[17] = 0.379;
		fEfficiency[18] = 0.383;
		fEfficiency[19] = 0.384;
		fEfficiency[20] = 0.381;
		fEfficiency[21] = 0.382;
		fEfficiency[22] = 0.372;
		fEfficiency[23] = 0.362;
		fEfficiency[24] = 0.354;
		fEfficiency[25] = 0.344;
		fEfficiency[26] = 0.327;
		fEfficiency[27] = 0.300;
		fEfficiency[28] = 0.275;
		fEfficiency[29] = 0.259;
		fEfficiency[30] = 0.244;
		fEfficiency[31] = 0.231;
		fEfficiency[32] = 0.195;
		fEfficiency[33] = 0.155;
		fEfficiency[34] = 0.130;
		fEfficiency[35] = 0.113;
		fEfficiency[36] = 0.097;
		fEfficiency[37] = 0.083;
		fEfficiency[38] = 0.065;
		fEfficiency[39] = 0.055;
		fEfficiency[40] = 0.046;
		fEfficiency[41] = 0.038;
		fEfficiency[42] = 0.030;
		fEfficiency[43] = 0.022;
		fEfficiency[44] = 0.016;
		fEfficiency[45] = 0.011;
		fEfficiency[46] = 0.007;

	} else if (fDetectorType == 0) {
		//   ideal detector
		fLambdaMin = 160.;
		fLambdaMax = 640.;
		fLambdaStep = 20.;

		fEfficiency[0] = 0.99;
		fEfficiency[1] = 0.99;
		fEfficiency[2] = 0.99;
		fEfficiency[3] = 0.99;
		fEfficiency[4] = 0.99;
		fEfficiency[5] = 0.99;
		fEfficiency[6] = 0.99;
		fEfficiency[7] = 0.99;
		fEfficiency[8] = 0.99;
		fEfficiency[9] = 0.99;
		fEfficiency[10] = 0.99;
		fEfficiency[11] = 0.99;
		fEfficiency[12] = 0.99;
		fEfficiency[13] = 0.99;
		fEfficiency[14] = 0.99;
		fEfficiency[15] = 0.99;
		fEfficiency[16] = 0.99;
		fEfficiency[17] = 0.99;
		fEfficiency[18] = 0.99;
		fEfficiency[19] = 0.99;
		fEfficiency[20] = 0.99;
		fEfficiency[21] = 0.99;
		fEfficiency[22] = 0.99;
		fEfficiency[23] = 0.99;

	} else {
		LOG(ERROR) << "ERROR: photodetector type not specified" << FairLogger::endl;

		fLambdaMin = 100.;
		fLambdaMax = 100.;
		fLambdaStep = 100.;

		fEfficiency[0] = 0.;
	}
}

