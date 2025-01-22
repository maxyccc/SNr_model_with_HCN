#ifndef STEP0_SNR_MODEL_H
#define STEP0_SNR_MODEL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#ifndef timersub
#define timersub(a, b, result)                     \
    do {                                           \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;  \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) {               \
            --(result)->tv_sec;                    \
            (result)->tv_usec += 1000000;          \
        }                                          \
    } while (0)
#endif

/// @brief Gate parameters
typedef struct {
    double V_z; // Half-maximal activation voltage in mV
	double k_z; // in mV
    double x_min; // dimensionless, (1 - x_min) is the maximal activation
	double V_tau; // in mV
	double tau_0; // in ms
	double tau_1; // in ms
	double sig_0; // in mV
	double sig_1; // in mV
} Gate;

/// @brief Update gate variable 
/// @param gate Gate parameters
/// @param z Gate activation level
/// @param V Membrane potential in mV
/// @param dt Time step in ms
/// @return Increment of the gate activation level
void dz(const Gate *restrict gate, double *restrict z, double V, double dt) {
    double z_0 = (1. - gate->x_min) / (1. + exp((gate->V_z - V) / gate->k_z)); // logistic function
	double tau = gate->tau_0 + (gate->tau_1 - gate->tau_0) / (exp((gate->V_tau - V) / gate->sig_0) + exp((gate->V_tau - V) / gate->sig_1)); 
	double dz = (z_0 - *z) * (1 - exp(-dt / tau)); // stability ensured even when tau is smaller than dt
	*z += dz;
};

/// @brief Neuron state variables and parameters
typedef struct {    
	// Simulation time in ms
	double time;

	// Membrane potentials in mV
	double V_s;
	double V_d;

	// Gate activation levels, dimensionless
	double m_Na_f;
	double h_Na_f;
	double s_Na_f;
	double m_Na_p;
	double h_Na_p;
	double m_K;
	double h_K;
	double m_Ca;
	double h_Ca;
	double m_HCN_som;
	double m_HCN_den;
	
	// Gate parameters
	Gate prop_m_Na_f;
	Gate prop_h_Na_f;
	Gate prop_s_Na_f;
	Gate prop_m_Na_p;
	Gate prop_h_Na_p;
	Gate prop_m_K;
	Gate prop_h_K;
	Gate prop_m_Ca;
	Gate prop_h_Ca;
	Gate prop_m_HCN;

	// Synapse plasticity, dimensionless
	double D; // GPe depression
	double F; // Str facilitation
	double D_0; // initial D, also D max
	double F_0; // initial F, also F min
	double D_m; // D min
	double F_m; // F max

	// Ion concentrations in mM
	double Ca_in; // Somatic Calcium
	double Cl_som; // Somatic Cloride
	double Cl_den; // Dendritic Cloride

	// Ion channel channel delay constants in nS/pF = 1/ms
	double g_GABA_som;
	double g_GABA_den;
    
    // Stimulations, boolean
    int GPe_stim;
    int Str_stim;
    int SNr_stim;

	// Ion channel delay constants, in nS/pF = 1/ms
	double g_HCN_som;
	double g_HCN_den;
	double W_GPe;
	double W_Str;
	double W_SNr;
    double tau_GABA_som;
    double tau_GABA_den;

	// Electrical parameters in mV
	double V_th; // spike threshold in mV
    double I_app;
    double I_den;
    double E_leak;
} State;

