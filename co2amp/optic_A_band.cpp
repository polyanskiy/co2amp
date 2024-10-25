#include  "co2amp.h"


void A::AmplificationBand(void)
{
    //  DEFINITIONS

    //  Isotopologue numbers
    //  ------------------
    // | i | isotopologue |
    // |---|--------------|
    // | 0 |     626      |
    // | 1 |     628      |
    // | 2 |     828      |
    // | 3 |     636      |
    // | 4 |     638      |
    // | 5 |     838      |
    //  ------------------

    //  Vibrational levels
    //  --------------------------------------------------------------------------------------------
    // | vl |  level    |                               description                                 |
    // |----|-----------|---------------------------------------------------------------------------|
    // |  0 | 00^01(1)  |       upper level of regular band                                         |
    // |  1 | 10^00(1)  | lower level of 10 um regular band  &  lower level of    1st 4um band      |
    // |  2 | 10^00(2)  | lower level of  9 um regular band  &  lower level of    2nd 4um band      |
    // |  3 | 01^11(1)e |       upper level of hot band "e"                                         |
    // |  4 | 11^10(1)e | lower level of 10 um hot band "e"                                         |
    // |  5 | 11^10(2)e | lower level of  9 um hot band "e"                                         |
    // |  6 | 01^11(1)f |       upper level of hot band "f"                                         |
    // |  7 | 11^10(1)f | lower level of 10 um hot band "f"                                         |
    // |  8 | 11^10(2)f | lower level of  9 um hot band "f"                                         |
    // |  9 | 00^02(1)  |       upper level of sequence band                                        |
    // | 10 | 10^01(1)  | lower level of 10 um sequence band  &     upper level of 1st 4um band     |
    // | 11 | 10^01(2)  | lower level of  9 um sequence band  &     upper level of 2nd 4um band     |
    // | 12 | 02^21(1)e |                                           upper level of 3rd 4um band "e" |
    // | 13 | 02^20(1)e |                                        lower level of    3rd 4um band "e" |
    // | 14 | 02^21(1)f |                                           upper level of 3rd 4um band "f" |
    // | 15 | 02^20(1)f |                                        lower level of    3rd 4um band "f" |
    //  --------------------------------------------------------------------------------------------

    // Vibrational transitions ("bands" in HITRAN terminology)
    //  -----------------------------------------------------------
    // |band|   levels (HITRAN)    | levels (co2amp) | description |
    //  -----------------------------------------------------------
    // |  0 | 00^01(1) -> 10^00(1) |    0 -> 1       | reg 10 um   |
    // |  1 | 00^01(1) -> 10^00(2) |    0 -> 2       | reg  9 um   |
    // |  2 | 01^11(1) -> 11^10(1) |    3 -> 4 (e)   | hot 10 um   |
    // |    |                      |    6 -> 7 (f)   |             |
    // |  3 | 01^11(1) -> 11^10(2) |    3 -> 5 (e)   | hot  9 um   |
    // |    |                      |    6 -> 8 (f)   |             |
    // |  4 | 00^02(1) -> 10^01(1) |    9 -> 10      | seq 10 um   |
    // |  5 | 00^02(1) -> 10^01(2) |    9 -> 11      | seq  9 um   |
    // |  6 | 10^01(1) -> 10^00(1) |   10 -> 1       | 4um-1       |
    // |  7 | 10^01(2) -> 10^00(2) |   11 -> 2       | 4um-2       |
    // |  8 | 02^21(1) -> 02^20(1) |   12 -> 13 (e)  | 4um-3       |
    // |    |                      |   14 -> 15 (f)  |             |
    //  -----------------------------------------------------------

    double k = 1.3806488e-23;                                         // Boltzmann's constant, J/K
    double T2 = 1e-6 / (M_PI*7.61*750*(p_CO2+0.733*p_N2+0.64*p_He));  // transition dipole dephasing time, s
    double gamma = 1.0 / T2;   // Lorentzian HWHM


    // Rotational constants B, Hz
    // B[i][vl]
	// source: fit of HITRAN data (except 828 10um transition: Witteman The CO2 laser)
    double B[6][16] = {
        {1.1589e10, 1.1683e10, 1.1687e10, 1.1602e10, 1.1687e10, 1.1695e10, 1.1602e10, 1.1716e10, 1.1723e10, 1.1497e10, 1.1588e10, 1.1598e10, 1.15e10, 1.15e10, 1.15e10, 1.15e10}, //626
        {1.0936e10, 1.1034e10, 1.1019e10, 1.0949e10, 1.1036e10, 1.1032e10, 1.0965e10, 1.1063e10, 1.1055e10, 1.0859e10, 1.0955e10, 1.0946e10, 1.10e10, 1.10e10, 1.10e10, 1.10e10}, //628
        {1.0303e10, 1.0403e10, 1.0375e10, 1.0324e10, 1.0412e10, 1.0398e10, 1.0338e10, 1.0437e10, 1.0417e10, 1.03e10  , 1.03e10  , 1.03e10  , 1.03e10, 1.03e10, 1.03e10, 1.03e10}, //828
        {1.1593e10, 1.1668e10, 1.1700e10, 1.1605e10, 1.1676e10, 1.1702e10, 1.1623e10, 1.1703e10, 1.1733e10, 1.1512e10, 1.1585e10, 1.1623e10, 1.16e10, 1.16e10, 1.16e10, 1.16e10}, //636
        {1.0939e10, 1.1019e10, 1.1031e10, 1.0953e10, 1.1028e10, 1.1040e10, 1.0970e10, 1.1053e10, 1.1066e10, 0        , 0        , 0        , 1.10e10, 1.10e10, 1.10e10, 1.10e10}, //638
        {1.0315e10, 1.0403e10, 1.0394e10, 0        , 0        , 0        , 0        , 0        , 0        , 0        , 0        , 0        , 1.03e10, 1.03e10, 1.03e10, 1.03e10}  //838
    };


    // Populate transition arrays with HITRAN 2020 data

    // Find all .par files in the working directory
    std::vector<std::string> parFiles;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".par")
        {
            parFiles.push_back(entry.path().filename().string());
        }
    }

    // Loop over each file
    for (const auto& fileName : parFiles)
    {

        Debug(2, "Reading par file " + fileName + "...");

        std::ifstream infile(fileName);

        if (!infile)
        {
            std::cerr << "Error opening file: " << fileName << std::endl;
            continue; // Skip to the next file
        }

        std::string line;

        // Read each line from the file
        while (std::getline(infile, line))
        {
            // Ensure the line is 160 characters
            if (line.length() != 160)
                continue;

            // Molecule code
            std::string  mol_id = line.substr(0, 2);
            if(mol_id != " 2") // not CO2
                continue;

            // Isotopologue code in HITRAN
            char isot_id = line[2];
            char isot_map[6] =
                {
                '1', // 626
                '3', // 628
                '7', // 828
                '2', // 636
                '5', // 638
                '0'  // 838
                };
            int i; // isotopologue number in co2amp
            for (i = 0; i < 6; ++i)
            {
                if (isot_map[i] == isot_id)
                {
                    break; // exit the loop if a match is found
                }
            }
            if(i == 6) // no match found => not one of the supported isotopologues
                continue;

            // vibrational bands
            // Don't add data to the arrays yet (must make sure the rotational numbers of both levels are within 0...59 range
            std::string Vup_id = line.substr(73, 9);   // upper vibrational level e.g. " 0 1 1 11"
            std::string Vlo_id = line.substr(88, 9);   // lower vibrational level e.g. " 1 1 1 01"
            std::string band_map[9]=
                {
                " 0 0 0 11 -> 1 0 0 01", // reg 10 um
                " 0 0 0 11 -> 1 0 0 02", // reg  9 um
                " 0 1 1 11 -> 1 1 1 01", // hot 10 um (e,f)
                " 0 1 1 11 -> 1 1 1 02", // hot  9 um (e,f)
                " 0 0 0 21 -> 1 0 0 11", // seq 10 um
                " 0 0 0 21 -> 1 0 0 12", // seq  9 um
                " 1 0 0 11 -> 1 0 0 01", // 4um-1
                " 1 0 0 12 -> 1 0 0 02", // 4um-2
                " 0 2 2 11 -> 0 2 2 01"  // 4um-3 (e,f)
                };
            int band; // vibrational band number in co2amp
            for (band = 0; band < 9; ++band)
            {
                if (band_map[band] == Vup_id + " ->" + Vlo_id)
                {
                    break; // exit the loop if a match is found
                }
            }
            if(band == 9) // no match found => not one of the supported vibrational bands
                continue;
            // skip if the band excluded from calculations by user
            if( !band_reg && (band == 0 || band == 1) )
                continue;
            if( !band_hot && (band == 2 || band == 3) )
                continue;
            if( !band_seq && (band == 4 || band == 5) )
                continue;
            if( !band_4um && (band == 6 || band == 7 || band == 8) )
                continue;

            // Do final checks (rotational numbers of involved levels are 0...59)
            // and start adding data to the arrays defining transitions
            // j_up, j_lo, vl_up, vl_lo

            int J = std::stoi(line.substr(118, 3));    // Rotational J number (lower level)
            char pqr = line[117];                      // Rotational brunch ('P', 'Q' or 'R')
            char ef = line[121];                       // 'e' or 'f' sublevel

            switch(pqr)
            {
                case 'P':
                    if(J>59 || J<1)
                        continue;
                    j_lo[i].push_back(J);
                    j_up[i].push_back(J-1);
                    break;
                case 'R':
                    if(J>58)
                        continue;
                    j_lo[i].push_back(J);
                    j_up[i].push_back(J+1);
                    break;
                case 'Q':
                    if(J>59)
                        continue;
                    j_lo[i].push_back(J);
                    j_up[i].push_back(J);
                    break;
            }


            switch(band)
            {
                case 0: // reg 10 um
                    vl_up[i].push_back(0);
                    vl_lo[i].push_back(1);
                    break;
                case 1: // reg 9 um
                    vl_up[i].push_back(0);
                    vl_lo[i].push_back(2);
                    break;
                case 2: // hot 10 um
                    if(ef == 'e')
                    {
                        vl_up[i].push_back(3);
                        vl_lo[i].push_back(4);
                    }
                    else //f
                    {
                        vl_up[i].push_back(6);
                        vl_lo[i].push_back(7);
                    }
                    break;
                case 3: // hot 9 um
                    if(ef == 'e')
                    {
                        vl_up[i].push_back(3);
                        vl_lo[i].push_back(5);
                    }
                    else //f
                    {
                        vl_up[i].push_back(6);
                        vl_lo[i].push_back(8);
                    }
                    break;
                case 4: // seq 10 um
                    vl_up[i].push_back(9);
                    vl_lo[i].push_back(10);
                    break;
                case 5: // seq 9 um
                    vl_up[i].push_back(9);
                    vl_lo[i].push_back(11);
                    break;
                case 6: // 4um-1
                    vl_up[i].push_back(10);
                    vl_lo[i].push_back(1);
                    break;
                case 7: // 4um-2
                    vl_up[i].push_back(11);
                    vl_lo[i].push_back(2);
                    break;
                case 8: // 4um-3
                    if(ef == 'e')
                    {
                        vl_up[i].push_back(12);
                        vl_lo[i].push_back(13);
                    }
                    else //f
                    {
                        vl_up[i].push_back(14);
                        vl_lo[i].push_back(15);
                    }
                    break;
            }

            // Transition frequencies, Hz
            double wn = std::stod(line.substr(3, 12)); // Transition wavenumber (1/cm)
            v[i].push_back(wn*c*100); // v[Hz] = v[1/cm] * c[cm/s]
            // alternative: calculate transition frequencies from molecular constants (will need to define V[i][band])
            // double Bup = B[i][vl_up[i].back()];
            // double Blo = B[i][vl_lo[i].back()];
            // double Jup = j_up[i].back();
            // double Jlo = j_lo[i].back();
            // v[i].push_back(V[i][band] + Bup*Jup*(Jup[i]+1) - Blo*Jlo*(Jlo+1));


            // Transition cross-sections, m^2
            double A = std::stod(line.substr(25, 10)); // Einstein coefficient A (1/s)

            sigma[i].push_back( pow(1/(wn*100),2) * A / 4 / (M_PI*gamma) ); // wn*100: 1/cm -> 1/m

            if(J==20)
            {
                Debug(2, "Isot: " + std::to_string(i) +
                            "; Band:" + Vup_id + " ->" + Vlo_id + " " + std::string(1,pqr) + std::to_string(J) + std::string(1,ef) +
                            "; freq = " + std::to_string(v[i].back()/1e12) + " THz" +
                            "; A = " + std::to_string(A) + " 1/s");
            }
        }
    }



    // Normalized populations of rotational sublevels
    // nop[i][vl][j]
    for(int i=0; i<6; ++i)
    {
        for(int vl=0; vl<16; ++vl)
        {
            for(int j=0; j<60; ++j)
            {
                nop[i][vl][j] = h*B[i][vl]/(k*T0) * (2*j+1) * exp(-h*B[i][vl]/(k*T0)*j*(j+1));
            }

            // Symmetry of molecules
            // isotopologue:     626 628 828 636 638 838
            bool symmetric[6] = { 1 , 0 , 1 , 1 , 0 , 1 };

            // n2 quanta (+2*nu1)
            // defines angular momentum splitting of rotational sub-levels
            // vl:             0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
            int nu2[16]    = { 0, 2, 2, 1, 3, 3, 1, 3, 3, 0, 2, 2, 2, 2, 2, 2 };

            // is vibration symmetric (g)
            // vl:             0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
            //bool g[16]     = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };

            // is an f sub-level (if nu2>0)
            // vl:             0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
            //bool f[16]     = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };

            // is coupled by Fermi resonounce between nu1 and 2nu2
            // vl:             0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
            bool fermi[16] = { 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0  };

            if(symmetric[i]) // symmetric molecules
            {
                /*if(!f[vl]) // f
                {
                    for(int j=0; j<60; j+=2)
                    {
                        nop[i][vl][j] = 0; // only even J's not populated
                    }
                }
                else // e
                {
                    for(int j=1; j<60; j+=2)
                    {
                        nop[i][vl][j] = 0; // odd J's not populated
                    }
                }*/

                // double sub-level's population to keep total polulation of vibrational level
                for(int j=0; j<60; ++j)
                {
                    nop[i][vl][j] *= 2;
                }
            }

            // nu2+1 sub-levels in levels with nu2 quanta:
            // nu2 = 0: 1 sub-level  (l = 0)
            //       1: 2 sub-levels (l = -1, +1)
            //       2: 3 sub-levels (l = -2, 0, +2)
            //       3: 4 sub-levels (l = -3, -1, +1, +3)
            //       4: ...
            double frac = 1.0/(nu2[vl]+1);// fraction of total population of a vibrational level in one sub-level

            // if level is Fermi-loupled with another vibration, the populations are mixed up and distributed equally
            // we assume the other level is not split ("1" in the following expression)
            if(fermi[vl])
            {
                frac = (frac + 1) / 2;
            }

            // now, apply the fraction to total population
            for(int j=0; j<60; ++j)
            {
                nop[i][vl][j] *= frac;
            }


        }
    }

}
