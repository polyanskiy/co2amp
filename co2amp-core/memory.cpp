#include  "co2amp.h"


void AllocateMemory()
{
    /*int x, k, pulse;

    if(!strcmp(pumping, "discharge")){
        discharge = malloc(sizeof(double*)*3);
        discharge[0] = malloc(sizeof(double)*n_discharge_points);
        discharge[1] = malloc(sizeof(double)*n_discharge_points);
        discharge[2] = malloc(sizeof(double)*n_discharge_points);
    }

    component_id = malloc(sizeof(char*) * n_components);
    component_type = malloc(sizeof(char*) * n_components);
    component_param1 = malloc(sizeof(char*) * n_components);
    component_param2 = malloc(sizeof(char*) * n_components);
    for(k=0; k<n_components; k++){
        component_id[k] = malloc(sizeof(char)*64); // max 64 symbols
        component_type[k] = malloc(sizeof(char)*64);
        component_param1[k] = malloc(sizeof(char)*64);
        component_param2[k] = malloc(sizeof(char)*64);
    }
    component_Dr = malloc(sizeof(double)*n_components);
    layout_distance = malloc(sizeof(double)*n_propagations);
    layout_component = malloc(sizeof(int)*n_propagations);
    layout_time = malloc(sizeof(double)*n_propagations);

    gainSpectrum = malloc(sizeof(double) * n0);

    if(n_amsections>0 && p_CO2+p_N2+p_He>0){
        e2 = malloc(sizeof(double*) * n_amsections);
        e3 = malloc(sizeof(double*) * n_amsections);
        e4 = malloc(sizeof(double*) * n_amsections);
        T = malloc(sizeof(double*) * n_amsections);
        for(k=0; k<n_amsections; k++){
            e2[k] = malloc(sizeof(double)*x0);
            e3[k] = malloc(sizeof(double)*x0);
            e4[k] = malloc(sizeof(double)*x0);
            T[k] = malloc(sizeof(double)*x0);
        }
    }

    E = malloc(sizeof(double complex**)*n_pulses);
    for(pulse=0; pulse<n_pulses; pulse++){
        E[pulse] = malloc(sizeof(double complex*)*x0);
        for(x=0; x<x0; x++)
            E[pulse][x] = malloc(sizeof(double complex)*n0);
    }*/

    // temporary - for nonlinear absorption in Ge
    /*alpha = malloc(sizeof(double)*x0);
    for(x=0; x<x0; x++)
        alpha[x] = 2; // 1/m (=0.02 1/cm)*/
}


void FreeMemory()
{
    /*int x, k, pulse;

    if(!strcmp(pumping, "discharge")){
        free(discharge[0]);
        free(discharge[1]);
        free(discharge[2]);
        free(discharge);
    }

    for(k=0; k<n_components; k++){
        free(component_id[k]);
        free(component_type[k]);
        free(component_param1[k]);
        free(component_param2[k]);
    }
    free(component_id);
    free(component_type);
    free(component_param1);
    free(component_param2);
    free(component_Dr);
    free(layout_distance);
    free(layout_component);
    free(layout_time);

    free(gainSpectrum);

    if(n_amsections>0 && p_CO2+p_N2+p_He>0)
    {
        for(k=0; k<n_amsections; k++){
            //free(e1[k]);
            free(e2[k]);
            free(e3[k]);
            free(e4[k]);
            free(T[k]);
        }
        //free(e1);
        free(e2);
        free(e3);
        free(e4);
        free(T);
    }

    for(pulse=0; pulse<=n_pulses-1; pulse++){
        for(x=0; x<x0; x++)
            free(E[pulse][x]);
        free(E[pulse]);
    }
    free(E);*/

    /*free(alpha);*/  // temporary - for nonlinear absorption in Ge
}


void AllocateMemoryBoltzmann(void)
{
    unsigned int j;

    //u = malloc(sizeof(double) * b0);
    u = new double[b0];


    //Q1 = malloc(sizeof(double*) * 11);
    Q1 = new double*[11];
    for(j=0; j<11; j++){
        //Q1[j] = malloc(sizeof(double) * b0);
        Q1[j] = new double[b0];
    }

    //Q2 = malloc(sizeof(double*) * 16);
    Q2 = new double*[16];
    for(j=0; j<16; j++)
        //Q2[j] =malloc(sizeof(double) * b0);
        Q2[j] = new double[b0];

    //Qm1 = malloc(sizeof(double) * b0);
    //Qm2 = malloc(sizeof(double) * b0);
    //Qm3 = malloc(sizeof(double) * b0);
    Qm1 = new double[b0];
    Qm2 = new double[b0];
    Qm3 = new double[b0];

    //Q = malloc(sizeof(double) * b0);
    Q = new double[b0];

    //M =  malloc(sizeof(double*) * b0);
    M = new double*[b0];
    for(j=0; j<b0; j++)
        //M[j] =  malloc(sizeof(double) * b0);
        M[j] = new double[b0];

    //f = malloc(sizeof(double) * b0);
    f = new double[b0];
}


void FreeMemoryBoltzmann(void)
{
    unsigned int j;

    //free(u);
    delete u;

    for(j=0; j<11; j++)
        //free(Q1[j]);
        delete Q1[j];
    //free(Q1);
    delete Q1;

    for(j=0; j<16; j++)
        //free(Q2[j]);
        delete Q2[j];
    //free(Q2);
    delete Q2;

    //free(Qm1);
    //free(Qm2);
    //free(Qm3);
    delete Qm1;
    delete Qm2;
    delete Qm3;

    //free(Q);
    delete Q;

    for(j=0; j<b0; j++)
        //free(M[j]);
        delete M[j];
    //free(M);
    delete M;

    //free(f);
    delete f;
}
