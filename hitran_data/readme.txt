co2amp will scan all .par files in hitran_data directory for ro-vibrational transitions
in supported vibrational bands

Make sure that transitions are not duplicated in multiple files!

The following bands are supported as of 2025-12-17

   -------------------------------------------------------------
  |band #|   levels (HITRAN)    | levels (co2amp) | description |
  |-------------------------------------------------------------|
  |  0   | 00^01(1) -> 10^00(1) |    0 -> 1       | reg 10 um   |
  |  1   | 00^01(1) -> 10^00(2) |    0 -> 2       | reg  9 um   |
  |  2e  | 01^11(1) -> 11^10(1) |    5 -> 7  (e)  | hot 10 um   |
  |  2f  |                      |    6 -> 8  (f)  | "           |
  |  3e  | 01^11(1) -> 11^10(2) |    5 -> 9  (e)  | hot  9 um   |
  |  3f  |                      |    6 -> 10 (f)  | "           |
  |  4   | 00^02(1) -> 10^01(1) |   13 -> 14      | seq 10 um   |
  |  5   | 00^02(1) -> 10^01(2) |   13 -> 15      | seq  9 um   |
  |  6   | 10^01(1) -> 10^00(1) |   14 -> 1       | 4um         |
  |  7   | 10^01(2) -> 10^00(2) |   15 -> 2       | "           |
  |  8e  | 02^21(1) -> 02^20(1) |   16 -> 3  (e)  | "           |
  |  8f  |                      |   17 -> 4  (f)  | "           |
   -------------------------------------------------------------
	 

"estimate" .par files must have 160-character lines and include following data on proper positions in the lines:
- molecule ID ("2" for CO2)
- isotopologue ID
- wavenumber
- Upper vibrational level
- Lower vibrational level
- Rotational transition (J of lower state, P/R, e/f symmetry)

example (P14 ro-vibrational transition of band #4 of '628' isotopologue (HITRAN ID "3"):
 23  953.555835           3.100E-01                                       0 0 0 21       1 0 0 11                    P 14e                                      
(note spaces in the beginning and the end of the line!!!)
