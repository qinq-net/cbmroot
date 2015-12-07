// Revision 1.3, 14/03/2015 shabir
// Beam-pipe for MUCH, material: Aluminium
// till 110cm no changes, cone from z = 110 (at 1.25 deg) -120 (at 4.3 deg)
// lead as part of beam pipe as well as 1st absorber shielding
// from z = 120 - 180 cm and from 2.9 - 5.7 deg   
// from z >180 beam-pipe r_in = Z*tan(2.9)+10mm, 1cm left for clamp connection
// 5mm beam-pipe thickness from 180cm

//  $Id: pipe_newvac.geo,v 1.2 2006/02/03 14:16:13 hoehne Exp $

//  History of cvs commits:
//  $Log: pipe_newvac.geo,v $
//  Revision 1.2  2006/02/03 14:16:13  hoehne
//  update because of slightly modified rich.geo
//
//  Revision 1.1  2005/09/20 09:27:49  friese
//  Modified beam pipe in sts
//
//  - Revision   Johann M. Heuser, 21.7.2005
//  - new geometry for the vacuum section: barrel-like shape
//  - diameter at z=1700: reduced from
// Revision for Much: continuous pipe
//----------------------------------------------------------
pipe1
cave
PCON 
aluminium
7
0. 360.
 -50.  25.  25.5
  25.  25.  25.5
  35. 400. 400.5
 230. 400. 400.5
 240.  10.  10.5
1100.  24.  24.5
1250.  94.  94.5
0. 0. 0.
1.  0.  0.  0.  1.  0.  0.  0.  1

pipevac1
cave
PCON
vacuum
7
0. 360.  
 -50.  0.  25.   
  25.  0.  25.  
  35.  0. 400.
 230.  0. 400.   
 240.  0.  10.
1100.  0.  24.
1250.  0.  93.
0. 0. 0.
1.  0.  0.  0. 1.  0.  0. 0. 1

pipe2
cave
PCON 
lead
2
0. 360.
1250.  63. 124.
1850.  94. 184.
0. 0. 0.
1.  0.  0.  0.  1.  0.  0.  0.  1

pipevac2
cave
PCON 
vacuum
2
0. 360.
1250. 0. 63.
1850. 0. 94.
0. 0. 0.
1.  0.  0.  0.  1.  0.  0.  0.  1

pipe3
cave
PCON 
aluminium
4
0. 360.
1850.  104. 109
4000.  212. 217
7000.  212. 217
7050.  135. 140
0. 0. 0.
1.  0.  0.  0.  1.  0.  0.  0.  1

pipevac3
cave
PCON
vacuum
4
0. 360.  
1850.  0. 104.  
4000.  0. 212.
7000.  0. 212.
7050.  0. 135.
0. 0. 0.
1.  0.  0.  0. 1.  0.  0. 0. 1
//
