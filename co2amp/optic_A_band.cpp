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
    // |                              0 0 1                               |
    // |                                                                  |
    // |  0 | 00^01(1)e |   u    | upper level of regular band            |
    // |------------------------------------------------------------------|
    // |                          1 0 0 + 0 2 0                           |
    // |                                                                  |
    // |  1 | 10^00(1)e |   g    | 1,2:     lower levels of reg bands     |
    // |  2 | 10^00(2)e |   g    | 1,2,3,4: lower levels of 4um bands     |
    // |  3 | 02^20(1)e |   g    |                                        |
    // |  4 | 02^20(1)f |   g    | 1 & 2: mixed [10^00, 02^00] levels     |
    // |------------------------------------------------------------------|
    // |                              0 1 1                               |
    // |                                                                  |
    // |  5 | 01^11(1)e |   u    | upper levels of hot bands              |
    // |  6 | 01^11(1)f |   u    |                                        |
    // |------------------------------------------------------------------|
    // |                          1 1 0 + 0 3 0                           |
    // |                                                                  |
    // |  7 | 11^10(1)e |   u    | 7,8,9,10: lower levels of hot bands    |
    // |  8 | 11^10(1)f |   u    | 11,12: not currently included in       |
    // |  9 | 11^10(2)e |   u    |        the amplification model         |
    // | 10 | 11^10(2)f |   u    |                                        |
    // | 11 | 03^30(1)e |   u    | 7 & 9:  mixed [11^10e, 03^10e] levels  |
    // | 12 | 03^30(1)f |   u    | 8 & 10: mixed [11^10f, 03^10f] levels  |
    // |------------------------------------------------------------------|
    // |                              0 0 2                               |
    // |                                                                  |
    // | 13 | 00^02(1)e |   g    | upper level of sequence band           |
    // |------------------------------------------------------------------|
    // |                          1 0 1 + 0 2 1                           |
    // |                                                                  |
    // | 14 | 10^01(1)e |   u    | 14,15:       lower levels of seq bands |
    // | 15 | 10^01(2)e |   u    | 14,15,16,17: upper levels of 4um bands |
    // | 16 | 02^21(1)e |   u    |                                        |
    // | 17 | 02^21(1)f |   u    | 14 & 15: mixed [10^01, 02^01] levels   |
    //  ------------------------------------------------------------------

    // Vibrational transitions ("bands" in HITRAN terminology)
    //  -----------------------------------------------------------
    // |band|   levels (HITRAN)    | levels (co2amp) | description |
    //  -----------------------------------------------------------
    // |  0 | 00^01(1) -> 10^00(1) |    0 -> 1       | reg 10 um   |
    // |  1 | 00^01(1) -> 10^00(2) |    0 -> 2       | reg  9 um   |
    // |  2 | 01^11(1) -> 11^10(1) |    5 -> 7  (e)  | hot 10 um   |
    // |    |                      |    6 -> 8  (f)  | "           |
    // |  3 | 01^11(1) -> 11^10(2) |    5 -> 9  (e)  | hot  9 um   |
    // |    |                      |    6 -> 10 (f)  | "           |
    // |  4 | 00^02(1) -> 10^01(1) |   13 -> 14      | seq 10 um   |
    // |  5 | 00^02(1) -> 10^01(2) |   13 -> 15      | seq  9 um   |
    // |  6 | 10^01(1) -> 10^00(1) |   14 -> 1       | 4um         |
    // |  7 | 10^01(2) -> 10^00(2) |   15 -> 2       | "           |
    // |  8 | 02^21(1) -> 02^20(1) |   16 -> 3  (e)  | "           |
    // |    |                      |   17 -> 4  (f)  | "           |
    //  -----------------------------------------------------------

    double tau2 = 1e-6 / (M_PI*7.61*750*(p_CO2+0.73*p_N2+0.64*p_He));  // transition dipole dephasing time, s
    double gamma = 1 / tau2;   // Lorentzian HWHM


    // Rotational constants B, Hz
    // B[is][vl]
    // source: fit of HITRAN data
    double B_tmp[NumIso][NumVib] = {
    // vl:  0         1         2         3         4         5         6         7         8         9         10        11        12        13        14        15        16        17
        {1.16E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.18E+10, 1.18E+10, 1.15E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.17E+10}, //626
        {1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.09E+10, 1.09E+10, 1.09E+10, 1.10E+10, 1.10E+10}, //727
        {1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.05E+10, 1.05E+10, 1.02E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10}, //828
        {1.16E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.17E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.17E+10, 1.18E+10, 1.17E+10, 1.18E+10, 1.18E+10, 1.15E+10, 1.16E+10, 1.16E+10, 1.17E+10, 1.17E+10}, //636
        {1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.09E+10, 1.10E+10,        0,        0,        0,        0,        0,        0,        0, 1.09E+10, 1.09E+10, 1.10E+10, 1.10E+10}, //737
        {1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.04E+10, 1.05E+10, 1.05E+10, 1.02E+10, 1.03E+10, 1.03E+10, 1.04E+10, 1.04E+10}, //838
        {1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.13E+10, 1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.12E+10, 1.13E+10, 1.13E+10, 1.13E+10, 1.13E+10}, //627
        {1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.10E+10}, //628
        {1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.08E+10, 1.08E+10, 1.05E+10, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10}, //728
        {1.13E+10, 1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.13E+10, 1.13E+10, 1.13E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.14E+10, 1.12E+10, 1.13E+10, 1.13E+10, 1.13E+10, 1.13E+10}, //637
        {1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.10E+10, 1.10E+10, 1.10E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.11E+10, 1.09E+10, 1.09E+10, 1.10E+10, 1.10E+10, 1.10E+10}, //638
        {1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.07E+10, 1.08E+10, 1.08E+10,        0, 1.06E+10, 1.06E+10, 1.07E+10, 1.07E+10}  //738
    };

    // Vibrational constants G, Hz
    // G[is][vl]
    // source: fit of HITRAN data
    double G_tmp[NumIso][NumVib] = {
        // vl:  0            1             2             3             4             5             6             7             8             9             10           11            12            13            14            15            16            17
        {7.042553e+13, 4.161676e+13, 3.853553e+13, 4.002624e+13, 4.002624e+13, 9.005802e+13, 9.005802e+13, 6.226257e+13, 6.226257e+13, 5.793400e+13, 5.793400e+13, 6.005580e+13, 6.005581e+13, 1.401028e+14, 1.113664e+14, 1.083102e+14, 1.097022e+14, 1.097022e+14}, //626
        {6.986942e+13, 4.091988e+13, 3.772097e+13, 3.970844e+13, 3.970844e+13, 8.934676e+13, 8.934676e+13, 6.140119e+13, 6.140119e+13, 5.696820e+13, 5.696820e+13, 5.958039e+13, 5.958039e+13, 1.389995e+14, 1.101047e+14, 1.069801e+14, 1.088366e+14, 1.088366e+14}, //727
        {6.937342e+13, 4.038480e+13, 3.688419e+13, 3.942513e+13, 3.942513e+13, 8.871241e+13, 8.871241e+13, 6.069384e+13, 6.069384e+13, 5.602234e+13, 5.602234e+13, 5.915649e+13, 5.915649e+13, 1.380154e+14, 1.090664e+14, 1.056829e+14, 1.080647e+14, 1.080647e+14}, //828
        {6.845722e+13, 4.107342e+13, 3.794852e+13, 3.889102e+13, 3.889102e+13, 8.754654e+13, 8.754654e+13, 6.107053e+13, 6.107053e+13, 5.685676e+13, 5.685676e+13, 5.835015e+13, 5.835015e+13, 1.362121e+14, 1.089119e+14, 1.057589e+14, 1.066456e+14, 1.066456e+14}, //636
        {6.788379e+13, 4.020974e+13, 3.729090e+13, 3.856347e+13, 3.856347e+13, 8.681306e+13, 8.681307e+13,            0,            0,            0,            0,            0,            0,            0, 1.074725e+14, 1.045617e+14, 1.057530e+14, 1.057530e+14}, //737
        {6.737208e+13, 3.953352e+13, 3.658622e+13, 3.827120e+13, 3.827120e+13, 8.615853e+13, 8.615853e+13, 5.926637e+13, 5.926637e+13, 5.514265e+13, 5.514265e+13, 5.742282e+13, 5.742282e+13, 1.340587e+14, 1.062772e+14, 1.033808e+14, 1.049562e+14, 1.049562e+14}, //838
        {7.015184e+13, 4.125222e+13, 3.814223e+13, 3.986768e+13, 3.986768e+13, 8.970696e+13, 8.970696e+13, 6.182016e+13, 6.182016e+13, 5.746100e+13, 5.746100e+13, 5.981858e+13, 3.986768e+13, 1.395595e+14, 1.107240e+14, 1.076636e+14, 1.092742e+14, 1.092742e+14}, //627
        {6.991497e+13, 4.094694e+13, 3.775661e+13, 3.972688e+13, 3.972688e+13, 8.940144e+13, 8.940144e+13, 6.143764e+13, 6.143764e+13, 5.701264e+13, 5.701264e+13, 5.960795e+13, 5.960795e+13, 1.390888e+14, 1.101777e+14, 1.070601e+14, 1.089005e+14, 1.089005e+14}, //628
        {6.962484e+13, 4.064148e+13, 3.731199e+13, 3.956702e+13, 3.956702e+13, 8.903315e+13, 8.903315e+13, 6.103891e+13, 6.103890e+13, 5.650260e+13, 5.650260e+13, 5.936887e+13, 5.936887e+13, 1.385140e+14, 1.095778e+14, 1.063448e+14, 1.084544e+14, 1.084544e+14}, //728
        {6.817541e+13, 4.062495e+13, 3.763366e+13, 3.872764e+13, 3.872764e+13, 8.718494e+13, 8.718494e+13, 6.056383e+13, 6.056383e+13, 5.643763e+13, 5.643763e+13, 5.810565e+13, 5.810565e+13, 1.356526e+14, 1.081816e+14, 1.051784e+14, 1.062047e+14, 1.062047e+14}, //637
        {6.793212e+13, 4.024045e+13, 3.732115e+13, 3.858248e+13, 3.858248e+13, 8.687084e+13, 8.687084e+13, 6.012168e+13, 6.012168e+13, 5.603843e+13, 5.603843e+13, 5.788854e+13, 5.788854e+13, 1.351688e+14, 1.075520e+14, 1.046394e+14, 1.058201e+14, 1.058201e+14}, //638
        {6.763179e+13, 3.985595e+13, 3.695121e+13, 3.841755e+13, 3.841755e+13, 8.648984e+13, 8.648984e+13, 5.966249e+13, 5.966249e+13, 5.557934e+13, 5.557934e+13, 5.764160e+13, 5.764160e+13,            0, 1.068645e+14, 1.039876e+14, 1.053588e+14, 1.053588e+14}  //738
    };

    for(int is=0; is<NumIso; ++is)
    {
        for(int vl=0; vl<NumVib; ++vl)
        {
            B[is][vl] = B_tmp[is][vl];
            G[is][vl] = G_tmp[is][vl];
        }
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
                    /*if(J>79)
                        continue;
                    j_lo[is].push_back(J);
                    j_up[is].push_back(J);*/
                    break;
            }

            // Transition frequency (only add to array after checking that J's are within the intended range)
            v[is].push_back(v_Hz);

            // Associate vibrational levels with the transition
            switch(band)
            {
                case 0: // reg 10 um
                    vl_up[is].push_back(0);
                    vl_lo[is].push_back(1);
                    break;
                case 1: // reg 9 um
                    vl_up[is].push_back(0);
                    vl_lo[is].push_back(2);
                    break;
                case 2: // hot 10 um
                    if(ef == 'e')
                    {
                        vl_up[is].push_back(5);
                        vl_lo[is].push_back(7);
                    }
                    else //f
                    {
                        vl_up[is].push_back(6);
                        vl_lo[is].push_back(8);
                    }
                    break;
                case 3: // hot 9 um
                    if(ef == 'e')
                    {
                        vl_up[is].push_back(5);
                        vl_lo[is].push_back(9);
                    }
                    else //f
                    {
                        vl_up[is].push_back(6);
                        vl_lo[is].push_back(10);
                    }
                    break;
                case 4: // seq 10 um
                    vl_up[is].push_back(13);
                    vl_lo[is].push_back(14);
                    break;
                case 5: // seq 9 um
                    vl_up[is].push_back(13);
                    vl_lo[is].push_back(15);
                    break;
                case 6: // 4um-1
                    vl_up[is].push_back(14);
                    vl_lo[is].push_back(1);
                    break;
                case 7: // 4um-2
                    vl_up[is].push_back(15);
                    vl_lo[is].push_back(2);
                    break;
                case 8: // 4um-3
                    if(ef == 'e')
                    {
                        vl_up[is].push_back(16);
                        vl_lo[is].push_back(3);
                    }
                    else //f
                    {
                        vl_up[is].push_back(17);
                        vl_lo[is].push_back(4);
                    }
                    break;
            }

            // Transition cross-sections, m^2
            double A = std::stod(line.substr(25, 10)); // Einstein coefficient A (1/s)

            sigma[is].push_back( pow(1/(wn*100),2) * A / 4 / (M_PI*gamma) ); // wn*100: 1/cm -> 1/m

            //if(J==20)
            if(debug_level >= 3)
            {
                std::cout << "  "
                          << "Isot: " << isotopologue[is] << "; "
                          << "Band:" << Vup_id << " ->" << Vlo_id << " " << std::string(1,pqr) << std::to_string(J) << std::string(1,ef) << "; "
                          << "freq = " << std::to_string(v[is].back()/1e12) << " THz; "
                          << "A = " + std::to_string(A) + " 1/s"
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
            //              vl: 0      1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17
            int l[]         = { 0,     0,     0,     2,     2,     1,     1,     1,     1,     1,    1,     3,     3,     0,     0,     0,     2,     2    };
            char parity[]   = {'u',   'g',   'g',   'g',   'g',   'g',   'g',   'u',   'u',   'u',   'u',   'u',   'u',   'g',   'u',   'u',   'u',   'u'  };
            char symmetry[] = {'e',   'e',   'e',   'e',   'f',   'e',   'f',   'e',   'e',   'f',   'f',   'e',   'f',   'e',   'e',   'e',   'e',   'f'  };

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
