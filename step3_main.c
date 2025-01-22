#include "step0_snr_model.h"
#include "step0_config.h"
#include <stdio.h>

typedef struct {
    double *spike_times;
    int num_spikes;
} Spikes;


Spikes full_simulation(State *restrict s, int duration, double GPe_stim_time, double Str_stim_time) {
    Spikes spikes;
    spikes.num_spikes = 0;
    spikes.spike_times = (double *)malloc(CONFIG_spikes_init_size * sizeof(double));
    for (int i = 0; i < duration*CONFIG_1ms_step_num; i++) {
        if (i<=GPe_stim_time*CONFIG_1ms_step_num && (i+1)>GPe_stim_time*CONFIG_1ms_step_num) {
            s->GPe_stim = 1;
        } else {
            s->GPe_stim = 0;
        }
        if (i<=Str_stim_time*CONFIG_1ms_step_num && (i+1)>Str_stim_time*CONFIG_1ms_step_num) {
            s->Str_stim = 1;
        } else {
            s->Str_stim = 0;
        }
        if (f(s, CONFIG_dt)) {
            spikes.spike_times[spikes.num_spikes] = s->time;
            spikes.num_spikes++;
        }
    }
    return spikes;
}


int batch_simulation(double W_GPe, double W_Str, double tau, const char* HCN,
    double GPe_stim, double Str_stim, const char* task_id) {
    // load conductances
    char filename[512];
    if (strcmp(HCN, "som") == 0) {
        strcpy(filename, SAVE_DIR "g_HCN_som.bin");
    } else if (strcmp(HCN, "den") == 0) {
        strcpy(filename, SAVE_DIR "g_HCN_den.bin");
    } else {
        strcpy(filename, SAVE_DIR "g_HCN_zero.bin");
    }
    double g_HCN[NUM_current];
    size_t N0;
    printf("%s \n", filename);
    read_binary_file(filename, g_HCN, &N0);

    // load current
    strcpy(filename, SAVE_DIR "task4_I.bin");
    double I[NUM_current];
    size_t N1;
    printf("%s \n", filename);
    read_binary_file(filename, I, &N1);

    // simulate for all possible conductances
    strcpy(filename, SAVE_DIR);
    strcat(filename, task_id);
    strcat(filename, ".csv");
    FILE *result = fopen(filename, "w");
    printf("Result writing in %s \n", filename);
    if (result == NULL) {
        perror("Failed to open file");
        return 1;  // Or handle the error as needed
    }
    for (int j = 0; j < NUM_current; j++) {
        State s = init_state();
        s.W_GPe = W_GPe;
        s.tau_GABA_som = tau;
        s.W_Str = W_Str;
        s.tau_GABA_den = tau;
        s.I_app = I[j];
        if (strcmp(HCN, "som") == 0) {
            s.g_HCN_som = g_HCN[j];
        } else if (strcmp(HCN, "den") == 0) {
            s.g_HCN_den = g_HCN[j];
        }
        Spikes spikes = full_simulation(&s, DURATION_init + DURATION_test, GPe_stim, Str_stim);
        printf("#%d: I_app: %f, g_HCN_%s: %f, %d spikes \n", j, I[j], HCN, g_HCN[j], spikes.num_spikes);
        fprintf(result, "%d,", spikes.num_spikes);
        for (int i = 0; i < spikes.num_spikes; i++) {
            fprintf(result, "%f,", spikes.spike_times[i]);
        }
        free(spikes.spike_times);
        fprintf(result, "END\n");
    }
    fclose(result);
    printf("Result saved in %s \n", filename);
    return 0;
}

int main(int argc, char *argv[]) {
    struct timeval start_time, stop_time, elapsed_time;
    gettimeofday(&start_time, NULL);

    char HCN_choice[8] = "zero", task_id[128] = "happybird";
    double W_GPe = 0, W_Str = 0, tau = 0;
    double GPe_stim = 1000, Str_stim = 1000;

    // e.g. -GPe 0.1 -tau 5 -HCN som
    // Loop through command-line arguments
    for (int i = 1; i + 1 < argc; i+=2) {
        if (strcmp(argv[i], "-GPe") == 0) {
            W_GPe = strtod(argv[i + 1], NULL);
        } else if (strcmp(argv[i], "-Str") == 0) {
            W_Str = strtod(argv[i + 1], NULL);
        } else if (strcmp(argv[i], "-tau") == 0) {
            tau = strtod(argv[i + 1], NULL);
        } else if (strcmp(argv[i], "-GPe_stim") == 0) {
            GPe_stim = strtod(argv[i + 1], NULL);
        } else if (strcmp(argv[i], "-Str_stim") == 0) {
            Str_stim = strtod(argv[i + 1], NULL);
        }else if (strcmp(argv[i], "-HCN") == 0) {
            strncpy(HCN_choice, argv[i + 1], sizeof(HCN_choice) - 1);
            HCN_choice[sizeof(HCN_choice) - 1] = '\0';
        } else if (strcmp(argv[i], "-o") == 0) {
            strncpy(task_id, argv[i + 1], sizeof(task_id) - 1);
            task_id[sizeof(task_id) - 1] = '\0';
        } else {
            printf("Unimplemented option: %s\n", argv[i]);
            return 1;
        }
    }

    printf("W_GPe: %f\n", W_GPe);
    printf("W_Str: %f\n", W_Str);
    printf("tau: %f\n", tau);
    printf("GPe_stim: %f\n", GPe_stim);
    printf("Str_stim: %f\n", Str_stim);
    printf("HCN_choice: %s\n", HCN_choice);
    printf("task_id: %s\n", task_id);

    printf("batch simulation begins \n");
    batch_simulation(W_GPe, W_Str, tau, HCN_choice, GPe_stim, Str_stim, task_id);
    printf("batch finishes \n");

    gettimeofday(&stop_time, NULL);
    timersub(&stop_time, &start_time, &elapsed_time);
    printf("Total time was %f seconds.\n", elapsed_time.tv_sec + elapsed_time.tv_usec * 1e-6);
    return 0;
}