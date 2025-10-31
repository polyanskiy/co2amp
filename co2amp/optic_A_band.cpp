#include  "co2amp.h"


void A::AmplificationBand(void)
{
    //  DEFINITIONS

    //  Isotopologue numbers
    //  -------------------------------
    // | i  | isotopologue | HITRAN ID |
    // |----|--------------|-----------|
    // | 0  |     626      |     1     |
    // | 1  |     727      |     9     |
    // | 2  |     828      |     7     |
    // | 3  |     636      |     2     |
    // | 4  |     737      |     B     |
    // | 5  |     838      |     0     |
    // | 6  |     627      |     4     |
    // | 7  |     628      |     3     |
    // | 8  |     728      |     8     |
    // | 9  |     637      |     6     |
    // | 10 |     638      |     5     |
    // | 11 |     738      |     A     |
    //  -------------------------------

    // isotopologue namas (for output only)
    std::string isotopologue[12] =
        {"626", "727", "828", "636", "737", "838", "627", "628", "728", "637", "638", "738"};

    // map HITRAN isotopologue id's (0...B) to co2amp numbering (0...11)
    char isot_map[12] =
        { '1',   '9',   '7',   '2',   'B',   '0',   '4',   '3',   '8',   '6',   '5',   'A'};


    //  Vibrational levels
    //  --------------------------------------------------------------------------------
    // | gr | vl |  level    | parity | weight |              description               |
    // |--------------------------------------------------------------------------------|
    // |                                     0 0 1                                      |
    // |                                                                                |
    // |  0 |  0 | 00^01(1)e |   u    |   1    | upper level of regular band            |
    // |--------------------------------------------------------------------------------|
    // |                                 1 0 0 + 0 2 0                                  |
    // |                                                                                |
    // |  1 |  1 | 10^00(1)e |   g    |  1/3   | 1,2:     lower levels of reg bands     |
    // |    |  2 | 10^00(2)e |   g    |  1/3   | 1,2,3,4: lower levels of 4um bands     |
    // |    |  3 | 02^20(1)e |   g    |  1/6   |                                        |
    // |    |  4 | 02^20(1)f |   g    |  1/6   | 1 & 2: mixed [10^00, 02^00] levels     |
    // |--------------------------------------------------------------------------------|
    // |                                     0 1 1                                      |
    // |                                                                                |
    // |  2 |  5 | 01^11(1)e |   u    |  1/2   | upper levels of hot bands              |
    // |    |  6 | 01^11(1)f |   u    |  1/2   |                                        |
    // |--------------------------------------------------------------------------------|
    // |                                 1 1 0 + 0 3 0                                  |
    // |                                                                                |
    // |  3 |  7 | 11^10(1)e |   u    |  3/16  | 7,8,9,10: lower levels of hot bands    |
    // |    |  8 | 11^10(2)e |   u    |  3/16  | 11,12: not currently included in       |
    // |    |  9 | 11^10(1)f |   u    |  3/16  |        the amplification model         |
    // |    | 10 | 11^10(2)f |   u    |  3/16  |                                        |
    // |    | 11 | 03^30(1)e |   u    |  1/8   | 7 & 8:  mixed [11^10e, 03^10e] levels  |
    // |    | 12 | 03^30(1)f |   u    |  1/8   | 9 & 10: mixed [11^10f, 03^10f] levels  |
    // |--------------------------------------------------------------------------------|
    // |                                     0 0 2                                      |
    // |                                                                                |
    // |  4 | 13 | 00^02(1)e |   g    |   1    | upper level of sequence band           |
    // |--------------------------------------------------------------------------------|
    // |                                 1 0 1 + 0 2 1                                  |
    // |                                                                                |
    // |  5 | 14 | 10^01(1)e |   u    |  1/3   | 14,15:       lower levels of seq bands |
    // |    | 15 | 10^01(2)e |   u    |  1/3   | 14,15,16,17: upper levels of 4um bands |
    // |    | 16 | 02^21(1)e |   u    |  1/6   |                                        |
    // |    | 17 | 02^21(1)f |   u    |  1/6   | 14 & 15: mixed [10^01, 02^01] levels   |
    // |--------------------------------------------------------------------------------|
    // |                                     0 1 2                                      |
    // |                                                                                |
    // |  6 |    |           |        |        | Levels and transitions to be added     |
    // |--------------------------------------------------------------------------------|
    // |                                 1 1 1 + 0 3 1                                  |
    // |                                                                                |
    // |  7 |                                    Levels and transitions to be added     |
    // |--------------------------------------------------------------------------------|
    // |                                     0 0 3                                      |
    // |                                                                                |
    // |  8 |                                    Levels and transitions to be added     |
    // |--------------------------------------------------------------------------------|
    // |                                 1 0 2 + 0 2 2                                  |
    // |                                                                                |
    // |  9 |                                    Levels and transitions to be added     |
    //  --------------------------------------------------------------------------------

    // Vibrational transitions ("bands" in HITRAN terminology)
    //  -----------------------------------------------------------
    // |band|   levels (HITRAN)    | levels (co2amp) | description |
    //  -----------------------------------------------------------
    // |  0 | 00^01(1) -> 10^00(1) |    0 -> 1       | reg 10 um   |
    // |  1 | 00^01(1) -> 10^00(2) |    0 -> 2       | reg  9 um   |
    // |  2 | 01^11(1) -> 11^10(1) |    5 -> 7  (e)  | hot 10 um   |
    // |    |                      |    6 -> 9  (f)  | "           |
    // |  3 | 01^11(1) -> 11^10(2) |    5 -> 8  (e)  | hot  9 um   |
    // |    |                      |    6 -> 10 (f)  | "           |
    // |  4 | 00^02(1) -> 10^01(1) |   13 -> 14      | seq 10 um   |
    // |  5 | 00^02(1) -> 10^01(2) |   13 -> 15      | seq  9 um   |
    // |  6 | 10^01(1) -> 10^00(1) |   14 -> 1       | 4um         |
    // |  7 | 10^01(2) -> 10^00(2) |   15 -> 2       | "           |
    // |  8 | 02^21(1) -> 02^20(1) |   16 -> 3  (e)  | "           |
    // |    |                      |   17 -> 4  (f)  | "           |
    //  -----------------------------------------------------------

    double k = 1.3806488e-23;                                         // Boltzmann's constant, J/K
    double T2 = 1e-6 / (M_PI*7.61*750*(p_CO2+0.733*p_N2+0.64*p_He));  // transition dipole dephasing time, s
    double gamma = 1 / T2;   // Lorentzian HWHM


    // Rotational constants B, Hz
    // B[i][vl]
    // source: fit of HITRAN data
    double B[NumIso][NumVib] = {
    // vl:  0         1         2         3         4         5         6         7         8         9         10        11        12        13        14        15        16        17
        {1.16E+10, 1.17E+10, 1.16E+10, 1.17E+10, 1.17E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.18E+10, 1.18E+10, 1.15E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.17E+10}, //626
        {1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.09E+10, 1.09E+10, 1.09E+10, 1.10E+10, 1.10E+10}, //727
        {1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.05E+10, 1.05E+10, 1.02E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10}, //828
        {1.16E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.18E+10, 1.17E+10, 1.17E+10, 1.18E+10, 1.18E+10, 1.15E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.17E+10}, //636
        {1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.09E+10, 1.10E+10,        0,        0,        0,        0,        0,        0,        0, 1.09E+10, 1.09E+10, 1.10E+10, 1.10E+10}, //737
        {1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.05E+10, 1.05E+10, 1.02E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10}, //838
        {1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.13E+10, 1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.12E+10, 1.13E+10, 1.13E+10, 1.13E+10, 1.13E+10}, //627
        {1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10}, //628
        {1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.08E+10, 1.08E+10, 1.05E+10, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10}, //728
        {1.13E+10, 1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.13E+10, 1.13E+10, 1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.12E+10, 1.13E+10, 1.13E+10, 1.13E+10, 1.13E+10}, //637
        {1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.09E+10, 1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10}, //638
        {1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.08E+10, 1.08E+10,        0, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10}  //738
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

        Debug(2, "Reading HITRAN file \'" + fileName + "\' (use debug level 3 to display transitions)...");
        Debug(3, "Transitions to be included in calculations:");

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
            int i; // isotopologue number in co2amp
            for (i=0; i<NumIso; ++i)
            {
                if (isot_map[i] == isot_id)
                {
                    break; // exit the loop if a match is found
                }
            }
            if(i == NumIso) // no match found => not one of the supported isotopologues
                continue;

            if(N_iso[i]==0.0) // skip zero-content isotopologues
                continue;

            // vibrational bands
            // Don't add data to the arrays yet (must make sure the rotational numbers of both levels are within 0...79 range
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

            // Read rotational level info and calculate transition frequency
            int J = std::stoi(line.substr(118, 3));    // Rotational J number (lower level)
            char pqr = line[117];                      // Rotational brunch ('P', 'Q' or 'R')
            char ef = line[121];                       // 'e' or 'f' sublevel
            double wn = std::stod(line.substr(3, 12)); // Transition wavenumber 1/cm
            double v_Hz = wn*c*100;                    // Transition frequency v[Hz] = v[1/cm] * c[cm/s]

            // Ignore transitions centered outside of the calculation window defined by the pulse_time grid
            double Dv = 1.0/(t_max-t_min);   // frequency step, Hz
            if(v_Hz<v0-Dv*n0/2 || v_Hz>v0+Dv*n0/2)
                continue;

            // Do final checks (rotational numbers of involved levels are 0...79)
            // and start adding data to the arrays defining transitions
            // j_up, j_lo, vl_up, vl_lo
            switch(pqr)
            {
                case 'P':
                    if(J>79 || J<1)
                        continue;
                    j_lo[i].push_back(J);
                    j_up[i].push_back(J-1);
                    break;
                case 'R':
                    if(J>78)
                        continue;
                    j_lo[i].push_back(J);
                    j_up[i].push_back(J+1);
                    break;
                case 'Q':
                    continue; // ignore Q branch
                    /*if(J>79)
                        continue;
                    j_lo[i].push_back(J);
                    j_up[i].push_back(J);*/
                    break;
            }

            // Transition frequency (only add to array after checking that J's are within the intended range)
            v[i].push_back(v_Hz);

            // Associate vibrational levels with the transition
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
                        vl_up[i].push_back(5);
                        vl_lo[i].push_back(7);
                    }
                    else //f
                    {
                        vl_up[i].push_back(6);
                        vl_lo[i].push_back(9);
                    }
                    break;
                case 3: // hot 9 um
                    if(ef == 'e')
                    {
                        vl_up[i].push_back(5);
                        vl_lo[i].push_back(8);
                    }
                    else //f
                    {
                        vl_up[i].push_back(6);
                        vl_lo[i].push_back(10);
                    }
                    break;
                case 4: // seq 10 um
                    vl_up[i].push_back(13);
                    vl_lo[i].push_back(14);
                    break;
                case 5: // seq 9 um
                    vl_up[i].push_back(13);
                    vl_lo[i].push_back(15);
                    break;
                case 6: // 4um-1
                    vl_up[i].push_back(14);
                    vl_lo[i].push_back(1);
                    break;
                case 7: // 4um-2
                    vl_up[i].push_back(15);
                    vl_lo[i].push_back(2);
                    break;
                case 8: // 4um-3
                    if(ef == 'e')
                    {
                        vl_up[i].push_back(16);
                        vl_lo[i].push_back(3);
                    }
                    else //f
                    {
                        vl_up[i].push_back(17);
                        vl_lo[i].push_back(4);
                    }
                    break;
            }

            // Transition cross-sections, m^2
            double A = std::stod(line.substr(25, 10)); // Einstein coefficient A (1/s)

            sigma[i].push_back( pow(1/(wn*100),2) * A / 4 / (M_PI*gamma) ); // wn*100: 1/cm -> 1/m

            //if(J==20)
            if(debug_level >= 3)
            {
                std::cout << "  "
                          << "Isot: " << isotopologue[i] << "; "
                          << "Band:" << Vup_id << " ->" << Vlo_id << " " << std::string(1,pqr) << std::to_string(J) << std::string(1,ef) << "; "
                          << "freq = " << std::to_string(v[i].back()/1e12) << " THz; "
                          << "A = " + std::to_string(A) + " 1/s"
                          << std::endl;
            }
        }
        Debug(2, "Finished reading file");
    }


    // Normalized populations of rotational sublevels
    // nop[i][vl][j]
    for(int i=0; i<NumIso; ++i)
    {
        for(int vl=0; vl<NumVib; ++vl)
        {
            // Chsrcteristics of vibraitonal levels
            //              vl: 0      1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17
            int l[]         = { 0,     0,     0,     2,     2,     1,     1,     1,     1,     1,    1,     3,     3,     0,     0,     0,     2,     2    };
            char parity[]   = {'u',   'g',   'g',   'g',   'g',   'g',   'g',   'u',   'u',   'u',   'u',   'u',   'u',   'g',   'u',   'u',   'u',   'u'  };
            char symmetry[] = {'e',   'e',   'e',   'e',   'f',   'e',   'f',   'e',   'e',   'f',   'f',   'e',   'f',   'e',   'e',   'e',   'e',   'f'  };

            //general expression for energy distribution between rotational sub-levels
            for(int j=0; j<NumRot; ++j)
            {
                nop[i][vl][j] = h*B[i][vl]/(k*T0) * (2*j+1) * exp(-h*B[i][vl]/(k*T0)*j*(j+1));
            }

            // vibrational levels with J<l are not populated
            for(int j=0; j<l[vl]; ++j)
            {
                nop[i][vl][j] = 0;
            }

            if(i<6) // symmetric molecules (626, 727, 828, 636, 737, 838) have forbidden rottational sub-levels
            {
                if( (parity[vl]=='g' && symmetry[vl]=='e') || (parity[vl]=='u' && symmetry[vl]=='f') ) // only even J's populated
                {
                    for(int j=1; j<NumRot; j+=2)
                    {
                        nop[i][vl][j] = 0; // odd J's not populated
                    }
                } // only odd J's populated
                else
                {
                    for(int j=0; j<NumRot; j+=2)
                    {
                        nop[i][vl][j] = 0; // even J's not populated
                    }
                }

                // double sub-level's population to keep total polulation of vibrational level
                for(int j=0; j<NumRot; ++j)
                {
                    nop[i][vl][j] *= 2;
                }
            }
        }
    }
}
