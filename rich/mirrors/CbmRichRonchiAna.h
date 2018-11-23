
#ifndef RICH_MIRRORS_CBMRICHRONCHIANA_H_
#define RICH_MIRRORS_CBMRICHRONCHIANA_H_

#include "TObject.h"
#include <string>
#include <vector>

#include <cmath>

using namespace std;

class TH2;

class CbmRichRonchiAna : public TObject 
{
public:
    CbmRichRonchiAna();
    
    virtual ~CbmRichRonchiAna();

    void Run();

    void SetTiffFileNameV(const string& fileName) {fTiffFileNameV = fileName;}
    void SetTiffFileNameH(const string& fileName) {fTiffFileNameH = fileName;}

private:

    string fTiffFileNameV;
    string fTiffFileNameH;
    
    vector<vector<int> > ReadTiffFile(const string& fileName);

    void DoMeanIntensityY(vector<vector<int> >& data);

    void DoPeakFinderY(vector<vector<int> >& data);

    void DoRotation(vector<vector<int> >& data);

    void DoMeanY(vector<vector<int> >& data);

    void FillH2WithVector(TH2* hist, const vector<vector<int> >& data);
    
    vector<vector<int> > DoSuperpose(const vector<vector<int> >& dataH, const vector<vector<int> >& dataV);
    
    vector<pair<int,int> > DoIntersection(const vector<vector<int> >& dataH, const vector<vector<int> >& dataV);
    
    void DoIntersectionFinder(vector<vector<int> >& dataSup);
    
    vector<pair<int,int> > DoNumInt(vector<pair<int,int> >& intersectionXY, vector<vector<int> >& dataSup);
    
   
    /**
    * \brief Copy constructor.
    */
    CbmRichRonchiAna(const CbmRichRonchiAna&);

    /**
    * \brief Assignment operator.
    */
    CbmRichRonchiAna& operator=(const CbmRichRonchiAna&);

    ClassDef(CbmRichRonchiAna,1)
};

#endif /* RICH_MIRRORS_CBMRICHRONCHIANA_H_ */

