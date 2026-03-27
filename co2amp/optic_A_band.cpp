#include  "co2amp.h"


void A::AmplificationBand(void)
{
    //  DEFINITIONS

    //  Isotopologue numbers
    //  -------------------------------
    // | is | isotopologue | HITRAN ID |
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
    //  ------------------------------------------------------------------
    // | vl |  level    | parity |              description               |
    // |------------------------------------------------------------------|
    // |                          Ground state                            |
    // |                                                                  |
    // |  0 | 00^00(1)e |   g    | Upper level of regular band            |
    // |------------------------------------------------------------------|
    // |                       0 0 1, 0 0 2, 0 0 3                        |
    // |                                                                  |
    // |  1 | 00^01(1)e |   u    | Upper level of regular band;           |
    // |                           Optical pumping @ 4.3 um               |
    // |  2 | 00^02(1)e |   g    | Upper level of sequence band;          |
    // |                           Optical pumping @ 2.2 um (asymm. mol.) |
    // |  3 | 00^03(1)e |   u    | Optical pumping @ 1.44 um              |
    // |------------------------------------------------------------------|
    // |                          1 0 0 + 0 2 0                           |
    // |                                                                  |
    // |  4 | 10^00(1)e |   g    | Lower levels of 4um bands;             |
    // |  5 | 10^00(2)e |   g    | Lower levels of reg bands (1 & 2)      |
    // |  6 | 02^20(1)e |   g    |                                        |
    // |  7 | 02^20(1)f |   g    |                                        |
    // |------------------------------------------------------------------|
    // |                          1 1 0 + 0 3 0                           |
    // |                                                                  |
    // |  8 | 11^10(1)e |   u    | Lower levels of hot bands (8,9,10,11)  |
    // |  9 | 11^10(1)f |   u    |                                        |
    // | 10 | 11^10(2)e |   u    |                                        |
    // | 11 | 11^10(2)f |   u    |                                        |
    // | 12 | 03^30(1)e |   u    |                                        |
    // | 13 | 03^30(1)f |   u    |                                        |
    // |------------------------------------------------------------------|
    // |                              0 1 1                               |
    // |                                                                  |
    // | 14 | 01^11(1)e |   u    | Upper levels of hot bands              |
    // | 15 | 01^11(1)f |   u    |                                        |
    // |                              0 0 2                               |
    // |------------------------------------------------------------------|
    // |                          1 0 1 + 0 2 1                           |
    // |                                                                  |
    // | 16 | 10^01(1)e |   u    | Upper levels of 4um bands              |
    // | 17 | 10^01(2)e |   u    | Lower levels of seq bands (16 & 17)    |
    // | 18 | 02^21(1)e |   u    | Optical pumping @ 2.8 um               |
    // | 19 | 02^21(1)f |   u    |                                        |
    //  ------------------------------------------------------------------|
    // |                      2 0 1 + 1 2 1  + 0 4 1                      |
    // |                                                                  |
    // | 20 | 20^01(1)e |   u    | Optical pumping @ 2.8 um               |
    // | 21 | 20^01(2)e |   u    |                                        |
    // | 22 | 20^01(3)e |   u    |                                        |
    // | 23 | 12^21(1)e |   u    |                                        |
    // | 24 | 12^21(1)f |   u    |                                        |
    // | 25 | 12^21(2)e |   u    |                                        |
    // | 26 | 12^21(2)f |   u    |                                        |
    // | 27 | 04^41(1)e |   u    |                                        |
    // | 28 | 04^41(1)f |   u    |                                        |
    //  ------------------------------------------------------------------

    // Vibrational transitions ("bands" in HITRAN terminology)
    //  -----------------------------------------------------------
    // |band|   levels (HITRAN)    | levels (co2amp) | description |
    //  -----------------------------------------------------------
    // |  0 | 00^01(1) -> 10^00(1) |    1 -> 4       | reg 10 um   |
    // |  1 | 00^01(1) -> 10^00(2) |    1 -> 5       | reg  9 um   |
    // |  2 | 01^11(1) -> 11^10(1) |   14 -> 8  (e)  | hot 10 um   |
    // |    |                      |   15 -> 9  (f)  | "           |
    // |  3 | 01^11(1) -> 11^10(2) |   14 -> 10 (e)  | hot  9 um   |
    // |    |                      |   15 -> 11 (f)  | "           |
    // |  4 | 00^02(1) -> 10^01(1) |    2 -> 16      | seq 10 um   |
    // |  5 | 00^02(1) -> 10^01(2) |    2 -> 17      | seq  9 um   |
    // |  6 | 10^01(1) -> 10^00(1) |   16 -> 4       | 4um         |
    // |  7 | 10^01(2) -> 10^00(2) |   17 -> 5       | "           |
    // |  8 | 02^21(1) -> 02^20(1) |   18 -> 6  (e)  | "           |
    // |    |                      |   19 -> 7  (f)  | "           |
    //  -----------------------------------------------------------



    // Vibrational constants G, Hz
    // G[is][vl]
    // source: fit of HITRAN data
    double G_tmp[NumIso][NumVib] = {
    //vl: 0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17     18     19     20     21     22     23     24     25     26     27     28
        {0.000, 0.704, 1.401, 2.090, 0.416, 0.385, 0.400, 0.400, 0.623, 0.623, 0.579, 0.579, 0.601, 0.601, 0.901, 0.901, 1.114, 1.083, 1.097, 1.097, 1.529, 1.492, 1.455, 1.517, 1.517, 1.465, 1.465, 1.490, 1.490}, // 626
        {0.000, 0.699, 1.390, 0,     0.409, 0.377, 0.397, 0.397, 0.614, 0.614, 0.570, 0.570, 0.596, 0.596, 0.893, 0.893, 1.101, 1.070, 1.088, 1.088, 1.511, 1.469, 1.435, 1.502, 1.502, 1.449, 1.449, 0,     0    }, // 727
        {0.000, 0.694, 1.380, 0,     0.404, 0.369, 0.394, 0.394, 0.607, 0.607, 0.560, 0.560, 0.592, 0.592, 0.887, 0.887, 1.091, 1.057, 1.081, 1.081, 1.496, 1.449, 1.416, 1.488, 1.488, 1.434, 1.434, 1.468, 1.468}, // 828
        {0.000, 0.685, 1.362, 2.033, 0.411, 0.379, 0.389, 0.389, 0.611, 0.611, 0.569, 0.569, 0.584, 0.584, 0.875, 0.875, 1.089, 1.058, 1.066, 1.066, 1.496, 1.465, 1.423, 1.481, 1.481, 1.430, 1.430, 1.449, 1.449}, // 636
        {0.000, 0.679, 0,     0,     0.402, 0.373, 0.386, 0.386, 0,     0,     0,     0,     0,     0,     0.868, 0.868, 1.075, 1.046, 1.058, 1.058, 0,     0,     0,     0,     0,     0,     0,     0,     0    }, // 737
        {0.000, 0.674, 1.341, 0,     0.395, 0.366, 0.383, 0.383, 0.593, 0.593, 0.551, 0.551, 0.574, 0.574, 0.862, 0.862, 1.063, 1.034, 1.050, 1.050, 1.458, 1.421, 1.388, 1.449, 1.449, 1.400, 1.400, 0,     0    }, // 838
        {0.000, 0.702, 1.396, 0,     0.413, 0.381, 0.399, 0.399, 0.618, 0.618, 0.575, 0.575, 0.598, 0.598, 0.897, 0.897, 1.107, 1.077, 1.093, 1.093, 1.520, 1.481, 1.445, 1.510, 1.510, 1.457, 1.457, 1.484, 1.484}, // 627
        {0.000, 0.699, 1.391, 2.075, 0.409, 0.378, 0.397, 0.397, 0.614, 0.614, 0.570, 0.570, 0.596, 0.596, 0.894, 0.894, 1.102, 1.071, 1.089, 1.089, 1.512, 1.470, 1.436, 1.503, 1.503, 1.450, 1.450, 1.479, 1.479}, // 628
        {0.000, 0.696, 1.385, 0,     0.406, 0.373, 0.396, 0.396, 0.610, 0.610, 0.565, 0.565, 0.594, 0.594, 0.890, 0.890, 1.096, 1.063, 1.085, 1.085, 1.503, 1.459, 1.426, 1.495, 1.495, 1.441, 1.441, 1.473, 1.473}, // 728
        {0.000, 0.682, 1.357, 0,     0.406, 0.376, 0.387, 0.387, 0.606, 0.606, 0.564, 0.564, 0.581, 0.581, 0.872, 0.872, 1.082, 1.052, 1.062, 1.062, 1.486, 1.454, 1.415, 1.472, 1.472, 1.423, 1.423, 1.443, 1.443}, // 637
        {0.000, 0.679, 1.352, 0,     0.402, 0.373, 0.386, 0.386, 0.601, 0.601, 0.560, 0.560, 0.579, 0.579, 0.869, 0.869, 1.076, 1.046, 1.058, 1.058, 1.476, 1.443, 1.407, 1.465, 1.465, 1.415, 1.415, 1.437, 1.437}, // 638
        {0.000, 0.676, 0,     0,     0.399, 0.370, 0.384, 0.384, 0.597, 0.597, 0.556, 0.556, 0.576, 0.576, 0.865, 0.865, 1.069, 1.040, 1.054, 1.054, 1.467, 1.432, 1.397, 0,     0,     0,     0,     0,     0    }  // 738
    };

    // Rotational constants B, Hz
    // B[is][vl]
    // source: fit of HITRAN data
    double B_tmp[NumIso][NumVib] = {
    //vl: 0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17     18     19     20     21     22     23     24     25     26     27     28
        {1.170, 1.161, 1.151, 1.142, 1.170, 1.171, 1.174, 1.174, 1.170, 1.173, 1.171, 1.174, 1.176, 1.176, 1.162, 1.164, 1.160, 1.162, 1.165, 1.165, 1.162, 1.160, 1.164, 1.165, 1.165, 1.166, 1.166, 1.170, 1.170}, // 626
        {1.101, 1.092, 1.084, 0,     1.102, 1.104, 1.105, 1.105, 1.102, 1.105, 1.102, 1.104, 1.107, 1.107, 1.094, 1.095, 1.093, 1.092, 1.096, 1.097, 1.095, 1.091, 1.094, 1.097, 1.097, 1.098, 1.098, 0,     0    }, // 727
        {1.040, 1.032, 1.023, 0,     1.041, 1.042, 1.044, 1.044, 1.042, 1.044, 1.040, 1.042, 1.046, 1.046, 1.033, 1.034, 1.033, 1.031, 1.036, 1.036, 1.035, 1.031, 1.031, 1.036, 1.036, 1.036, 1.036, 1.040, 1.040}, // 828
        {1.170, 1.161, 1.152, 1.143, 1.168, 1.172, 1.174, 1.174, 1.169, 1.172, 1.172, 1.175, 1.176, 1.176, 1.162, 1.164, 1.159, 1.163, 1.165, 1.165, 1.159, 1.160, 1.166, 1.164, 1.164, 1.167, 1.167, 1.170, 1.170}, // 636
        {1.101, 1.093, 0,     0,     1.100, 1.102, 1.105, 1.105, 0,     0,     0,     0,     0,     0,     1.094, 1.095, 1.092, 1.094, 1.097, 1.097, 0,     0,     0,     0,     0,     0,     0,     0,     0    }, // 737
        {1.040, 1.032, 1.024, 0,     1.040, 1.042, 1.044, 1.044, 1.041, 1.043, 1.041, 1.043, 1.045, 1.045, 1.033, 1.035, 1.032, 1.032, 1.036, 1.036, 1.034, 1.031, 1.034, 1.036, 1.036, 1.036, 1.036, 0,     0    }, // 838
        {1.135, 1.126, 1.117, 0,     1.136, 1.138, 1.139, 1.139, 1.136, 1.139, 1.136, 1.139, 1.141, 1.141, 1.127, 1.129, 1.126, 1.127, 1.130, 1.131, 1.128, 1.124, 1.128, 1.131, 1.131, 1.131, 1.131, 1.135, 1.135}, // 627
        {1.104, 1.095, 1.086, 1.078, 1.105, 1.107, 1.108, 1.108, 1.105, 1.108, 1.105, 1.107, 1.110, 1.110, 1.096, 1.098, 1.096, 1.095, 1.099, 1.099, 1.098, 1.093, 1.096, 1.100, 1.100, 1.100, 1.100, 1.104, 1.104}, // 628
        {1.070, 1.062, 1.053, 0,     1.071, 1.073, 1.074, 1.074, 1.072, 1.074, 1.071, 1.073, 1.076, 1.076, 1.063, 1.064, 1.063, 1.061, 1.066, 1.066, 1.065, 1.061, 1.062, 1.066, 1.066, 1.067, 1.067, 1.070, 1.070}, // 728
        {1.135, 1.126, 1.118, 0,     1.134, 1.137, 1.139, 1.139, 1.135, 1.137, 1.137, 1.140, 1.141, 1.141, 1.128, 1.129, 1.125, 1.128, 1.131, 1.131, 1.126, 1.125, 1.131, 1.130, 1.130, 1.132, 1.132, 1.135, 1.135}, // 637
        {1.104, 1.095, 1.087, 0,     1.103, 1.105, 1.108, 1.108, 1.104, 1.107, 1.105, 1.108, 1.110, 1.110, 1.097, 1.098, 1.095, 1.097, 1.099, 1.099, 1.096, 1.094, 1.099, 1.099, 1.099, 1.102, 1.102, 1.103, 1.103}, // 638
        {1.070, 1.062, 0,     0,     1.071, 1.073, 1.070, 1.071, 1.074, 1.074, 1.071, 1.073, 1.071, 1.074, 1.063, 1.065, 1.062, 1.063, 1.066, 1.066, 1.063, 1.060, 1.065, 0,     0,     0,     0,     0,     0    }  // 738
    };

    for(int is=0; is<NumIso; ++is)
    {
        for(int vl=0; vl<NumVib; ++vl)
        {
            G[is][vl] = G_tmp[is][vl] * 1e14;
            B[is][vl] = B_tmp[is][vl] * 1e10;
        }
    }

    // masses of a CO2 molecules [kg]
    double amu = 1.66053906660e-27; //atomic mass unit [kg]
                         // 626 727 828 636 737 838 627 628 728 637 638 738
    double m_iso[NumIso] = {44, 46, 48, 45, 47, 49, 45, 46, 47, 46, 47, 48};
    for(int is=0; is<NumIso; ++is)
    {
        m_iso[is] *= amu;
    }


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
            int is; // isotopologue number in co2amp
            for (is=0; is<NumIso; ++is)
            {
                if (isot_map[is] == isot_id)
                {
                    break; // exit the loop if a match is found
                }
            }
            if(is == NumIso) // no match found => not one of the supported isotopologues
                continue;

            if(N_iso[is]==0.0) // skip zero-content isotopologues
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

            // skip if the band is excluded from calculations by user
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
                    j_lo[is].push_back(J);
                    j_up[is].push_back(J-1);
                    break;
                case 'R':
                    if(J>78)
                        continue;
                    j_lo[is].push_back(J);
                    j_up[is].push_back(J+1);
                    break;
                case 'Q':
                    continue; // ignore Q branch
                    break;
            }

            // Transition frequency (only add to array after checking that J's are within the intended range)
            v[is].push_back(v_Hz);

            // Associate vibrational levels with the transition
            switch(band)
            {
                case 0: // reg 10 um
                    vl_up[is].push_back(1);
                    vl_lo[is].push_back(4);
                    break;
                case 1: // reg 9 um
                    vl_up[is].push_back(1);
                    vl_lo[is].push_back(5);
                    break;
                case 2: // hot 10 um
                    if(ef == 'e')
                    {
                        vl_up[is].push_back(14);
                        vl_lo[is].push_back(8);
                    }
                    else //f
                    {
                        vl_up[is].push_back(15);
                        vl_lo[is].push_back(9);
                    }
                    break;
                case 3: // hot 9 um
                    if(ef == 'e')
                    {
                        vl_up[is].push_back(14);
                        vl_lo[is].push_back(10);
                    }
                    else //f
                    {
                        vl_up[is].push_back(15);
                        vl_lo[is].push_back(11);
                    }
                    break;
                case 4: // seq 10 um
                    vl_up[is].push_back(2);
                    vl_lo[is].push_back(16);
                    break;
                case 5: // seq 9 um
                    vl_up[is].push_back(2);
                    vl_lo[is].push_back(17);
                    break;
                case 6: // 4um-1
                    vl_up[is].push_back(16);
                    vl_lo[is].push_back(14);
                    break;
                case 7: // 4um-2
                    vl_up[is].push_back(17);
                    vl_lo[is].push_back(5);
                    break;
                case 8: // 4um-3
                    if(ef == 'e')
                    {
                        vl_up[is].push_back(18);
                        vl_lo[is].push_back(6);
                    }
                    else //f
                    {
                        vl_up[is].push_back(19);
                        vl_lo[is].push_back(7);
                    }
                    break;
            }

            // Transition cross-sections, m^2
            double A = std::stod(line.substr(25, 10)); // Einstein coefficient A (1/s)

            // Transition dipole dephasing time, s (real tau2 associated with collisons)
            double tau2_L = 1e-6 / (M_PI*7.61*750*(p_CO2+0.73*p_N2+0.64*p_He));

            // Lorentzian FWHM of the transition (homogineous collisional broadeneig) [Hz]
            double fwhm_L = 1 / (M_PI*tau2_L);

            // Doppler FWHM [Hz]
            double fwhm_D = v_Hz * sqrt(8 * kB * T0 * log(2) / (m_iso[is] * c*c));

            // Effective FWHM - Lorentzian+Doppler [Hz]
            double fwhm_eff = sqrt(fwhm_L*fwhm_L + fwhm_D*fwhm_D);

            // Effective tau2: not very accurate physically, but allows to account for
            // Doppler broadening without breaking internal consistency of the model
            double tau2_eff = 1 / (M_PI*fwhm_eff);

            // Wavelength, m
            double lambda = 1/(wn*100);

            // peak cross-section, m^2
            double sigma_eff = lambda*lambda * A / (4*M_PI*M_PI*fwhm_eff);

            sigma[is].push_back(sigma_eff);

            fwhm[is].push_back(fwhm_eff);

            tau2[is].push_back(tau2_eff);

            // Polarization dephasing factor (tau2): half-time-step
            // (same for all pulses)
            //dephase_exp[is].push_back( exp(-0.5*Dt / tau2_eff) );

            // Detuning phase factor (rho rotation): half-time-step
            // (may be different for different pulses if different vc)
            for(size_t pulse_n=0; pulse_n<pulses.size(); ++pulse_n)
            {
                std::complex<double> a = 1.0 / tau2_eff + I * (2.0*M_PI*(pulses[pulse_n]->vc - v_Hz));
                precalc_a[is].push_back( a );
                precalc_exp[is].push_back( exp(-a*Dt) );
            }
            precalc_b_part[is].push_back( - sigma_eff / (2*tau2_eff) ); // b = b_part * Dn * E_in

            //if(J==20)
            if(debug_level >= 3)
            {
                std::cout << "  "
                          << "Isot: " << isotopologue[is] << "; "
                          << "Band:" << Vup_id << " ->" << Vlo_id << " " << std::string(1,pqr) << std::to_string(J) << std::string(1,ef) << "; "
                          << "freq = " << std::to_string(v[is].back()/1e12) << " THz; "
                          << "A = " + std::to_string(A) + " 1/s; "
                          << "fwhm_L = " + std::to_string(fwhm_L/1e6) + " MHz; "
                          << "fwhm_D = " + std::to_string(fwhm_D/1e6) + " MHz; "
                          << "fwhm = " + std::to_string(fwhm_eff/1e6) + " MHz"
                          << std::endl;
            }
        }
        Debug(2, "Finished reading file");
    }


    // Fractional populations of rotational sublevels
    // f_rot[is][vl][j]
    for(int is=0; is<NumIso; ++is)
    {
        for(int vl=0; vl<NumVib; ++vl)
        {
            // Chsrcteristics of vibraitonal levels
            // vl:              0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17     18     19     20     21     22     23     24     25     26     27     28
            // HITRAN:         00001e 00011e 00021e 00031e 10001e 10002e 02201e 02201f 11101e 11101f 11102e 11102f 03301e 03301f 01111e 01111f 10011e 10012e 02211e 02211f 20011e 20012e 20013e 12211e 12211f 12212e 12212f 04411e 04411f
            int  l[]        = { 0,     0,     0,     0,     0,     0,     2,     2,     1,     1,     1,     1,     3,     3,     1,     1,     0,     0,     2,     2,     0,     0,     0,     2,     2,     2,     2,     4,     4    };
            char parity[]   = {'g',   'u',   'g',   'u',   'g',   'g',   'g',   'g',   'u',   'u',   'u',   'u',   'u',   'u',   'g',   'g',   'u',   'u',   'u',   'u',   'u',   'u',   'u',   'u',   'u',   'u',   'u',   'u',   'u'  };
            char symmetry[] = {'e',   'e',   'e',   'e',   'e',   'e',   'e',   'f',   'e',   'f',   'e',   'f',   'e',   'f',   'e',   'f',   'e',   'e',   'e',   'f',   'e',   'e',   'e',   'e',   'f',   'e',   'f',   'e',   'f'  };

            //general expression for energy distribution between rotational sub-levels
            for(int j=0; j<NumRot; ++j)
            {
                f_rot[is][vl][j] = h*B[is][vl]/(kB*T0) * (2*j+1) * exp(-h*B[is][vl]/(kB*T0)*j*(j+1));
            }

            // vibrational levels with J<l are not populated
            for(int j=0; j<l[vl]; ++j)
            {
                f_rot[is][vl][j] = 0;
            }

            if(is<6) // symmetric molecules (626, 727, 828, 636, 737, 838) have forbidden rottational sub-levels
            {
                if( (parity[vl]=='g' && symmetry[vl]=='e') || (parity[vl]=='u' && symmetry[vl]=='f') ) // only even J's populated
                {
                    for(int j=1; j<NumRot; j+=2)
                    {
                        f_rot[is][vl][j] = 0; // odd J's not populated
                    }
                } // only odd J's populated
                else
                {
                    for(int j=0; j<NumRot; j+=2)
                    {
                        f_rot[is][vl][j] = 0; // even J's not populated
                    }
                }

                // double sub-level's population to keep total polulation of vibrational level
                for(int j=0; j<NumRot; ++j)
                {
                    f_rot[is][vl][j] *= 2;
                }
            }
        }
    }
}
