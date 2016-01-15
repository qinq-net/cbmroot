#include <iostream>
#include <stdlib.h>

void tiltPoint()
{
	Double_t X=-6.837481, Y=127.064519, Z=210.256706;
	Double_t detXOrig=51.000000, detYOrig=135.196678, detZOrig=207.021121;
	Double_t XpmtTilt=32.762850, YpmtTilt=135.196679, ZpmtTilt=210.678286;
	Double_t theta1=0., phi1=-5.;
	Double_t theta=theta1*TMath::Pi()/180., phi=phi1*TMath::Pi()/180.;
	Double_t xDet1=0., yDet1=0., zDet1=0.;
	Double_t xDet2=0., yDet2=0., zDet2=0.;
	Double_t xDet3=0., yDet3=0., zDet3=0.;

	// Calculations from CbmRichHitProducer::tiltPoint
	if (X>0 && Y>0) {
		xDet1 = X*TMath::Cos(phi) + Z*TMath::Sin(phi) - detZOrig*TMath::Sin(phi);
		yDet1 = -X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) + Z*TMath::Sin(theta)*TMath::Cos(phi);
		zDet1 = -X*TMath::Cos(theta)*TMath::Sin(phi) - Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
	}
	if (X>0 && Y<0) {
		xDet1 = X*TMath::Cos(phi) + inPos->Z()*TMath::Sin(phi) - detZOrig*TMath::Sin(phi);
		yDet1 = X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) - Z*TMath::Sin(theta)*TMath::Cos(phi);
		zDet1 = -X*TMath::Cos(theta)*TMath::Sin(phi) + Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
	}
	if (X<0 && Y<0) {
		xDet1 = X*TMath::Cos(phi) - Z*TMath::Sin(phi) + detZOrig*TMath::Sin(phi);
		yDet1 = -X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) - Z*TMath::Sin(theta)*TMath::Cos(phi);
		zDet1 = X*TMath::Cos(theta)*TMath::Sin(phi) + Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
	}
	if (X<0 && Y>0) {
		xDet1 = X*TMath::Cos(phi) - Z*TMath::Sin(phi) + detZOrig*TMath::Sin(phi);
		yDet1 = X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) + Z*TMath::Sin(theta)*TMath::Cos(phi);
		zDet1 = X*TMath::Cos(theta)*TMath::Sin(phi) - Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
	}

	// New calculations 1
	if (X>0 && Y>0) {
                xDet2 = X*TMath::Cos(phi) + Z*TMath::Sin(phi) - detZOrig*TMath::Sin(phi);
                yDet2 = X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) - Z*TMath::Sin(theta)*TMath::Cos(phi);
                zDet2 = -X*TMath::Cos(theta)*TMath::Sin(phi) + Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
        }
        if (X>0 && Y<0) {
                xDet2 = X*TMath::Cos(phi) + inPos->Z()*TMath::Sin(phi) - detZOrig*TMath::Sin(phi);
                yDet2 = -X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) + Z*TMath::Sin(theta)*TMath::Cos(phi);
                zDet2 = -X*TMath::Cos(theta)*TMath::Sin(phi) - Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
        }
        if (X<0 && Y<0) {
                xDet2 = X*TMath::Cos(phi) - Z*TMath::Sin(phi) + detZOrig*TMath::Sin(phi);
                yDet2 = X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) + Z*TMath::Sin(theta)*TMath::Cos(phi);
                zDet2 = X*TMath::Cos(theta)*TMath::Sin(phi) - Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
        }
        if (X<0 && Y>0) {
                xDet2 = X*TMath::Cos(TMath::Abs(phi)) - Z*TMath::Sin(TMath::Abs(phi)) + detZOrig*TMath::Sin(TMath::Abs(phi));
                yDet2 = -X*TMath::Sin(TMath::Abs(theta))*TMath::Sin(TMath::Abs(phi)) + Y*TMath::Cos(TMath::Abs(theta)) - Z*TMath::Sin(TMath::Abs(theta))*TMath::Cos(TMath::Abs(phi));
                zDet2 = X*TMath::Cos(TMath::Abs(theta))*TMath::Sin(TMath::Abs(phi)) + Y*TMath::Sin(TMath::Abs(theta)) + Z*TMath::Cos(TMath::Abs(theta))*TMath::Cos(TMath::Abs(phi));
	}

	// New calculations 2
	if (X>0 && Y>0) {
                xDet3 = TMath::Cos(phi)*(X-XpmtTilt) + TMath::Sin(phi)*TMath::Sin(theta)*(Y-YpmtTilt) - TMath::Sin(phi)*TMath::Cos(theta)*(Z-ZpmtTilt);
                yDet3 = TMath::Cos(theta)*(Y-YpmtTilt) + TMath::Sin(theta)*(Z-ZpmtTilt);
                zDet3 = TMath::Sin(phi)*(X-XpmtTilt) - TMath::Cos(phi)*TMath::Sin(theta)*(Y-YpmtTilt) + TMath::Cos(phi)*TMath::Cos(theta)*(Z-ZpmtTilt);
		xDet3+=XpmtTilt;
		yDet3+=YpmtTilt;
		zDet3+=ZpmtTilt;
        }
        if (X>0 && Y<0) {
                xDet3 = X*TMath::Cos(phi) + inPos->Z()*TMath::Sin(phi) - detZOrig*TMath::Sin(phi);
                yDet3 = -X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) + Z*TMath::Sin(theta)*TMath::Cos(phi);
                zDet3 = -X*TMath::Cos(theta)*TMath::Sin(phi) - Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
	}
        if (X<0 && Y<0) {
                xDet3 = X*TMath::Cos(phi) - Z*TMath::Sin(phi) + detZOrig*TMath::Sin(phi);
                yDet3 = X*TMath::Sin(theta)*TMath::Sin(phi) + Y*TMath::Cos(theta) + Z*TMath::Sin(theta)*TMath::Cos(phi);
                zDet3 = X*TMath::Cos(theta)*TMath::Sin(phi) - Y*TMath::Sin(theta) + Z*TMath::Cos(theta)*TMath::Cos(phi);
        }
        if (X<0 && Y>0) {
                xDet3 = TMath::Cos(-phi)*(X-XpmtTilt) - TMath::Sin(-phi)*TMath::Sin(theta)*(Y-YpmtTilt) + TMath::Sin(-phi)*TMath::Cos(theta)*(Z-ZpmtTilt);
                yDet3 = TMath::Cos(theta)*(Y-YpmtTilt) + TMath::Sin(theta)*(Z-ZpmtTilt);
                zDet3 = -TMath::Sin(-phi)*(X-XpmtTilt) - TMath::Cos(-phi)*TMath::Sin(theta)*(Y-YpmtTilt) + TMath::Cos(-phi)*TMath::Cos(theta)*(Z-ZpmtTilt);
		xDet3+=XpmtTilt;
		yDet3+=YpmtTilt;
		zDet3+=ZpmtTilt;

	}

	cout << setprecision(10);
	cout << "Reference position = {-6.639975, 128.346811, 207.020864}" << endl;
	cout << "Point position on the tilted PMT plane = {" << X << ", " << Y << ", " << Z << "}" << endl;
	cout << "Position after rotation of the PMT plane:" << endl;
	cout << "* using calculations from tiltPoint method = {" << xDet1 << ", " << yDet1 << ", " << zDet1 << "}" << endl;
	cout << "* using new calculations = {" << xDet2 << ", " << yDet2 << ", " << zDet2 << "}" << endl;
	cout << "* using inverse transformation = {" << xDet3 << ", " << yDet3 << ", " << zDet3 << "}" << endl;
}
