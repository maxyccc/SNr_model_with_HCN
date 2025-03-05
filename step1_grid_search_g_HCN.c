#include "bio_data/SNrModel.h"
#include "step0_config.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double *spike_times;
    int num_spikes;
} Spikes;

// double uniform_rng(double min, double max)
// {
//     double range = (max - min);
//     double div = RAND_MAX / range;
//     return min + (rand() / div);
// }

Spikes simple_simulation(State *restrict s, int duration) {
    Spikes spikes;
    spikes.num_spikes = 0;
    spikes.spike_times = (double *)malloc(CONFIG_spikes_init_size * sizeof(double));
    for (int i = 0; i < duration*CONFIG_1ms_step_num; i++) {
        if (f(s, CONFIG_dt)) {
            spikes.spike_times[spikes.num_spikes] = s->time;
            spikes.num_spikes++;
        }
        // printf("%f, %f, %f\n", s->time, s->V_d, s->V_s);  // For debug
    }
    return spikes;
}

// return firing rate in Hz, 0 if less than 1Hz
double calculate_firing_rate(State *restrict s) {
    Spikes spikes = simple_simulation(s, PREPARE_DURATION_init + PREPARE_DURATION_test);
    if (spikes.num_spikes == 0) {
        free(spikes.spike_times);
        return 0.0;
    }
    int first_spike_id = spikes.num_spikes - 1;
    for (int i = 0; i < spikes.num_spikes; i++) {
        if (spikes.spike_times[i] >= PREPARE_DURATION_init) {
            first_spike_id = i + 0;
            break;
        }
    }
    if (first_spike_id >= spikes.num_spikes - 1) {
        free(spikes.spike_times);
        return 1.0;
    }
//    double first_spike = spikes.spike_times[first_spike_id];
//    double last_spike = spikes.spike_times[spikes.num_spikes - 1];
    double firing_rate = 1e3 * (spikes.num_spikes - first_spike_id)/PREPARE_DURATION_test;
    free(spikes.spike_times);
    return firing_rate;
}

// Previous task 4 in reference repository
void setup() {
    // ###################################################################
    // ############ TO Change: Search grid of g_HCN x I_app ##############
    // ############            see step0_config.h           ##############
    // ###################################################################
    const double* g = exp2space(START_conductance, END_conductance, NUM_conductance);
    const double* I = linspace(START_current, END_current, NUM_current);
    double r_0[NUM_current], r_som[NUM_conductance][NUM_current], r_den[NUM_conductance][NUM_current];

    // calculate firing rate r_0, r_som, r_den
    printf("Computing r_0 ... \n");
    for (int j=0; j < NUM_current; j++) {
        State s = init_state();
        s.I_app = I[j];
        r_0[j] = calculate_firing_rate(&s);
        printf("r_0[%d]: I_app %f, firerate %f\n", j, I[j],  r_0[j]);
    }
    printf("Computing r_som ... \n");
    for (int i = 0; i < NUM_conductance; i++) {
        for (int j = 0; j < NUM_current; j++) {
            State s = init_state();
            s.I_app = I[j];
            s.g_HCN_som = g[i];
            r_som[i][j] = calculate_firing_rate(&s);
            printf("r_som[%d][%d]: I_app %f, g_HCN_som %f, firerate %f\n", i, j, I[j], g[i], r_som[i][j]);
        }
    }
    printf("Computing r_den ... \n");
    for (int i = 0; i < NUM_conductance; i++) {
        for (int j = 0; j < NUM_current; j++) {
            State s = init_state();
            s.I_app = I[j];
            s.g_HCN_den = g[i];
            r_den[i][j] = calculate_firing_rate(&s);
            printf("r_den[%d][%d]: I_app %f, g_HCN_den %f, firerate %f\n", i, j, I[j], g[i], r_den[i][j]);
        }
    }

    // save result
    char file[128];
    printf("Saving intermediate data at: %s \n", SAVE_DIR);

    strcpy(file, SAVE_DIR "prepared_g.bin");
    write_binary_file(file, g, NUM_conductance);

    strcpy(file, SAVE_DIR "prepared_I.bin");
    write_binary_file(file, I, NUM_current);

    strcpy(file, SAVE_DIR "prepared_r_0.bin");
    write_binary_file(file, r_0, NUM_current);

    strcpy(file, SAVE_DIR "prepared_r_som.bin");
    write_binary_file(file, NULL, 0);
    for (int i = 0; i < NUM_conductance; i++) {
        append_binary_file(file, r_som[i], NUM_current);
    }

    strcpy(file, SAVE_DIR "prepared_r_den.bin");
    write_binary_file(file, NULL, 0);
    for (int i = 0; i < NUM_conductance; i++) {
        append_binary_file(file, r_den[i], NUM_current);
    }
}

int main() {
    struct timeval start_time, stop_time, elapsed_time;
    gettimeofday(&start_time, NULL);

    printf("Step1 grid search g_HCN begins \n");
    setup();
    printf("Step1 grid search g_HCN finishes \n");

    gettimeofday(&stop_time, NULL);
    timersub(&stop_time, &start_time, &elapsed_time);
    printf("Running time: %f seconds.\n", elapsed_time.tv_sec + elapsed_time.tv_usec * 1e-6);
    return 0;
}