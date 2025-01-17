#include "diffusibles.h"
#include <iostream>
#include <omp.h>
#include <vector>

Diffusibles::Diffusibles(double DX, double Dconst, double KIL4Tumor, double KIL4M2, double KIFNG, double KM1F) {
    dx = DX;
    D = Dconst;
    kIL4Tumor = KIL4Tumor;
    kIL4M2 = KIL4M2;
    kIFNG = KIFNG;
    kM1f = KM1F;

    for(int i=0; i<100; ++i){
        for(int j=0; j<100; ++j){
            M1f[i][j] = 0;
            IL4[i][j] = 0;
            IFNG[i][j] = 0;
        }
    }
}

void Diffusibles::diffusion(CellGrids cg, double tstep) {
// diffusion via finite difference
    // if the max change in concentration is
    // less than 0.0001, we assume that an
    // equilibrium has been reached

    double dt = 0.2 * dx * dx / D;
    double t = 60 * 60 * tstep / dt;

    double maxDif = 0;
    double c = 0;

    int i, j;
    omp_set_num_threads(2);
    for (int q = 0; q < t; q++) {
        maxDif = 0;
#pragma omp parallel for
        for (i = 1; i < 99; i++) {
            for (j = 1; j < 99; j++) {
                double IL4_0 = IL4[i][j];
                double M1f_0 = M1f[i][j];
                double IFNG_0 = IFNG[i][j];

                IL4[i][j] = IL4[i][j] + dt*(kIL4Tumor*cg.ccg[i][j] + kIL4M2*cg.m2g[i][j])
                            + (dt * D / (dx * dx)) * (IL4[i + 1][j] + IL4[i - 1][j]
                                                      + IL4[i][j + 1] + IL4[i][j - 1]
                                                      - 4 * IL4[i][j]);

                M1f[i][j] = M1f[i][j] + dt * (kM1f*cg.ccg[i][j])
                            + (dt * D / (dx * dx)) * (M1f[i + 1][j] + M1f[i - 1][j]
                                                      + M1f[i][j + 1] + M1f[i][j - 1]
                                                      - 4 * M1f[i][j]);

                IFNG[i][j] = IFNG[i][j] + dt*(kIFNG*cg.actT[i][j])
                             + (dt*D/(dx*dx))*(IFNG[i+1][j] + IFNG[i-1][j]
                                               + IFNG[i][j+1] + IFNG[i][j-1]
                                               - 4*IFNG[i][j]);

                if(IL4_0 > 0){
                    c = (IL4[i][j] - IL4_0)/IL4_0;
                    if(c > maxDif){maxDif=c;}
                }
                if(M1f_0 > 0){
                    c = (M1f[i][j] - M1f_0)/M1f_0;
                    if(c > maxDif){maxDif=c;}
                }
                if(IFNG_0 > 0){
                    c = (IFNG[i][j] - IFNG_0)/IFNG_0;
                    if(c > maxDif){maxDif=c;}
                }

                if(IL4[i][j]<0 || M1f[i][j]<0 || IFNG[i][j] < 0){
                    std::cout << "IL4 " << IL4[i][j] << std::endl;
                    std::cout << "M1f " << M1f[i][j] << std::endl;
                    std::cout << "IFNG " << IFNG[i][j] << std::endl;
                    throw std::runtime_error("diffusibles::77");
                }
            }
        }
        if(maxDif < 0.0001 && q > 5){break;}
    }

}
