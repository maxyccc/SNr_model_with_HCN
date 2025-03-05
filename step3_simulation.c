#include "bio_data/SNrModel.h"
#include "step0_config.h"
#include <stdio.h>
#include <sys/stat.h>

#ifdef _WIN32 // For _mkdir on Windows
    #include <direct.h>
    #define MAKE_DIR(path) _mkdir(path)
#else  // For mkdir on macOS/Linux
    #include <sys/types.h>
    #include <sys/stat.h>
    #define MAKE_DIR(path) mkdir(path, 0777)
#endif

char* dirname(const char* path) {  // ChatGPT generated
    // Make a copy of the path to avoid modifying the original
    char* path_copy = strdup(path);
    if (path_copy == NULL) {
        return NULL; // Memory allocation failed
    }

    // Find the last occurrence of the directory separator '/'
    char* last_slash = strrchr(path_copy, '/');

    if (last_slash != NULL) {
        // If a slash is found, truncate the string at that point
        if (last_slash == path_copy) {
            // If the slash is the first character, return "/"
            last_slash[1] = '\0';
        } else {
            // Otherwise, truncate the string at the slash
            *last_slash = '\0';
        }
    } else {
        // If no slash is found, return "."
        strcpy(path_copy, ".");
    }

    return path_copy;
}

int directory_not_exists(const char *path) {
    struct stat info;
    if (stat(path, &info) == 0 && (info.st_mode & S_IFDIR)) {
        return 0; // Directory exists
    }
    return 1;
}

typedef struct {
    double *spike_times;
    double *I_HCN_som;
    double *m_HCN_som;
    double *g_HCN_som;
    double *I_app;
    double *I_TRPC3;
    double *I_HCN_den;
    double *m_HCN_den;
    double *g_HCN_den;
    double *Vs;
    double *Vd;
    double *I_GABA_som;
    double *E_GABA_som;
    double *g_GABA_som;
    double *D;
    double *I_GABA_den;
    double *E_GABA_den;
    double *g_GABA_den;
    double *F;
    int num_spikes;
} Spikes;


void write_csv(const char *filename, double *data, int num) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }
    for (int i = 0; i < num; i++) {
        fprintf(file, "%f,", data[i]);
    }
    fprintf(file, "END\n");
    fclose(file);
    printf("Result saved in %s \n", filename);
}


Spikes spike_simulation(State *restrict s, int duration, double GPe_stim_time, double Str_stim_time) {
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


Spikes full_simulation(State *restrict s, int duration, double GPe_stim_time, double Str_stim_time) {
    Spikes spikes;
    spikes.num_spikes = 0;
    spikes.spike_times = (double *)malloc(CONFIG_spikes_init_size * sizeof(double));
    spikes.I_HCN_som = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.m_HCN_som = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.g_HCN_som = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.I_app = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.I_TRPC3 = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.I_HCN_den = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.m_HCN_den = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.g_HCN_den = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.Vs = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.Vd = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.I_GABA_som = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.E_GABA_som = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.g_GABA_som = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.D = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.I_GABA_den = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.E_GABA_den = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.g_GABA_den = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
    spikes.F = (double *)malloc(CONFIG_1ms_step_num*duration * sizeof(double));
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
        spikes.I_HCN_som[i] = s->g_HCN_som * s->m_HCN_som * (s->V_s - E_HCN);
        spikes.m_HCN_som[i] = s->m_HCN_som;
        spikes.g_HCN_som[i] = s->g_HCN_som;
        spikes.I_app[i] = s->I_app;
        spikes.I_TRPC3[i] = g_TRPC3 * (s->V_d - E_TRPC3);
        spikes.I_HCN_den[i] = s->g_HCN_den * s->m_HCN_den * (s->V_d - E_HCN);
        spikes.m_HCN_den[i] = s->m_HCN_den;
        spikes.g_HCN_den[i] = s->g_HCN_den;
        spikes.Vs[i] = s->V_s;
        spikes.Vd[i] = s->V_d;
        spikes.E_GABA_som[i] = V_T * log((p_Cl * Cl_out + p_HCO3 * HCO3_out) / (p_Cl * s->Cl_som + p_HCO3 * HCO3_in)) / z_GABA;;
        spikes.I_GABA_som[i] = s->g_GABA_som * (s->V_s - spikes.E_GABA_som[i]);
        spikes.g_GABA_som[i] = s->g_GABA_som;
        spikes.D[i] = s->D;
        spikes.E_GABA_den[i] = V_T * log((p_Cl * Cl_out + p_HCO3 * HCO3_out) / (p_Cl * s->Cl_den + p_HCO3 * HCO3_in)) / z_GABA;;
        spikes.I_GABA_den[i] = s->g_GABA_den * (s->V_d - spikes.E_GABA_den[i]);
        spikes.g_GABA_den[i] = s->g_GABA_den;
        spikes.F[i] = s->F;
    }
    return spikes;
}


