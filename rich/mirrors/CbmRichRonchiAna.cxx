
#include "CbmRichRonchiAna.h"

#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/tiff_dynamic_io.hpp>
//#include <boost/gil/extension/io/png_dynamic_io.hpp>

#include <iostream>
#include "TH2D.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"

using namespace boost::gil;
using namespace std;

// For Ubuntu one needs dev version of libtiff
//sudo apt-get install libtiff-dev

CbmRichRonchiAna::CbmRichRonchiAna()
{

}

CbmRichRonchiAna::~CbmRichRonchiAna()
{

}

void CbmRichRonchiAna::Run()
{
    vector<vector<int> > data = ReadTiffFile(fTiffFileNameV);
    //vector<vector<int> > dataH = ReadTiffFile(fTiffFileNameH);

    int width = data.size();
    int height = data[0].size();

    TH2D* hInit = new TH2D("hInit", "hInit;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hPeak = new TH2D("hPeak", "hPeak;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++){
            hInit->SetBinContent(x, y, data[x][y]);
        }
    }

    // averaging
    // probably one can implement weights
    int halfAvWindow = 3;
    int threshold = 20;
    for (int x = 0; x < width; x++) {
        for (int y = 0 + halfAvWindow; y < height - halfAvWindow; y++){
            int total = 0;
            for (int iW = -halfAvWindow; iW <= halfAvWindow; iW++) {
                total += data[x][y + iW];
            }
            data[x][y] = total / (2 * halfAvWindow + 1);
            if (data[x][y] <= threshold) data[x][y] = 0;
        }
    }

    //peak finder
    int halfWindow = 6;
    for (int x = 0; x < width; x++) {
        for (int y = 0 + halfWindow; y < height - halfWindow; y++){
            int curData = data[x][y];
            bool isPeak = (curData > data[x][y-1]) && (curData > data[x][y+1]);
            if (!isPeak) continue;

            bool isBiggest = true;
            for (int iW = -halfWindow; iW <= halfWindow; iW++) {
                if (iW == 0) continue;
                if (curData <= data[x][y + iW]) {
                    isBiggest = false;
                    break;
                }
            }
            if (isBiggest) {
                hPeak->SetBinContent(x, y, curData);
            }
        }
    }

    // Drawing
    {
        TCanvas* c = new TCanvas("c1", "c1", 1000, 1000);
        DrawH2(hInit);
    }

    {
        TH1D* h1 = hInit->ProjectionY("_py1", 100, 100);
        TH1D* h2 = hInit->ProjectionY("_py2", 200, 200);
        TH1D* h3 = hInit->ProjectionY("_py3", 300, 300);
        TH1D* h4 = hInit->ProjectionY("_py4", 400, 400);

        TH1D* hP1 = hPeak->ProjectionY("_pyP1", 100, 100);
        TH1D* hP2 = hPeak->ProjectionY("_pyP2", 200, 200);
        TH1D* hP3 = hPeak->ProjectionY("_pyP3", 300, 300);
        TH1D* hP4 = hPeak->ProjectionY("_pyP4", 400, 400);


        TCanvas* c2 = new TCanvas("c2", "c2", 1000, 1000);
        c2->Divide(2,2);
        c2->cd(1);
        DrawH1({h1,hP1}, {"Init", "Peak"});
        c2->cd(2);
        DrawH1({h2,hP2}, {"Init", "Peak"});
        c2->cd(3);
        DrawH1({h3,hP3}, {"Init", "Peak"});
        c2->cd(4);
        DrawH1({h4,hP4}, {"Init", "Peak"});
    }

    {
        TCanvas* c = new TCanvas("c3", "c3", 1000, 1000);
        DrawH2(hPeak);
    }



