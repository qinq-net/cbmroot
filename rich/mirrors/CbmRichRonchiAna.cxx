
#include "CbmRichRonchiAna.h"

#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/tiff_dynamic_io.hpp>
//#include <boost/gil/extension/io/png_dynamic_io.hpp>

#include <iostream>
#include "TH2D.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"
#include "TEllipse.h"
#include <math.h>

using namespace boost::gil;
using namespace std;

// For Ubuntu one needs dev version of libtiff
// sudo apt-get install libtiff-dev

CbmRichRonchiAna::CbmRichRonchiAna()
{

}

CbmRichRonchiAna::~CbmRichRonchiAna()
{

}

void CbmRichRonchiAna::Run()
{
    // Initialization
    vector<vector<int> > dataV;
    vector<vector<int> > dataH;
        
    if ( fTiffFileNameV == "" || fTiffFileNameH == "" ) {
        Fatal("CbmRichRonchiAna::Run:", "No FileNameV or FileNameH!");
    }
    else { cout << "FileNameV: " << fTiffFileNameV << " and FileNameH: " << fTiffFileNameH << endl;
        dataV = ReadTiffFile(fTiffFileNameV);
        dataH = ReadTiffFile(fTiffFileNameH);
    }

    int width = dataV.size();
    int height = dataV[0].size();

    TH2D* hInitH = new TH2D("hInitH", "hInitH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanIntensityH = new TH2D("hMeanIntensityH", "hMeanIntensityH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hPeakH = new TH2D("hPeakH", "hPeakH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanYH = new TH2D("hMeanYH", "hMeanYH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);

    TH2D* hInitV = new TH2D("hInitV", "hInitV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanIntensityV = new TH2D("hMeanIntensityV", "hMeanIntensityV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hPeakV = new TH2D("hPeakV", "hPeakV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanYV = new TH2D("hMeanYV", "hMeanYV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    
    TH2D* hSuperpose = new TH2D("hSuperpose", "hSuperpose;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    
    TH2D* hIntersections = new TH2D("hIntersections", "hIntersections;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);


    // vertical image
    FillH2WithVector(hInitV, dataV);
    DoRotation(dataV);
    DoMeanIntensityY(dataV);
    FillH2WithVector(hMeanIntensityV, dataV);
    DoPeakFinderY(dataV);
    FillH2WithVector(hPeakV, dataV);
    DoMeanY(dataV);
    DoRotation(dataV);
    FillH2WithVector(hMeanYV, dataV);

    // horizontal image
    FillH2WithVector(hInitH, dataH);
    DoMeanIntensityY(dataH);
    FillH2WithVector(hMeanIntensityH, dataH);
    DoPeakFinderY(dataH);
    FillH2WithVector(hPeakH, dataH);
    DoMeanY(dataH);
    FillH2WithVector(hMeanYH, dataH);

    // finding intersections
    vector<pair<int,int> > intersectionXY = DoIntersection(dataH, dataV);
    vector<vector<int> > dataSup = DoSuperpose(dataH, dataV);
    FillH2WithVector(hSuperpose, dataSup);
    
    // numbering the intersections
    vector<vector<int>> intNumberXY = DoFindBasePoint(dataH, dataV, intersectionXY);
    int values = DoSearchNextLine(dataH, dataV, intNumberXY);
    //DoScanLine(dataH, dataV, intNumberXY, values);
 
    {
        TCanvas* c = new TCanvas("ronchi_2d_horizontal", "ronchi_2d_horizontal", 2000, 500);
        c->Divide(4,1);
        c->cd(1);
        DrawH2(hInitH);
        c->cd(2);
        DrawH2(hMeanIntensityH);
        c->cd(3);
        DrawH2(hPeakH);
        c->cd(4);
        DrawH2(hMeanYH);
    }

    {
        TCanvas* c = new TCanvas("ronchi_2d_vertical", "ronchi_2d_vertical", 2000, 500);
        c->Divide(4,1);
        c->cd(1);
        DrawH2(hInitV);
        c->cd(2);
        DrawH2(hMeanIntensityV);
        c->cd(3);
        DrawH2(hPeakV);
        c->cd(4);
        DrawH2(hMeanYV);
    }
    
    {
        TCanvas* c = new TCanvas("Superpose", "Superpose", 1000, 1000);
        DrawH2(hSuperpose);
    }
    
    {
        TH1D* h1 = hInitH->ProjectionY("_py1", 100, 100);
        TH1D* h2 = hInitH->ProjectionY("_py2", 200, 200);
        TH1D* h3 = hInitH->ProjectionY("_py3", 300, 300);
        TH1D* h4 = hInitH->ProjectionY("_py4", 500, 500);

        TH1D* hM1 = hMeanIntensityH->ProjectionY("_pyM1", 100, 100);
        TH1D* hM2 = hMeanIntensityH->ProjectionY("_pyM2", 200, 200);
        TH1D* hM3 = hMeanIntensityH->ProjectionY("_pyM3", 300, 300);
        TH1D* hM4 = hMeanIntensityH->ProjectionY("_pyM4", 400, 400);

        TH1D* hP1 = hPeakH->ProjectionY("_pyP1", 100, 100);
        TH1D* hP2 = hPeakH->ProjectionY("_pyP2", 200, 200);
        TH1D* hP3 = hPeakH->ProjectionY("_pyP3", 300, 300);
        TH1D* hP4 = hPeakH->ProjectionY("_pyP4", 400, 400);

        TCanvas* c2 = new TCanvas("ronchi_1d_slices_horizontal", "ronchi_1d_slices_horizontal", 1000, 1000);
        c2->Divide(2,2);
        c2->cd(1);
        DrawH1({h1,hM1,hP1}, {"Init", "Mean", "Peak"});
        c2->cd(2);
        DrawH1({h2,hM2,hP2}, {"Init", "Mean", "Peak"});
        c2->cd(3);
        DrawH1({h3,hM3,hP3}, {"Init", "Mean", "Peak"});
        c2->cd(4);
        DrawH1({h4,hM4,hP4}, {"Init", "Mean", "Peak"});
    }
    
    {
        TCanvas* c = new TCanvas("ronchi_2d_intersection", "ronchi_2d_intersection", 1000, 1000);
        DrawH2(hSuperpose);
        
        for (int i = 0; i < intersectionXY.size(); i++) {
            pair<int,int> xy = intersectionXY[i];
            TEllipse* center = new TEllipse(xy.first, xy.second, 3);
            center->Draw();
        }
    }
    
   /* {
        TCanvas* c = new TCanvas("ronchi_2d_Number_intersection", "ronchi_2d_Number_intersection", 1000, 1000);
        DrawH2(hSuperpose);
        
        for (int i = 0; i < intNumberXY.size()-1; i++) {
            pair<int,int> xy;
            xy.first = intNumberXY[i][0];
            xy.second = intNumberXY[i][1];
            TEllipse* center = new TEllipse(xy.first, xy.second, 3);
            center->Draw();
        }
    }*/
}

// finding base intersection
vector<vector<int>> CbmRichRonchiAna::DoFindBasePoint(const vector<vector<int>>& dataH, const vector<vector<int>>& dataV, const vector<pair<int,int>>& intersectionXY)
{
    vector<vector<int>> intNumberXY; 

   
    int nX = dataH[0].size();
    int nY = dataH.size();
    int nI = intersectionXY.size();
    int l = 0;
    int lineDistance = 20; // approx. distance of lines
    int currentY = 0;
    int startx = 0;
    
    bool line = false;
    bool intersection = false;
    
    
    // if first continuous line is considered as base line   
    for (int y0 = lineDistance; y0 < nY; y0++) {    // finding base line
        if (dataH[511][y0] > 0) {
            currentY = y0;
            break;
        }
    }
            
    for (int x = 510; x > lineDistance; x--) {  // finding beginning of base line
        line = false;
        for (int ySearch = currentY-1; ySearch <= currentY+1; ySearch++) {
            if (dataH[x][ySearch] > 0) {
                currentY = ySearch;
                line = true;
                break;
            }
        }
        if (line == false) {
            startx = x+1;
            break;
        }
    }
    
    intNumberXY.resize(nI);
    for (int x = startx; x < nX; x++) {  // finding first intersection on base line
        intNumberXY[l].resize(4);
        for (int ySearch = currentY-1; ySearch <= currentY+1; ySearch++) {
            if (dataH[x][ySearch] > 0 && dataV[x][ySearch] > 0) {
                intersection = true;
                intNumberXY[0][0] = x;
                intNumberXY[0][1] = ySearch;
                intNumberXY[0][2] = 0;
                intNumberXY[0][3] = 0;
            }
        }
        if (intersection == true) break;
    }
    
    /*
    
    // if first line bottom left is considered as base line
    bool foundLine = false;
    for (int x = 1.5*lineDistance; x < nX; x++) {       
        for (int y = lineDistance; y < 3*lineDistance; y++) {
            if (dataH[x][y] > 0) {
                basePointXY.first = x;
                basePointXY.second = y;
                foundLine = true;
                break;
            }
        }
        if (foundLine == true) break;
    } */
    return intNumberXY;  
}


// searching for next line
int CbmRichRonchiAna::DoSearchNextLine(const vector<vector<int>>& dataH, const vector<vector<int>>& dataV, vector<vector<int>>& intNumberXY)
{
    int nX = dataH[0].size();
    int nY = dataH.size();
    int lineDistance = 20; // approx. distance of lines
    
    int i = 0;  // line index of intersection
    int j = 0;  // column index of intersection
    int l = 0;  // l-th argument of vector intNumberXY   
    int xInit = intNumberXY[0][0];     
    int yInit = intNumberXY[0][1];        
                
    int values[] = {i,j,l,xInit,yInit,0};   // last value indicates if last line had been reached, to cancel for further searching
    
   cout << "Intersection 0: ["  << intNumberXY[0][0] << "," << intNumberXY[0][1] << "," << intNumberXY[0][2] << "," << intNumberXY[0][3] << "]" << endl; 
        
    DoScanLine(dataH, dataV, intNumberXY, values);  // scanning and numbering intersections of first line
    
    int currentX = xInit;
    int currentY = yInit;
    int counterX = 0;
    int counterY = 0;
    
    for (int y = yInit+1; y <= nY-2*lineDistance; y++) {  // search next intersection above
        counterY++;
        //if (values[5] == 1) break;
        if (y > 0.9*nY) break;          // can be deleted if bool 'lastLine' in 'DoScanLine' is active 
        for (int x = currentX-1; x <= currentX+1;x++) {
            if (dataV[x][y] > 0 && dataH[x][y] > 0) {
                values[1]++;    // j
                values[2]++;    // l
                values[3] = x;  // xInit
                values[4] = y;  // yInit
                j = values[1];
                l = values[2];
               cout << "l = " << l << "    j = " << j << endl;
                //xInit = x;    not needed
                yInit = y; 
                intNumberXY[l].push_back(x);
                intNumberXY[l].push_back(y);
                intNumberXY[l].push_back(i);
                intNumberXY[l].push_back(j);
                counterY = 0;    
               cout << "IntersectionBase  " << l << ": [" << intNumberXY[l][0] << "," << intNumberXY[l][1] << "," << intNumberXY[l][2] << "," << intNumberXY[l][3] << "]" << endl;
                DoScanLine(dataH, dataV, intNumberXY, values);
                break;
            }
            else if (dataV[x][y] > 0 && dataH[x][y] == 0) {
                currentX = x;
                break;
            }
            if (counterY > 2*lineDistance) {  // if no intersection in range of 2*lineDistance, go to next intersection on the right and search from there
               cout << "Skipping" << endl;
                xInit = values[3];
                yInit = values[4];
               cout << "xInit before = " << xInit << "    yInit before = " << yInit << endl;
                currentY = yInit;
                bool foundNext = false;
                for (int x2 = xInit+1; x2 < nX-2*lineDistance; x2++) {
                    counterX++;
                    for (int y2 = currentY-1; y2 <= currentY+1; y2++) {
                        if (dataV[x2][y2] > 0 && dataH[x2][y2] > 0) {
                            currentX = x2;
                            cout << "y = " << y << endl;
                            xInit = x2;
                            yInit = y2;
                            y = yInit;  // y = yInit+1? why does it work yet?
                            i++;
                            values[0] = i;
                            foundNext = true;
                            counterX = 0;
                            counterY = 0;
                           cout << "xInit after = " << xInit << "    yInit after = " << yInit << endl;
                            break;                           
                        }
                        else if (dataH[x2][y2] > 0) {
                            currentY = y2;
                            break;
                        }
                        if (counterX > 2*lineDistance) {
                           cout << "Gap or end of line!" << endl;
                            counterX = 0;
                            break;
                        }
                    }
                    if (foundNext == true) break;
                }
                if (foundNext == true) break;
            }            
        }        
    }
}  


// scanning the line 'j' and numbering intersections
void CbmRichRonchiAna::DoScanLine(const vector<vector<int>>& dataH, const vector<vector<int>>& dataV, vector<vector<int>>& intNumberXY, int values[])     // marks intersections on current line and puts data into vector intNumberXY
{
    int nX = intNumberXY.size();
    int lineDistance = 20;
    int initialI = values[0];
    int currentY = values[4];
        
    int i = values[0];
    int j = values[1];
    int l = values[2];
    int xInit = values[3];
    int yInit = values[4];
           
    bool line = false;
    bool lineAbove = false; // both this and next bool to detect last line and end the program (has yet to be installed)
    bool lastLine = false;
    
    for (int x = xInit-1; x > 30; x--) {    // searching for intersections on the left side of initial intersection
        line = false;
        for (int ySearch = currentY-1; ySearch <= currentY+1; ySearch++) {
            if (dataH[x][ySearch] > 0 && dataV[x][ySearch] > 0) {
                currentY = ySearch;
                i--;
                l++;
                intNumberXY[l].push_back(x);
                intNumberXY[l].push_back(ySearch);
                intNumberXY[l].push_back(i);
                intNumberXY[l].push_back(j);
                line = true;
                cout << "IntersectionLeft " << l << ": [" << intNumberXY[l][0] << "," << intNumberXY[l][1] << "," << intNumberXY[l][2] << "," << intNumberXY[l][3] << "]" << endl;
                break;
            }
            else if (dataH[x][ySearch] > 0) {
                currentY = ySearch;
                line = true;
                break;
            }
        }
        if (line == false) break;   // end of line or a gap had been reached
    }
    
    currentY = yInit;
    i = initialI;
    
    for (int x = xInit+1; x < nX-30; x++) {      // searching for intersections on the right side of initial intersection
        line = false;        
        if (x == 511) {     // looking, if this is the last continuous line
            lineAbove = false;
            for (int y2 = currentY; y2 <= currentY+2*lineDistance; y2++) {
                if (dataH[x][y2] > 0) {
                    lineAbove = true;
                }
            }
            if (lineAbove == false) values[5] = 1;
           cout << "                                        values[5] = " << values[5] << endl;
        }
        
        for (int ySearch = currentY-1; ySearch <= currentY+1; ySearch++) {
            
            if (dataH[x][ySearch] > 0 && dataV[x][ySearch] > 0) {
                currentY = ySearch;
                i++;
                l++;                
                intNumberXY[l].push_back(x);
                intNumberXY[l].push_back(ySearch);
                intNumberXY[l].push_back(i);
                intNumberXY[l].push_back(j);
                line = true;
                cout << "IntersectionRight " << l << ": [" << intNumberXY[l][0] << "," << intNumberXY[l][1] << "," << intNumberXY[l][2] << "," << intNumberXY[l][3] << "]" << endl;
                break;
            }
            else if (dataH[x][ySearch] > 0) {
                currentY = ySearch;
                line = true;
                break;
            }
        }
        if (line == false) break;
    }
    
    values[2] = l;
    
    cout << "values (i="<< i << ", j=" << j << ", l=" << l << ", xInit=" << xInit << ", yInit=" << yInit << ")"  << endl;
}


// calculating mean position in Y
void CbmRichRonchiAna::DoMeanY(vector<vector<int> >& data)
{
    int meanHalfLength = 5;
    int meanHalfHeight = 3;
    int nX = data.size();
    int nY = data[0].size();
    
    for (int x = meanHalfLength; x < nX-meanHalfLength; x++) {
        for (int y = meanHalfHeight; y < nY-meanHalfHeight; y++) {
            if (data[x][y] == 0) continue;
            int curData = data[x][y];
            int sumY = 0;
            int divider = 0;
            for (int x2 = -meanHalfLength; x2 <= meanHalfLength; x2++) {
                for (int y2 = -meanHalfHeight; y2 <= meanHalfHeight; y2++) {
                    if (data[x+x2][y+y2] > 0) {
                        sumY += y+y2;
                        divider++;
                    }
                }
            }
            data[x][y] = 0;
            y=(int) sumY/divider;   
            data[x][y] = curData;
            curData = 0;
            sumY = 0;
        }
    }
}

void CbmRichRonchiAna::DoRotation(vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();
    for (int x = 0; x < nX; x++) {
        for (int y = x+1; y < nY; y++) {
            swap(data[x][y], data[y][x]);
        }
    }
}

void CbmRichRonchiAna::DoPeakFinderY(vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();
    int halfWindow = 10;

    for (int x = 0; x < nX; x++) {
        for (int y = 0 + halfWindow; y < nY - halfWindow; y++){
            bool isPeak = (data[x][y] >= data[x][y - 1]) && (data[x][y] >= data[x][y + 1]);
            if (!isPeak) {
                data[x][y] = 0;
                continue;
            }
            bool isBiggest = true;
            for (int iW = -halfWindow; iW <= halfWindow; iW++) {
                if (iW == 0) continue;
                if (data[x][y + iW] >= data[x][y]) {
                    isBiggest = false;
                    break;
                }
            }
            data[x][y] = (isBiggest && isPeak)? data[x][y] : 0;
        }
    }
}

void CbmRichRonchiAna::DoMeanIntensityY(vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();

    int halfAvWindow = 6;
    int threshold = 23;
    vector<int> weights = {100, 80, 50, 20, 10, 5};

    for (int x = 0; x < nX; x++) {
        for (int y = 0; y < nY; y++){
            int total = 0;
            int weightSum = 0;
            for (int iW = -halfAvWindow; iW <= halfAvWindow; iW++) {
                int iWAbs = std::abs(iW);
                int weight = (iWAbs < weights.size() )? weights[iWAbs] : weights[weights.size() - 1];
                weightSum += weight;
                int ind = y + iW;
                if (ind < 0) ind = 0;
                if (ind >= nY) ind = nY - 1;
                total += data[x][ind] * weight;
            }
            data[x][y] = total / weightSum;
            if (data[x][y] <= threshold) data[x][y] = 0;
        }
    }
}

void CbmRichRonchiAna::FillH2WithVector(TH2* hist, const vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();
    
    for (int x = 0; x < nX; x++) {
        for (int y = 0; y < nY; y++){
            if (data[x][y] != 0) {
                hist->SetBinContent(x, y, data[x][y]);
            }
        }
    }
}

vector<pair<int,int> > CbmRichRonchiAna::DoIntersection(const vector<vector<int> >& dataH, const vector<vector<int> >& dataV)
{
    int nX = dataV.size();
    int nY = dataV[0].size();
    
    vector<pair<int,int> > intersectionXY;
    
    for (int y = 0; y < nY; y++) {
        for (int x = 0; x < nX; x++) {
            if (dataH[x][y] > 0 && dataV[x][y] > 0) {
                intersectionXY.push_back(make_pair(x,y));
            }
        }
    }
    cout << "Number of intersections: " << intersectionXY.size() << endl;
    //for (int i = 0; i < intersectionXY.size(); i++) {
    //    cout << "[" << intersectionXY[i].first << "," << intersectionXY[i].second << "]" << endl;
    //}    
    return intersectionXY;
}


vector<vector<int> > CbmRichRonchiAna::DoSuperpose(const vector<vector<int> >& dataH, const vector<vector<int> >& dataV)
{
    int nX = dataV.size();
    int nY = dataV[0].size();
    
    vector<vector<int> > dataSup;
    dataSup.resize(nX);
    for (int x = 0; x < nX; x++) {
        dataSup[x].resize(nY);
        for (int y = 0; y < nY; y++) {
            dataSup[x][y] = dataH[x][y] + dataV[x][y];
        }
    }
    return dataSup;
}


vector<vector<int> > CbmRichRonchiAna::ReadTiffFile(const string& fileName)
{
    vector<vector<int> > data;
    rgba8_image_t img;
    tiff_read_and_convert_image(fileName,img);

    int height = img.height();
    int width = img.width();

    data.resize(width);

    auto view = const_view(img);
    for (int x = 0; x < width; ++x) 
    {
        auto it = view.col_begin(x);
        data[x].resize(height);
        for (int y = 0; y < height; ++y) 
        {
            int r = boost::gil::at_c<0>(it[y]);
            //int g = boost::gil::at_c<1>(it[y]);
            //int b = boost::gil::at_c<2>(it[y]);
            //int a = boost::gil::at_c<3>(it[y]);
            data[x][y] = r;
        }
    }
    return data;
}


ClassImp(CbmRichRonchiAna)