int batch_simulation(double W_GPe, double W_Str, double tau, const char* HCN,
    double GPe_stim, double Str_stim, const char* task_id, int num_sim) {
    // load conductances
    char g_value_filename[512];
    if (strcmp(HCN, "som") == 0) {
        strcpy(g_value_filename, SAVE_DIR "selected_g_HCN_som.bin");
    } else if (strcmp(HCN, "den") == 0) {
        strcpy(g_value_filename, SAVE_DIR "selected_g_HCN_den.bin");
    } else {
        strcpy(g_value_filename, SAVE_DIR "selected_g_HCN_zero.bin");
    }
    double g_HCN[1024];
    size_t N0;
    printf("%s \n", g_value_filename);
    read_binary_file(g_value_filename, g_HCN, &N0);

    // load current
    char I_value_filename[512];
    if (strcmp(HCN, "som") == 0) {
        strcpy(I_value_filename, SAVE_DIR "selected_I_HCN_som.bin");
    } else if (strcmp(HCN, "den") == 0) {
        strcpy(I_value_filename, SAVE_DIR "selected_I_HCN_den.bin");
    } else {
        strcpy(I_value_filename, SAVE_DIR "selected_I_HCN_zero.bin");
    }
    double I[1024];
    size_t N1;
    printf("%s \n", I_value_filename);
    read_binary_file(I_value_filename, I, &N1);

    // simulate for all possible conductances
    char filename[512];
    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, ".csv");
    printf("Result writing in %s \n", filename);
    FILE *result = fopen(filename, "w");
    if (result == NULL) {
        perror("Failed to open file");
        return 1;  // Or handle the error as needed
    }
    for (int j = 0; j < num_sim; j++) {
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
        Spikes spikes = spike_simulation(&s, SIM_DURATION_total, GPe_stim, Str_stim);
        printf("#%d: I_app: %f, g_HCN_%s: %f, %d spikes \n", j, I[j], HCN, g_HCN[j], spikes.num_spikes);
        fprintf(result, "%d,", spikes.num_spikes);
        for (int i = 0; i < spikes.num_spikes; i++) {
            fprintf(result, "%f,", spikes.spike_times[i]);
        }
        free(spikes.spike_times);
    }
    fprintf(result, "END\n");
    fclose(result);
    printf("Result saved in %s \n", filename);
    return 0;
}

int single_simulation(double W_GPe, double W_Str, double tau, const char* HCN,
    double GPe_stim, double Str_stim, const char* task_id, double g_HCN, double I_app) {
    // simulate for all possible conductances
    char filename[512];
    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, ".csv");
    printf("Result writing in %s \n", filename);
    FILE *result = fopen(filename, "w");
    if (result == NULL) {
        perror("Failed to open file");
        return 1;  // Or handle the error as needed
    }

    State s = init_state();
    s.W_GPe = W_GPe;
    s.tau_GABA_som = tau;
    s.W_Str = W_Str;
    s.tau_GABA_den = tau;
    s.I_app = I_app;
    if (strcmp(HCN, "som") == 0) {
        s.g_HCN_som = g_HCN;
    } else if (strcmp(HCN, "den") == 0) {
        s.g_HCN_den = g_HCN;
    }
    Spikes spikes = full_simulation(&s, SIM_DURATION_total, GPe_stim, Str_stim);
    printf("#1: I_app: %f, g_HCN_%s: %f, %d spikes \n", I_app, HCN, g_HCN, spikes.num_spikes);
    fprintf(result, "%d,", spikes.num_spikes);
    for (int i = 0; i < spikes.num_spikes; i++) {
        fprintf(result, "%f,", spikes.spike_times[i]);
    }
    free(spikes.spike_times);

    fprintf(result, "END\n");
    fclose(result);
    printf("Result saved in %s \n", filename);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_I_HCN_som.csv");
    write_csv(filename, spikes.I_HCN_som, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.I_HCN_som);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_m_HCN_som.csv");
    write_csv(filename, spikes.m_HCN_som, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.m_HCN_som);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_g_HCN_som.csv");
    write_csv(filename, spikes.g_HCN_som, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.g_HCN_som);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_I_app.csv");
    write_csv(filename, spikes.I_app, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.I_app);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_I_TRPC3.csv");
    write_csv(filename, spikes.I_TRPC3, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.I_TRPC3);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_I_HCN_den.csv");
    write_csv(filename, spikes.I_HCN_den, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.I_HCN_den);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_m_HCN_den.csv");
    write_csv(filename, spikes.m_HCN_den, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.m_HCN_den);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_g_HCN_den.csv");
    write_csv(filename, spikes.g_HCN_den, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.g_HCN_den);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_Vd.csv");
    write_csv(filename, spikes.Vd, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.Vd);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_Vs.csv");
    write_csv(filename, spikes.Vs, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.Vs);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_I_GABA_som.csv");
    write_csv(filename, spikes.I_GABA_som, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.I_GABA_som);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_E_GABA_som.csv");
    write_csv(filename, spikes.E_GABA_som, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.E_GABA_som);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_g_GABA_som.csv");
    write_csv(filename, spikes.g_GABA_som, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.g_GABA_som);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_D.csv");
    write_csv(filename, spikes.D, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.D);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_I_GABA_den.csv");
    write_csv(filename, spikes.I_GABA_den, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.I_GABA_den);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_E_GABA_den.csv");
    write_csv(filename, spikes.E_GABA_den, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.E_GABA_den);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_g_GABA_den.csv");
    write_csv(filename, spikes.g_GABA_den, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.g_GABA_den);

    strcpy(filename, RESULT_DIR);
    strcat(filename, task_id);
    strcat(filename, "_F.csv");
    write_csv(filename, spikes.F, SIM_DURATION_total*CONFIG_1ms_step_num);
    free(spikes.F);

    return 0;
}


