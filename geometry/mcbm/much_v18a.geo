# Remarks:
# 1. Absorber and station positions are defined relative to MuchCave Zin
#
# 2. When defining distances between absorbers and stations keep in mind 
#    the following technological restrictions:
#    a) distance from layer center to absorber >=   5 cm 
#    b) distance between layer centers         >=  10 cm 
#
# 3. Absorber materials currently implemented:
#    I - MUCHiron
#    L - MUCHlead
#    W - MUCHwolfram
#    C - MUCHcarbon
#
# 4. There is a possibility to switch off detailed module design 
#    (Use module design = 0). One large module per layer is created 
#    in this case (at the front side of each layer).
# 
# 5. If detector type = 2 (straws), support will not be created. 

# General information
MuchCave Zin position [cm]  :     195
Acceptance tangent min      :     0.1
Acceptance tangent max      :     0.5
Number of absorbers         :       0
Number of stations          :       1

# Absorber specification
Absorber Zin position [cm]  :       0      50      100    160
Absorber thickness [cm]     :      20      20      20      30
Absorber material           :       C       I       I       I

# Station specification
Station Zcenter [cm]        :       0      85     140     210
Number of layers            :       2       3       4       4
Detector type               :       3       3       2       2
Distance between layers [cm]:      10      10      10      10
Support thickness [cm]      :     1.5     1.5     1.5     1.5
Use module design (0/1)     :       1       1       0       0

# GEM module specification (type 1)
Active volume lx [cm]       :    25.6
Active volume ly [cm]       :    25.6
Active volume lz [cm]       :     0.4
Spacer lx [cm]              :     0.5
Spacer ly [cm]              :       5
Overlap along y axis [cm]   :       2

# Straw module specification (type 2)
Straw thickness [cm]        :     0.6

# Sector-type GEM module specification (type 3)
Number of sectors/layer     :      14
Active volume lz [cm]       :     0.3
Spacer in r [cm]            :       2
Spacer in phi [cm]          :       2
Overlap in r [cm]           :       2