State init_state() {
    State s = {
		.time = 0,
		.E_leak = -60,
		
		.m_Na_f = 0.1,
		.h_Na_f = 0.9,
		.s_Na_f = 0.9,
		.m_Na_p = 0.01,
		.h_Na_p = 0.04,
		.m_K = 0.01,
		.h_K = 0.9,
		.m_Ca = 0.001,
		.h_Ca = 0.001,
		.m_HCN_som = 0.01,
		.m_HCN_den = 0.01,
				
		.prop_m_Na_f = {
			.V_z = -30.2,
			.k_z = 6.2,	
			.x_min = 0,		
			.V_tau = 1,		
			.tau_0 = 0.05,	
			.tau_1 = 0.05,	
			.sig_0 = 1,		
			.sig_1 = 1,		
		},
		.prop_h_Na_f = {
			.V_z = -63.3,	
			.k_z = -8.1,	
			.x_min = 0,		
			.V_tau = -43,	
			.tau_0 = 0.59,	
			.tau_1 = 35.1,	
			.sig_0 = 10,		
			.sig_1 = -5,		
		},
		.prop_s_Na_f = {
			.V_z = -30,	
			.k_z = -0.4,	
			.x_min = 0.15,		
			.V_tau = -40,	
			.tau_0 = 10,	
			.tau_1 = 50,	
			.sig_0 = 18.3,		
			.sig_1 = -10,		
		},
		.prop_m_Na_p = {
			.V_z = -50,	
			.k_z = 3,	
			.x_min = 0,		
			.V_tau = -42.6,	
			.tau_0 = 0.03,	
			.tau_1 = 0.146,	
			.sig_0 = 14.4,		
			.sig_1 = -14.4,		
		},
		.prop_h_Na_p = {
			.V_z = -57,	
			.k_z = -4,	
			.x_min = 0.154,		
			.V_tau = -34,	
			.tau_0 = 10,	
			.tau_1 = 17,	
			.sig_0 = 26,		
			.sig_1 = -31.9,		
		},
		.prop_m_K = {
			.V_z = -26,	
			.k_z = 7.8,	
			.x_min = 0,		
			.V_tau = -26,	
			.tau_0 = 0.1,	
			.tau_1 = 14,	
			.sig_0 = 13,		
			.sig_1 = -12,		
		},
		.prop_h_K = {
			.V_z = -20,	
			.k_z = -10,	
			.x_min = 0.6,		
			.V_tau = 0,	
			.tau_0 = 5,	
			.tau_1 = 20,	
			.sig_0 = 10,		
			.sig_1 = -10,		
		},
		.prop_m_Ca = {
			.V_z = -27.5,	
			.k_z = 3,	
			.x_min = 0,		
			.V_tau = 0,	
			.tau_0 = 0.5,	
			.tau_1 = 0.5,	
			.sig_0 = 1,		
			.sig_1 = 1,		
		},
		.prop_h_Ca = {
			.V_z = -52.5,	
			.k_z = -5.2,	
			.x_min = 0,		
			.V_tau = 0,	
			.tau_0 = 18,	
			.tau_1 = 18,	
			.sig_0 = 1,		
			.sig_1 = 1,		
		},
		.prop_m_HCN = {
			.V_z = -76.4, 
			.k_z = -3.3,	
			.x_min = 0,		
			.V_tau = -76.4, 	
			.tau_0 = 0,		
			.tau_1 = 3625,	
			.sig_0 = 6.56,		
			.sig_1 = -7.48,		
		},

		.Cl_den = 6,
		.Cl_som = 6,
		.Ca_in = 2.5e-4,
		
		.g_GABA_som = 0,
		.g_GABA_den = 0,

		.GPe_stim = 0,
		.Str_stim = 0,
		.SNr_stim = 0,

		.W_GPe = .2,
		.W_Str = .4,
		.W_SNr = .1,

		.g_HCN_som = 0,
		.g_HCN_den = 0,
		.tau_GABA_som = 3,
		.tau_GABA_den = 7.2,

		.D_0 = 1,
		.F_0 = 1, // .145 in Phillips2020
		.D_m = 1, // .67 in Phillips2020
		.F_m = 1,
		
    	.I_app = 0,
		.I_den = 0,
    	.V_th = -30, // -35 in Phillips2020
	};

    s.V_s = s.E_leak; // -60
    s.V_d = s.E_leak; // -60

    s.D = s.D_0;
    s.F = s.F_0;

    s.prop_m_HCN.V_tau = s.E_leak; // -60 
    s.prop_m_HCN.V_z = s.E_leak; // -60

    return s;
}

