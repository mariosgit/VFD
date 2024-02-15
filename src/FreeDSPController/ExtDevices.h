
#if 0

Param Address:  0x0000
Param Name:  SingleBandLevelDet2TCONST_2 0
Param Data:
0x00, 0x00, 0x4E, 0xB0
Data 5.32:
0.002401352
Data 28.0:
20144



Param Address:  0x0001
Param Name:  SingleBandLevelDet2hold_2 1
Param Data:
0x00, 0x00, 0x01, 0xE0
Data 5.32:
5.722046E-05
Data 28.0:
480



Param Address:  0x0002
Param Name:  SingleBandLevelDet2decay_2 2
Param Data:
0x00, 0x00, 0x07, 0x1C
Data 5.32:
0.0002169609
Data 28.0:
1820



Param Address:  0x0003
Param Name:  SingleBandLevelDet1TCONST_3 3
Param Data:
0x00, 0x00, 0x4E, 0xB0
Data 5.32:
0.002401352
Data 28.0:
20144



Param Address:  0x0004
Param Name:  SingleBandLevelDet1hold_3 4
Param Data:
0x00, 0x00, 0x01, 0xE0
Data 5.32:
5.722046E-05
Data 28.0:
480



Param Address:  0x0005
Param Name:  SingleBandLevelDet1decay_3 5
Param Data:
0x00, 0x00, 0x07, 0x1C
Data 5.32:
0.0002169609
Data 28.0:
1820






//////////////////////////////////////////////////
// Spacializer

Block Write
Time:  0:12:24 - 508ms
IC:  IC 1
Cell Name:  Nx2-1
Param Name:  StMuxSwSlew1coeffname
Param Address:  0x0055
Param Value:  1
Bytes:  4
Param Data:
0x00, 	0x00, 	0x00, 	0x01



Block Write
Time:  0:12:26 - 837ms
IC:  IC 1
Cell Name:  Nx2-1
Param Name:  StMuxSwSlew1coeffname
Param Address:  0x0055
Param Value:  0
Bytes:  4
Param Data:
0x00, 	0x00, 	0x00, 	0x00


//////////////////////////////////////////////////
// Volume

Safeload Write
Time:  0:14:2 - 110ms
IC:  IC 1
Cell Name:  SW vol 1
Param Name:  SWGain1940DBAlg1target
Param Address:  0x007E
Param Value:  0.0229086875915527
Bytes:  4
Param Data:
0x00, 	0x02, 	0xEE, 	0xAC



Safeload Write
Time:  0:14:2 - 110ms
IC:  IC 1
Cell Name:  SW vol 1
Param Name:  SWGain1940DBAlg1step
Param Address:  0x007F
Param Value:  0.000244140625
Bytes:  4
Param Data:
0x00, 	0x00, 	0x08, 	0x00





//////////////////////////////////////////////////
// Boost Freq

Safeload Write
Time:  0:14:44 - 70ms
IC:  IC 1
Cell Name:  Bass Boost1
Param Name:  BassBAlg_stereo1freq_varq
Param Address:  0x0056
Param Value:  0.00615227222442627
Bytes:  4
Param Data:
0x00, 	0x00, 	0xC9, 	0x99



Safeload Write
Time:  0:14:44 - 70ms
IC:  IC 1
Cell Name:  Bass Boost1
Param Name:  BassBAlg_stereo1freq_varq
Param Address:  0x0056
Param Value:  0.00615227222442627
Bytes:  4
Param Data:
0x00, 	0x00, 	0xC9, 	0x99





//////////////////////////////////////////////////
// Spacializer



//////////////////////////////////////////////////
// Spacializer




//////////////////////////////////////////////////
// Spacializer



//////////////////////////////////////////////////
// Spacializer





#endif
