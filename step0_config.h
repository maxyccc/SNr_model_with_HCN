// step0_config.h
#ifndef STEP0_CONFIG_H
#define STEP0_CONFIG_H

#include <math.h>

// basic
# define second 1e3;
#define CONFIG_dt 0.025  // ms
#define CONFIG_1ms_step_num ((int)ceil(1.0 / CONFIG_dt))
const int CONFIG_spikes_init_size = 1e4;

// Path
# define SAVE_DIR "C:/Users/maxyc/CLionProjects/SNr_model_with_HCN/intermediate_data/"
# define RESULT_DIR "C:/Users/maxyc/CLionProjects/SNr_model_with_HCN/result/"

// setup
const int NUM_conductance = 32;
const int NUM_current = 300;
const int DURATION_init = 1000;  // ms
const int DURATION_test = 1000;  // ms



#endif