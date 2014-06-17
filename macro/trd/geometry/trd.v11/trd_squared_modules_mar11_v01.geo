//*********************************************************
// TRD Mar11 geometry by DE
//*********************************************************
// stations located at 4500 / 6750 / 9000 mm in z
// with frontside of radiators of the 1st layer  
// v01 - 12 mm gas thickness		            
// v02 -  6 mm gas thickness (standard)          
// v03 - tilted, 6 mm gas thickness              
// v04 - tilted, aligned, 6 mm gas thickness     
// v05 - tilted layer pairs, 6 mm gas thickness  
//*********************************************************
//*********************************************************
// This is the geometry file of the following configuration
// Number of stations: 3
// Number of layers per station: 4
//*********************************************************
// Inner Radius of station 1: 240.000000
// Outer Radius of station 1: 4600.000000
// z-position at the middle of the station 1: 4800.000000
//*********************************************************
// Inner Radius of station 2: 240.000000
// Outer Radius of station 2: 6600.000000
// z-position at the middle of the station 2: 7050.000000
//*********************************************************
// Inner Radius of station 3: 490.000000
// Outer Radius of station 3: 8600.000000
// z-position at the middle of the station 3: 9300.000000
//*********************************************************
//*********************************
trd1
cave
PGON
air
4
45 360 4
-1300.000000 4300.000000 4600.000000 
-300.000000 1500.000000 4600.000000 
-300.000000 240.000000 4600.000000 
300.000000 240.000000 4600.000000 
0. 0. 4800.0
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1pad1#1001
trd1
BOX
air
250.000000 -250.000000 -67.500000
250.000000 250.000000 -67.500000
-250.000000 250.000000 -67.500000
-250.000000 -250.000000 -67.500000
250.000000 -250.000000 67.500000
250.000000 250.000000 67.500000
-250.000000 250.000000 67.500000
-250.000000 -250.000000 67.500000
500.000000 0.000000 -232.500000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1radiator
trd1mod1pad1#1001
BOX
polypropylene
230.000000 -230.000000 -14.500000
230.000000 230.000000 -14.500000
-230.000000 230.000000 -14.500000
-230.000000 -230.000000 -14.500000
230.000000 -230.000000 14.500000
230.000000 230.000000 14.500000
-230.000000 230.000000 14.500000
-230.000000 -230.000000 14.500000
0.000000 0.000000 -53.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1gas
trd1mod1pad1#1001
BOX
TRDgas
230.000000 -230.000000 -3.000000
230.000000 230.000000 -3.000000
-230.000000 230.000000 -3.000000
-230.000000 -230.000000 -3.000000
230.000000 -230.000000 3.000000
230.000000 230.000000 3.000000
-230.000000 230.000000 3.000000
-230.000000 -230.000000 3.000000
0.000000 0.000000 -35.500000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1padplane
trd1mod1pad1#1001
BOX
goldcoatedcopper
230.000000 -230.000000 -0.015000
230.000000 230.000000 -0.015000
-230.000000 230.000000 -0.015000
-230.000000 -230.000000 -0.015000
230.000000 -230.000000 0.015000
230.000000 230.000000 0.015000
-230.000000 230.000000 0.015000
-230.000000 -230.000000 0.015000
0.000000 0.000000 -32.485001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1mylar
trd1mod1pad1#1001
BOX
mylar
230.000000 -230.000000 -0.750000
230.000000 230.000000 -0.750000
-230.000000 230.000000 -0.750000
-230.000000 -230.000000 -0.750000
230.000000 -230.000000 0.750000
230.000000 230.000000 0.750000
-230.000000 230.000000 0.750000
-230.000000 -230.000000 0.750000
0.000000 0.000000 -31.720001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1electronics
trd1mod1pad1#1001
BOX
goldcoatedcopper
230.000000 -230.000000 -0.035000
230.000000 230.000000 -0.035000
-230.000000 230.000000 -0.035000
-230.000000 -230.000000 -0.035000
230.000000 -230.000000 0.035000
230.000000 230.000000 0.035000
-230.000000 230.000000 0.035000
-230.000000 -230.000000 0.035000
0.000000 0.000000 -30.935001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame1#1
trd1mod1pad1#1001
BOX
G10
250.000000 -10.000000 -18.299999
250.000000 10.000000 -18.299999
-250.000000 10.000000 -18.299999
-250.000000 -10.000000 -18.299999
250.000000 -10.000000 18.299999
250.000000 10.000000 18.299999
-250.000000 10.000000 18.299999
-250.000000 -10.000000 18.299999
0.000000 240.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame1#2
trd1mod1pad1#1001
0.000000 -240.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame2#1
trd1mod1pad1#1001
BOX
G10
10.000000 -230.000000 -18.299999
10.000000 230.000000 -18.299999
-10.000000 230.000000 -18.299999
-10.000000 -230.000000 -18.299999
10.000000 -230.000000 18.299999
10.000000 230.000000 18.299999
-10.000000 230.000000 18.299999
-10.000000 -230.000000 18.299999
240.000000 0.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame2#2
trd1mod1pad1#1001
-240.000000 0.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1fee1
trd1mod1pad1#1001
BOX
air
250.000000 -250.000000 -25.000000
250.000000 250.000000 -25.000000
-250.000000 250.000000 -25.000000
-250.000000 -250.000000 -25.000000
250.000000 -250.000000 25.000000
250.000000 250.000000 25.000000
-250.000000 250.000000 25.000000
-250.000000 -250.000000 25.000000
0.000000 0.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#1
trd1mod1fee1
BOX
polypropylene
230.000000 -2.000000 -25.000000
230.000000 2.000000 -25.000000
-230.000000 2.000000 -25.000000
-230.000000 -2.000000 -25.000000
230.000000 -2.000000 25.000000
230.000000 2.000000 25.000000
-230.000000 2.000000 25.000000
-230.000000 -2.000000 25.000000
0.000000 0.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#2
trd1mod1fee1
0.000000 50.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#3
trd1mod1fee1
0.000000 100.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#4
trd1mod1fee1
0.000000 150.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#5
trd1mod1fee1
0.000000 200.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#6
trd1mod1fee1
0.000000 -50.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#7
trd1mod1fee1
0.000000 -100.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#8
trd1mod1fee1
0.000000 -150.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#9
trd1mod1fee1
0.000000 -200.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
// ================================
//*********************************
trd1mod1pad1#1002
trd1
0.000000 500.000000 -232.500000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1pad1#1003
trd1
-500.000000 -0.000000 -232.500000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1pad1#1004
trd1
-0.000000 -500.000000 -232.500000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
// ================================
//*********************************
trd1mod1pad2#1005
trd1
BOX
air
250.000000 -250.000000 -67.500000
250.000000 250.000000 -67.500000
-250.000000 250.000000 -67.500000
-250.000000 -250.000000 -67.500000
250.000000 -250.000000 67.500000
250.000000 250.000000 67.500000
-250.000000 250.000000 67.500000
-250.000000 -250.000000 67.500000
500.000000 500.000000 -232.500000
0.  1.  0.  -1.  0.  0.  0.  0.  1.
//*********************************
// ================================
//*********************************
trd1mod1radiator
trd1mod1pad2#1005
BOX
polypropylene
230.000000 -230.000000 -14.500000
230.000000 230.000000 -14.500000
-230.000000 230.000000 -14.500000
-230.000000 -230.000000 -14.500000
230.000000 -230.000000 14.500000
230.000000 230.000000 14.500000
-230.000000 230.000000 14.500000
-230.000000 -230.000000 14.500000
0.000000 0.000000 -53.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1gas
trd1mod1pad2#1005
BOX
TRDgas
230.000000 -230.000000 -3.000000
230.000000 230.000000 -3.000000
-230.000000 230.000000 -3.000000
-230.000000 -230.000000 -3.000000
230.000000 -230.000000 3.000000
230.000000 230.000000 3.000000
-230.000000 230.000000 3.000000
-230.000000 -230.000000 3.000000
0.000000 0.000000 -35.500000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1padplane
trd1mod1pad2#1005
BOX
goldcoatedcopper
230.000000 -230.000000 -0.015000
230.000000 230.000000 -0.015000
-230.000000 230.000000 -0.015000
-230.000000 -230.000000 -0.015000
230.000000 -230.000000 0.015000
230.000000 230.000000 0.015000
-230.000000 230.000000 0.015000
-230.000000 -230.000000 0.015000
0.000000 0.000000 -32.485001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1mylar
trd1mod1pad2#1005
BOX
mylar
230.000000 -230.000000 -0.750000
230.000000 230.000000 -0.750000
-230.000000 230.000000 -0.750000
-230.000000 -230.000000 -0.750000
230.000000 -230.000000 0.750000
230.000000 230.000000 0.750000
-230.000000 230.000000 0.750000
-230.000000 -230.000000 0.750000
0.000000 0.000000 -31.720001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1electronics
trd1mod1pad2#1005
BOX
goldcoatedcopper
230.000000 -230.000000 -0.035000
230.000000 230.000000 -0.035000
-230.000000 230.000000 -0.035000
-230.000000 -230.000000 -0.035000
230.000000 -230.000000 0.035000
230.000000 230.000000 0.035000
-230.000000 230.000000 0.035000
-230.000000 -230.000000 0.035000
0.000000 0.000000 -30.935001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame1#1
trd1mod1pad2#1005
BOX
G10
250.000000 -10.000000 -18.299999
250.000000 10.000000 -18.299999
-250.000000 10.000000 -18.299999
-250.000000 -10.000000 -18.299999
250.000000 -10.000000 18.299999
250.000000 10.000000 18.299999
-250.000000 10.000000 18.299999
-250.000000 -10.000000 18.299999
0.000000 240.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame1#2
trd1mod1pad2#1005
0.000000 -240.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame2#1
trd1mod1pad2#1005
BOX
G10
10.000000 -230.000000 -18.299999
10.000000 230.000000 -18.299999
-10.000000 230.000000 -18.299999
-10.000000 -230.000000 -18.299999
10.000000 -230.000000 18.299999
10.000000 230.000000 18.299999
-10.000000 230.000000 18.299999
-10.000000 -230.000000 18.299999
240.000000 0.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1frame2#2
trd1mod1pad2#1005
-240.000000 0.000000 -49.200001
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1fee2
trd1mod1pad2#1005
BOX
air
250.000000 -250.000000 -25.000000
250.000000 250.000000 -25.000000
-250.000000 250.000000 -25.000000
-250.000000 -250.000000 -25.000000
250.000000 -250.000000 25.000000
250.000000 250.000000 25.000000
-250.000000 250.000000 25.000000
-250.000000 -250.000000 25.000000
0.000000 0.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#1
trd1mod1fee2
0.000000 0.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#2
trd1mod1fee2
0.000000 100.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#3
trd1mod1fee2
0.000000 200.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#4
trd1mod1fee2
0.000000 -100.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1feb#5
trd1mod1fee2
0.000000 -200.000000 0.000000
1.  0.  0.  0.  1.  0.  0.  0.  1.
//*********************************
// ================================
//*********************************
trd1mod1pad2#1006
trd1
-500.000000 500.000000 -232.500000
0.  1.  0.  -1.  0.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1pad2#1007
trd1
-500.000000 -500.000000 -232.500000
0.  1.  0.  -1.  0.  0.  0.  0.  1.
//*********************************
//*********************************
trd1mod1pad2#1008
trd1
500.000000 -500.000000 -232.500000
0.  1.  0.  -1.  0.  0.  0.  0.  1.
//*********************************