int main(int argc, char *argv[]) {
    struct timeval start_time, stop_time, elapsed_time;
    gettimeofday(&start_time, NULL);

    char HCN_choice[8] = "zero", task_id[128] = "test/mitten";
    int num_sim = NUM_samples;
    double W_GPe = 0, W_Str = 0, tau = 0;
    double GPe_stim = 1000, Str_stim = 1000;
    double g_HCN = DEFAULT_g_HCN;
    double I_app = DEFAULT_I_app;

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
        } else if (strcmp(argv[i], "-g_HCN") == 0) {
            g_HCN = strtod(argv[i + 1], NULL);
        } else if (strcmp(argv[i], "-I_app") == 0) {
            I_app = strtod(argv[i + 1], NULL);
        } else if (strcmp(argv[i], "-num") == 0) {
            num_sim = strtol(argv[i + 1], NULL, 10);
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

    printf("##########################\n");
    printf("############# Hyperparameters \n");
    printf("##########################\n");
    printf("W_GPe: %f\n", W_GPe);
    printf("W_Str: %f\n", W_Str);
    printf("tau: %f\n", tau);
    printf("GPe_stim: %f\n", GPe_stim);
    printf("Str_stim: %f\n", Str_stim);
    printf("NUM_simulation: %d\n", num_sim);
    printf("HCN_choice: %s\n", HCN_choice);
    printf("task_id: %s\n", task_id);

    if (num_sim == 1) {
        printf("g_HCN: %f\n", g_HCN);
        printf("I_app: %f\n", I_app);
        printf("\n");
        printf("single simulation begins \n");
        char task_path[512];
        strcpy(task_path, RESULT_DIR);
        strcat(task_path, task_id);
        // char *dirName = dirname(task_path);
        if (directory_not_exists(task_path)) {
            if (MAKE_DIR(task_path) == 0) {
                printf("Directory '%s' created successfully.\n", task_path);
            } else {
                printf("Error creating directory '%s'.\n", task_path);
                return 1;
            }
        }
        strcat(task_id, "/single");
        single_simulation(W_GPe, W_Str, tau, HCN_choice, GPe_stim, Str_stim, task_id, g_HCN, I_app);
        printf("single finishes \n");
    } else {
        printf("\n");
        printf("batch simulation begins \n");
        batch_simulation(W_GPe, W_Str, tau, HCN_choice, GPe_stim, Str_stim, task_id, num_sim);
        printf("batch finishes \n");
    }

    gettimeofday(&stop_time, NULL);
    timersub(&stop_time, &start_time, &elapsed_time);
    printf("##########################\n");
    printf("############# Total time was %f seconds. \n", elapsed_time.tv_sec + elapsed_time.tv_usec * 1e-6);
    printf("##########################\n");
    printf("\n");
    return 0;
}