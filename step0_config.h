// step0_config.h
#ifndef STEP0_CONFIG_H
#define STEP0_CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// basic
# define second 1e3;
#define CONFIG_dt 0.025  // ms
#define CONFIG_1ms_step_num ((int)ceil(1.0 / CONFIG_dt))
const int CONFIG_spikes_init_size = 1e4;

// Path
# define SAVE_DIR "C:/Users/maxyc/CLionProjects/SNr_model_with_HCN/intermediate_result/"
# define RESULT_DIR "C:/Users/maxyc/CLionProjects/SNr_model_with_HCN/simulation_result/"


// step 1 grid search hyperparameter
const int PREPARE_DURATION_init = 500;  // ms
const int PREPARE_DURATION_test = 1000;  // ms

const int NUM_conductance = 33;
const float START_conductance = -4;  // g_HCN min = 2^START_conductance
const float END_conductance = +4;  // g_HCN max = 2^END_conductance

const int NUM_current = 33;
const float START_current = -80;
const float END_current = +0;


// step 3 simulation
const int NUM_samples = 100;  // default trial number in each raster
const int SIM_DURATION_total = 2000;  // ms
const double DEFAULT_g_HCN = 1;
const double DEFAULT_I_app = -50;


static inline double* linspace(double start, double end, int n) {
    if (n <= 0) return NULL;
    double* array = (double*)malloc(n * sizeof(double));
    if (!array) return NULL;

    double step = (n > 1) ? (end - start) / (n - 1) : 0;
    for (int i = 0; i < n; i++) {
        array[i] = start + i * step;
    }
    return array;
}

static inline double* exp2space(double start, double end, int n) {
    if (n <= 0) return NULL;
    double* array = (double*)malloc(n * sizeof(double));
    if (!array) return NULL;

    double step = (n > 1) ? (end - start) / (n - 1) : 0;
    for (int i = 0; i < n; i++) {
        array[i] = pow(2, start + i * step);
    }
    return array;
}


#endif