//  // EXTRACTING THE MAXIMA (H)
//
//
//    int hrangey = 8;       // range of pixels (in both directions) in y that will be scanned to locate a maximum; has to be adapted, depending on distance of imaged lines
//    int brightness[heightnew];  // array that will be filled with the brightness values of column x and deleted after scanning each column
//    int imageh[widthnew][heightnew]; // array that represents the tailored image
//
//    for (unsigned int y = 0; y < heightnew; y++)    // initializing arrays
//    {
//        brightness[y] = 0;
//        for (unsigned int x = 0; x < widthnew; x++)
//        {
//         imageh[x][y] = 0;
//         }
//     }
//
//    for (int x = startx; x < (widthnew+startx); x++)
//    {
//        //int n = heightnew-hrangey-1;     // variable to flip the new image around the x axis. otherwise it is upside down to the origin image 'data'
//
//        for (int y = starty; y < (heightnew+starty); y++)     // giving every pixel in column x the brightness value of the corresponding column of 'data'
//        {
//           brightness[y-starty] = data[x][y];
//         }
//
//        for (int y = hrangey; y < (heightnew-hrangey); y++)
//        {
//            int max = 0;    // counter to detect maxima
//            for (int y1 = y-hrangey; y1 <= y+hrangey; y1++) // FOR-loop to compare pixel y with the neighboured ones
//            {
//                if (brightness[y] > brightness[y1]) {max++;};   // if it is greater than all neighbours, increase 'max' by ONE
//             }
//
//            if (max == (2*hrangey))    // if 'max' is twice the size of 'hrangey', it is a local maximum
//            {
//               imageh[x-startx][y] = 1; // in this case, fill this pixel with a ONE
//               // hist->SetBinContent(x, y, image[x-startx][y]);
//             }
//
//             //n -= 2;
//         }
//
//         for (int y = 0; y < heightnew; y++)    // deleting all entries of the array
//         {
//            brightness[y] = 0;
//          }
//     }
//
//
//
//  // DRAWING MEAN OF LINE DISTRIBUTION (H)
//
//
//  int windowmean = 8;      // half of approximate distribution of line pixels; defines the height of scanning window; has to be adapted according to image
//  int meanlength = 10;     // number of pixels to each side to calculate mean; defines width of scanning window
//  int locy = 0;            // location of line pixel in y
//  int counter = 0;         // counter for detected line pixels
//  float mean = 0;          // mean value in y that will be calculated
//
//  for (int x = meanlength; x < widthnew-meanlength; x++)    // still to improve: start at ZERO (assure that not averaging with not existing pixel at x,y < 0)
//  {
//      for (int y = windowmean; y < heightnew-windowmean; y++)   // increase y until a line pixel is found
//      {
//          if (imageh[x][y] == 1)    // if a line pixel is found, calculate the mean in y from the 'meanlength' pixels on each side by...
//          {
//               for (int x1 = (x-meanlength); x1 <= (x+meanlength); x1++)
//               {
//                   for (int y1 = (y-windowmean); y1 <= (y+windowmean); y1++)
//                   {
//                      if (imageh[x1][y1] == 1)
//                      {
//                          locy += y1;   //... summing up all values in y, where there pixels are found and...
//                          counter++;
//                       }
//                    }
//                }
//
//               mean = locy/counter;     //... divide them by the number of found line pixels
//               imageh[x][y] = 0;        // delete this pixel...
//               imageh[x][(int) mean] = 1;   //... and place it at the calculated mean value (which might be the old value)
//               locy = 0;        // resetting variables
//               counter = 0;
//               hist->SetBinContent(x, (int) mean, imageh[x][(int) mean]);
//               y += 2*windowmean;
//           }
//       }
//   }
//
//
//
//  // INTERPOLATING THE GAPS (H)
//
//
//  int winheight = 4;    // define the (half) height and (full) width of the scanning window
//  int winwidth = 80;    // 'winwidth' defines also maximum size of a gap, that will be interpolated
//
//  int posxleft = 0;     // positions of the last and next filled pixel on both sides of a gap
//  int posxright = 0;
//  int posyleft = 0;
//  int posyright = 0;
//
//  float slope = 0;      // slope of interpolated pixels with unit "pixel per column"
//
//  int gap = 0;
//  int nogap = 0;
//
//  for (int x = 0; x < widthnew-winwidth; x++)
//  {
//      for (int y = winheight; y < heightnew-winheight; y++)
//      {
//          if (imageh[x][y] == 1)     // if a pixel is detected, look for a pixel of this line in the next column
//          {
//              posxleft = x;          // memorize the position of the last detected existing pixel
//              posyleft = y;
//
//              for (int y1 = y-winheight; y1 <= y+winheight; y1++)   // scanning the next column to see if there is a gap or not
//              {
//                  if (imageh[x+1][y1] == 1) {nogap++;}
//               }
//
//              if (nogap == 0)     // if no pixel of the line in the next column is found (i.e. a gap) ...
//              {
//                  gap++;          //... increase 'gap' by ONE and ...
//
//                  for (int x1 = x+2; x1 <= x+winwidth; x1++)        //... find out the size of the gap by searching for the next pixel ...
//                  {                                                 //... in the next 'winwidth' columns
//                      for (int y2 = y-winheight; y2 <= y+winheight; y2++)
//                      {
//                          if (imageh[x1][y2] == 1)  // if the next pixel is found ...
//                          {
//                              nogap++;      //... 'nogap' will be increased (for the next if condition) and ...
//                              posxright = x1;   //... the position memorized
//                              posyright = y2;
//                           }
//                       }
//
//                      if (nogap == 0) {gap++;}      // if no pixel is detected in this column, 'gap' increases by ONE
//                      else if (nogap == 1) {break;} // if a pixel is detected, leave this loop
//                   }
//
//                   if (gap >= winwidth)     // if the gap is at least as large as window, it is too big to be interpolated (inform via stream on screen)
//                   {
//                       cout << "----------- GAP AT POSITION [" << x << "," << y << "] IS TOO BIG -------------" << endl;
//                       gap = 0;
//                       break;
//                    }
//
//                   slope = (posyright-posyleft)/(posxright-posxleft);   // calculate the slope of the gap
//                   int mult = 1;    // defines the ...th pixel of the gap that will be filled
//
//                   for (int x2 = x+1; x2 <= x+gap; x2++)      // filling up gaps by interpolating
//                   {
//                       imageh[x2][y+mult*((int) slope)]=2;
//                       mult++;
//                    }
//               }
//
//              else if (nogap == 1) // i.e. no gap, than reset variables
//              {
//                  nogap = 0;
//                  posxleft = 0;
//                  posyleft = 0;
//                  continue;
//               }
//
//              else {cout << "----------------------------------------------- Gap = " << gap << " ---- -> TOO MANY PIXELS PER LINE!" << endl;}
//
//              posxleft = 0;     // reset variables
//              posxright = 0;
//              posyleft = 0;
//              posyright = 0;
//              gap = 0;
//              nogap = 0;
//              slope = 0;
//           }
//
//           hist->SetBinContent(x, y, imageh[x][y]);
//       }
//   }
  

  
       
    

