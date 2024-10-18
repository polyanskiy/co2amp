Each data file contains information on the rotational lines of a single V-V transition for one CO₂ isotopologue. The data is presented in tab-delimited format, with each line representing one rotational transition in the following format:

nu(1/cm)   A(1/s)   J_U   J_L

where J_U and J_L are the rotational quantum numbers of the upper and lower levels of the transition (labeled as J' and J", respectively) in the HITRAN database.

While all rotational lines listed in HITRAN for a specific V-V transition can be included, only the lines with both rotational quantum numbers less than 60 (0...59) are considered by the co2amp model.