/// @brief Update neuron variables
/// @param x Neuron 
/// @param dt Time step in ms
/// @return Whether a spike occurs
int f(State *restrict x, double dt) {
	// Ion charges in e
	#define z_Ca 2
	#define z_Cl -1  
	#define z_HCO3 -1
	#define z_GABA -1

	// Voltages in mV
	#define V_T 26.54
    #define E_HCO3 -20
	#define E_Na 50
	#define E_K -90
	#define E_TRPC3 -37
	#define E_HCN -30 
    
	// Conductances in nS/pF
	#define g_Na_f 35 // fast Na+ channel 
	#define g_Na_p .175 // persistent Na+ channel 
	#define g_K 50 // K+ channel 
	#define g_Ca .7 // Ca++ channel 
	#define g_leak 0.04 // .04 in Phillips2020 // leak channel
	#define g_TRPC3 .1 // TRPC3 channel 
    #define g_SK 0 // Calcium-activated K+ channel
    #define g_KCC2_som 0 // between 0.0 to 0.4 nS/pF
    #define g_KCC2_den 0 // between 0.0 to 0.4 nS/pF
    #define g_ton_som 0 // between 0.0 to 1.0 nS/pF
    #define g_ton_den 0 // between 0.0 to 1.0 nS/pF

	// Permeabilities, in arbitrary unit
    #define p_Cl 4 // Cl- ion permeability
    #define p_HCO3 1 // HCO3- ion permeability

	// Ion concentrations in mM
    #define HCO3_in 11.8
    #define HCO3_out 25
    #define Cl_out 120
    #define Ca_out 4

	// SK channel parameters
    #define k_SK 0.4 // m_SK half-maximal activation in mM   //modified
    #define n_SK 4 // m_SK Hill coefficient, dimensionless
    
	// Conductance in nS
    #define g_C 26.5 // dendrite-soma coupling

	// Charge to concentration conversion factors, in mM/fC
	#define alpha_Ca 0.925e-7 // 1e-8 in Phillips2020
	#define alpha_Cl_som 1.85e-7 // 1.77e-7 in Phillips2020
	#define alpha_Cl_den 2.3e-6 // 2.2125e-7 in Phillips2020 

	// Capacitances in pF
    #define C_som 100 // soma capacitance
    #define C_den 40 // dendrite capacitance

	#define Ca_min 5e-8 // in mM
	#define tau_Ca 250 // time constant in ms
	#define tau_SD 200 // time constant in ms
	#define tau_DS 80 // time constant in ms
    
	#define tau_D 1000 // time constant in ms
	#define tau_F 1000 // time constant in ms
	#define alpha_D .565 // dimensionless
	#define alpha_F .125 // dimensionless
	
    // Reversal potentials in mV
	double E_Ca = V_T * log(Ca_out / x->Ca_in) / z_Ca; 
	double E_Cl_som = V_T * log(Cl_out / x->Cl_som) / z_Cl; 
	double E_Cl_den = V_T * log(Cl_out / x->Cl_den) / z_Cl; 
	double E_GABA_som = V_T * log((p_Cl * Cl_out + p_HCO3 * HCO3_out) / (p_Cl * x->Cl_som + p_HCO3 * HCO3_in)) / z_GABA;
	double E_GABA_den = V_T * log((p_Cl * Cl_out + p_HCO3 * HCO3_out) / (p_Cl * x->Cl_den + p_HCO3 * HCO3_in)) / z_GABA;

	// Outward currents in pA/pF = mV/ms
	double I_Na_f = g_Na_f * pow(x->m_Na_f, 3) * x->h_Na_f * x->s_Na_f * (x->V_s - E_Na); // fast Na+ current
	double I_Na_p = g_Na_p * pow(x->m_Na_p, 3) * x->h_Na_p * (x->V_s - E_Na); // persistent Na+ current
	double I_K = g_K * pow(x->m_K, 4) * x->h_K * (x->V_s - E_K); // K+ current
	double I_Ca = g_Ca * x->m_Ca * x->h_Ca * (x->V_s - E_Ca); // Ca++ current
	double I_leak = g_leak * (x->V_s - x->E_leak); // leak current
	double I_DS = g_C / C_som * (x->V_s - x->V_d); // coupling current
	double I_HCN_som = x->g_HCN_som * x->m_HCN_som * (x->V_s - E_HCN); // HCN current
	double I_GABA_som = x->g_GABA_som * (x->V_s - E_GABA_som); // GABA current

	double m_SK = 1. / (1. + pow(k_SK / x->Ca_in, n_SK));
	double I_SK = g_SK * m_SK * (x->V_s - E_K); // Calcium-activated K+ current 

	double I_SD = g_C / C_den * (x->V_d - x->V_s); // coupling current
	double I_TRPC3 = g_TRPC3 * (x->V_d - E_TRPC3); // dendrite current
	double I_HCN_den = x->g_HCN_den * x->m_HCN_den * (x->V_d - E_HCN); // HCN current     //questioned
	double I_GABA_den = x->g_GABA_den * (x->V_d - E_GABA_den); // GABA current

	double chi_som = (E_HCO3 - E_GABA_som) / (E_HCO3 - E_Cl_som);
	double chi_den = (E_HCO3 - E_GABA_den) / (E_HCO3 - E_Cl_den);
	double I_chi_som = chi_som * (x->g_GABA_som + g_ton_som) * (x->V_s - E_Cl_som);
	double I_chi_den = chi_den * (x->g_GABA_den + g_ton_den) * (x->V_d - E_Cl_den);        // modified
	double I_KCC2_som = g_KCC2_som * (E_K - E_Cl_som);
	double I_KCC2_den = g_KCC2_den * (E_K - E_Cl_den);

	double dVs_dt = -(I_Na_f + I_Na_p + I_K + I_Ca + I_leak + I_SK + I_DS + I_HCN_som + I_GABA_som) + x->I_app / C_som;
	double dVd_dt = -(I_SD + I_TRPC3 + I_HCN_den + I_GABA_den) + x->I_den / C_den;

	// State variable updates
	x->time += dt;
    dz(&x->prop_m_Na_f, &x->m_Na_f, x->V_s, dt);
    dz(&x->prop_h_Na_f, &x->h_Na_f, x->V_s, dt);
    dz(&x->prop_s_Na_f, &x->s_Na_f, x->V_s, dt);
	dz(&x->prop_m_Na_p, &x->m_Na_p, x->V_s, dt);
	dz(&x->prop_h_Na_p, &x->h_Na_p, x->V_s, dt);
	dz(&x->prop_m_K, &x->m_K, x->V_s, dt);
	dz(&x->prop_h_K, &x->h_K, x->V_s, dt);
	dz(&x->prop_m_Ca, &x->m_Ca, x->V_s, dt);
	dz(&x->prop_h_Ca, &x->h_Ca, x->V_s, dt);
	dz(&x->prop_m_HCN, &x->m_HCN_som, x->V_s, dt);
	dz(&x->prop_m_HCN, &x->m_HCN_den, x->V_d, dt);
    
	x->Ca_in += dt * (Ca_min - x->Ca_in) / tau_Ca;
	x->Ca_in -= dt * alpha_Ca * C_som * I_Ca;

	x->Cl_som += dt * (x->Cl_den - x->Cl_som) / tau_SD;
	x->Cl_den += dt * (x->Cl_som - x->Cl_den) / tau_DS;
	x->Cl_som += dt * alpha_Cl_som * (I_KCC2_som + I_chi_som);    // modified
	x->Cl_den += dt * alpha_Cl_den * (I_KCC2_den + I_chi_den);    // modified

	x->g_GABA_som *= exp(-dt / x->tau_GABA_som);
	x->g_GABA_den *= exp(-dt / x->tau_GABA_den);
	x->D += (x->D_0 - x->D) * (1 - exp(-dt / tau_D));
	x->F += (x->F_0 - x->F) * (1 - exp(-dt / tau_F));

	x->g_GABA_som += x->W_SNr * x->SNr_stim;
	if (x->GPe_stim) {
		x->g_GABA_som += x->W_GPe * x->D;
		x->D += alpha_D * (x->D_m - x->D);
	}
	if (x->Str_stim) {
		x->g_GABA_den += x->W_Str * x->F;
		x->F += alpha_F * (x->F_m - x->F);
	}

	double V_0 = x->V_s;
	x->V_s += dt * dVs_dt;
	x->V_d += dt * dVd_dt;
	return (V_0 < x->V_th) && (x->V_s >= x->V_th);
}