//------------------------------------------------------------------------------------------------------------------------   
// VERTICAL RULING

  // EXTRACTING THE MAXIMA (V)
  
/*  
    int vrangex = 8;      // range of pixels (in both directions) in x that will be scanned to locate a maximum
    
    int vbrightness[widthnew];  // ...
    int vimage[widthnew][heightnew]; 
    
    
    for (unsigned int x = 0; x < widthnew; x++)    // initializing arrays
    {
        vbrightness[x] = 0;
        for (unsigned int y = 0; y < widthnew; y++)
        {
         vimage[x][y] = 0;
         }
     }
              
    for (int y = starty; y < (heightnew+starty); y++)    
    {
        //int n = heightnew-2;     // variable to flip the new image around the x axis. otherwise it is upside down to the origin; is it -2 or -1 ???
        for (int x = startx; x < (widthnew+startx); x++)     // giving every pixel in col. x a brightness value 
        {
            vbrightness[x-startx] += data[x][y];  
         }

        for (int x = vrangex; x < (widthnew-vrangex); x++)
        {
            int max = 0;    // counter to detect maxima
            for (int x1 = x-vrangex; x1 <= x+vrangex; x1++)
            {
                if (vbrightness[x] > vbrightness[x1]) {max++;} 
             }
            
            if (max >= 2*vrangex)
            {
                vimage[x][y-starty] = 1;
                //hist->SetBinContent(x, y-starty, vimage[x][y-starty]);  
             }
                                      
             //n -= 2;            
         }         
         
        for (int x = 0; x < widthnew; x++)    // deleting all entries of the arrays 
        {
           vbrightness[x] = 0;
         }   
                     
     }
     
     
     
  // DRAWING MEAN OF LINE DISTRIBUTION (V)
  
  
  int window = 5;          // half of approximate distribution of line pixels; defines the width of scanning window; has to be adapted according to image
  int meanlengthV = 10;     // number of pixels to each side to calculate mean; defines height of scanning window
  int locx = 0;            // location of line pixel in x
  int counterV = 0;         // counter for detected line pixels
  float meanV = 0;          // mean value in x that will be calculated
  
  for (int y = meanlengthV; y < widthnew-meanlengthV; y++)    // still to improve: start at ZERO (assure that not averaging with not existing pixel at x,y < 0) 
  {
      for (int x = window; x < heightnew-window; x++)
      {    
          if (vimage[x][y] == 1)
          {
               for (int y1 = (y-meanlengthV); y1 <= (y+meanlengthV); y1++)
               {
                   for (int x1 = (x-window); x1 <= (x+window); x1++)
                   {
                      if (vimage[x1][y1] == 1)
                      {
                          locx += x1;
                          counterV++; 
                       }
                    }
                }
           
               meanV = locx/counterV;
               vimage[x][y] = 0;
               vimage[(int) meanV][y] = 1;
               locx = 0;        // resetting variables
               counterV = 0;
               hist->SetBinContent((int) meanV, y, vimage[(int) meanV][y]);
               x += 2*window; 
           } 
       }
   }
   
   
   
  // INTERPOLATING THE GAPS (V)
  
  
  int vwinwidth = 5;    // both variables define the (full) height and (half) width of the scanning window
  int vwinheight = 50;  // 'vwinheight' defines also maximum size of a gap that will be interpolated
  
  
  int posxbot = 0;      // positions of the last and next filled pixel on both sides of a gap
  int posxup = 0;
  int posybot = 0;         
  int posyup = 0;
  
  float slopeV = 0;      // slope of interpolated pixels with unit "pixel per column"
      
  int gapV = 0;
  int nogapV = 0;
    
  for (int y = 0; y < heightnew-vwinheight; y++)
  {
      
      for (int x = vwinwidth; x < widthnew-vwinwidth; x++)
      {
          if (vimage[x][y] == 1)     // if a pixel is detected, look for a pixel of this line in the next column
          {                          
              posxbot = x;          // memorize the position of the last detected existing pixel
              posybot = y;
              
              for (int x1 = x-vwinwidth; x1 <= x+vwinwidth; x1++)   // scanning the next column to see if there is a gap or not
              {
                  if (vimage[x1][y+1] == 1) {nogapV++;}
               }
               
              if (nogapV == 0)     // if no pixel of the line in the next column (i.e. a gap)...
              {
                  gapV++;
                                    
                  for (int y1 = y+2; y1 <= y+vwinheight; y1++)        // ...find out the size of the gap by searching for the next pixel
                  {
                      for (int x2 = x-vwinwidth; x2 <= x+vwinwidth; x2++)
                      {
                          if (vimage[x2][y1] == 1) 
                          {
                              nogapV++;
                              posxup = x2;
                              posyup = y1;
                           }
                       }  
                                      
                      if (nogapV == 0) {gapV++;}      // if no pixel is detected in this column, 'gap' increases by ONE
                      
                      else if (nogapV == 1) {break;} // if a pixel is detected, calculate the slope between last and first new pixel
                   }
                   
                   if (gapV >= vwinheight)     // if the gap is at least as large as window, it is too big to be interpolated (inform user via stream on screen)
                   {
                       cout << "----------- GAP AT POSITION [" << x << "," << y << "] IS TOO BIG -------------" << endl;
                       gapV = 0;
                       break;  
                    }
                   
                   slopeV = (posxup-posxbot)/(posyup-posybot);
                   int mult = 1;    // defines the ...th pixel of the gap that will be filled 
                   
                   for (int y2 = y+1; y2 <= y+gapV; y2++)      // filling up gaps by interpolating 
                   {
                       vimage[x+mult*((int) slopeV)][y2]=5;
                       mult++;
                    }
               }    
               
              else if (nogapV == 1) 
              {
                  nogapV = 0;
                  posxbot = 0;
                  posybot = 0;
                  continue;
               }
              
              else {cout << "----------------------------------------------- Gap = " << gapV << " ---- -> TOO MANY PIXELS PER LINE!" << endl;}          
                        
              posxbot = 0;     // reset variables
              posxup = 0;
              posybot = 0;         
              posyup = 0; 
              gapV = 0;
              nogapV = 0;
              slopeV = 0;           
           }
           
           hist->SetBinContent(x, y, vimage[x][y]);
       } 
   } */
     
