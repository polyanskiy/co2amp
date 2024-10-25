Include the following transitions when generating the par file in HITRAN

0001(1) -> 1000(1)   regular  10µm
0001(1) -> 1000(2)            9 µm
------------------------------------
0111(1) -> 1110(1)   hot      10 µm
0111(1) -> 1110(2)            9 µm
------------------------------------
0002(1) -> 1001(1)   sequence 10 µm
0002(1) -> 1001(2)            9 µm
------------------------------------
1001(1) -> 1000(1)   "4.5 µm"
1001(2) -> 1000(2)
1221(1) -> 0220(1)

co2amp will scan all .par files in hitran_data directory for supported transitions
Make sure that transitions are not duplicated in multiple files.