void write_binary_file(const char *filename, const double *data, size_t dataSize) {
    // Open file in binary write mode
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    
    // Write data to file
    size_t written = fwrite(data, sizeof(double), dataSize, file);
    if (written != dataSize) {
        perror("Error writing data to file");
    }
    
    // Close the file
    fclose(file);
}

void append_binary_file(const char *filename, const double *data, size_t dataSize) {
    // Open file in binary write mode
    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    
    // Write data to file
    size_t written = fwrite(data, sizeof(double), dataSize, file);
    if (written != dataSize) {
        perror("Error writing data to file");
    }
     
    // Close the file
    fclose(file);
}

void read_binary_file(const char *filename, double *data, size_t *dataSize) {
	// Open the file in binary mode

	FILE *file = fopen(filename, "rb"); 
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

	// Seek to the end of the file to determine its size
	fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
	printf("Loaded file size: %ld bytes\n", fileSize);

	// Calculate the number of double elements in the file
    *dataSize = fileSize / sizeof(double);

    // Allocate memory for the data array if it's not already allocated
    if (!data) {
        data = (double *)malloc(*dataSize * sizeof(double));
    }
	if (!data) {
		perror("Memory allocation failed");
		fclose(file);
		return;
	}

	// Read the file content into the data array
    size_t bytesRead = fread(data, sizeof(double), *dataSize, file);
    if (bytesRead != *dataSize) {
        perror("Error reading file");
    }
	
    fclose(file); // Close the file
}

#endif // STEP0_SNR_MODEL_H