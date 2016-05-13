#ifndef CBMRICHMIRROR_H
#define CBMRICHMIRROR_H


#include "FairTask.h"
#include "TVector3.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

class TClonesArray;
class TH1D;
class TH2D;


class CbmRichMirror
{
public:
	/*
	 * Constructor.
	 */
	CbmRichMirror() : fMirrorId(""), fMomentum(), fFittedRing(), fExtrapHit(), fRotAngles()
	{
	}

	/*
	 * Destructor.
	 */
    virtual ~CbmRichMirror() {}

    // Modifiers:
	void setMirrorId(string s) { fMirrorId = s; }
	void setMomentum(TVector3 v) { fMomentum = v; }
	void setFittedRing(Double_t xX, Double_t yY) { fFittedRing.push_back(xX), fFittedRing.push_back(yY); }
	void setExtrapHit(Double_t xX, Double_t yY) { fExtrapHit.push_back(xX), fExtrapHit.push_back(yY); }
	void setRotAngles(Double_t xX, Double_t yY) { fRotAngles.push_back(xX), fRotAngles.push_back(yY); }

	// Accessors:
	string getMirrorId() { return fMirrorId; }
	TVector3 getMomentum() { return fMomentum; }
	vector<Double_t> getFittedRing() { return fFittedRing; }
	vector<Double_t> getExtrapHit() { return fExtrapHit; }
	vector<Double_t> getRotAngles() { return fRotAngles; }

	void writeInfos(string mirrorID, TString outputDir)
	{
		ofstream outputFile;
		TString s = outputDir + "Mirror_" + mirrorID + ".txt";
		outputFile.open(s);
		if (outputFile.is_open())
			{
				outputFile << setprecision(7) << fMirrorId << "\t";
				outputFile << setprecision(7) << fMomentum.Px() << "\t";
				outputFile << setprecision(7) << fMomentum.Py() << "\t";
				outputFile << setprecision(7) << fMomentum.Pz() << "\t";
				outputFile << setprecision(7) << fFittedRing[0] << "\t";
				outputFile << setprecision(7) << fFittedRing[1] << "\t";
				outputFile << setprecision(7) << fExtrapHit[0] << "\t";
				outputFile << setprecision(7) << fExtrapHit[1] << "\n";
				outputFile.close();
				cout << "Wrote correction parameters to: " << s << endl;
			}
		else {cout << "Error in CbmRichMirror::Finish ; unable to open parameter file!" << endl;}
	}


private:
	string fMirrorId;
	TVector3 fMomentum;
	vector<Double_t> fFittedRing;
	vector<Double_t> fExtrapHit;
	vector<Double_t> fRotAngles;

    CbmRichMirror(const CbmRichMirror&);
    CbmRichMirror operator=(const CbmRichMirror&);

    ClassDef(CbmRichMirror, 1);
};

#endif