/*
//---------------------------------------------------------------------------------------------------------------------   
  // SUPERPOSING HORIZONTAL AND VERTICAL IMAGES
  
  
  int imagesup[widthnew][heightnew];
  
  for (int x = 0; x < widthnew; x++)        // intializing array
  {
      for (int y = 0; y < heightnew; y++)
      {
          imagesup[x][y] = 0;
       }
   }
   
  
  for (int x = 0; x < widthnew; x++)    // adding the values of horizontal and vertical image entries at [x][y] to the new array imagesup
  {
      for (int y = 0; y < heightnew; y++)
      {
          imagesup[x][y] = imageh[x][y] + imagev[x][y];
       }
   }
  
 
  // EXTRACTING INTERSECTIONS
  
  int range = 10;           // defines size of range in y to follow the horizontal lines (dependent on changes in y of a line from one column to the next)
  int intsecwindow = 1;     // defines size of scanning window to detect intersection
  int sum = 0;              // sums up the pixels in intersection window
  int linepos= 0;           // to note position in y of the left side of current line
  int x1 = 0;               // will define intersection points in FOR-loop below; are defined here already to use them outside the FOR-loop
  int y1 = 0;
  int start = 5;            // first column that contains all lines (ideally ZERO)
  
  for (int y = intsecwindow; y < heightnew-intsecwindow; y++)   // along horizontal lines it will be searched for intersection...
  {                                                             // ...points, perceivable through higher value of summed up pixels
      if (imagesup[start][y] == 0) {continue;}
          
      else  // line is found; now sum up the pixel values in intersection window 'intsecwindow' ahead of this first detected pixel
      {
          linepos = y;  // for, after reaching the end of this line, go to next line in y direction
          cout << "Line starts at y = " << y << endl;
           
          for (x1 = x; x1 < widthnew-2*intsecwindow; x1++)    // FOR-loop to go along the horizontal line
          {
              for (y1 = y-range; y1 <= y+range; y1++)   // etwas doppelt gemoppelt (da diese Spalte nochmals gerastert wird) aber einfacher
              {
                  if (imagesup[x1][y1] > 0)
                  {    
                      for (int x2 = x; x2 <= x + (2*intsecwindow); x2++)     // scanning area ahead x and around y 
                      {   
                          for (int y2 = y-intsecwindow; y2 <= y+intsecwindow; y2++)
                          {    
                              if (imagesup[x2][y2] > 0)  // sum up number of pixels in this window
                              {
                                  cout << "Pixel Value at Position " << x << "," << y << " = " << image[x][y] << endl;
                                  sum+=imagesup[x2][y2];
                               }
                           }
                       }
                          
                      if (sum > (2*intsecwindow+1) )     // indicates an intersection point
                      {
                          cout << "sum = " << sum << endl;
                          imagesup[x1+2*intsecwindow][y1] = 10;
                          sum = 0;
                          x1 += 2*intsecwindow+1;
                       }
                       
                       else 
                       {
                           sum = 0;
                           break;
                        }
                   }
               }
           }
       }
   }  */
    
    //return; 
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
