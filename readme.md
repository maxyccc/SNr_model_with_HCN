# SNr Model with HCN Channel

A combined Python and C repository for multi-compartment simulations of substantia nigra pars reticulata (SNr) neuron with HCN channel. 

The HCN channel can be optionally included on the soma or dendrites. The model incorporates inputs from the globus pallidus externus (GPe) and the striatum.

## Model Reference

This SNr neuron model is adapted from:

- **Phillips RS, Rosner I, Gittis AH, & Rubin JE (2020)**. *The effects of chloride dynamics on substantia nigra pars reticulata responses to pallidal and striatal inputs.* eLife. [Link](https://elifesciences.org/articles/55592)

The pipeline was adapted and simplified from this [repository](https://github.com/JiananJian/SNr_Model_ACC).

---

## Environment Setup

To use this repository, you need the following:

- **Python** and some basic packages
- **C compiler** 

---

## Manual

# *Step 0*

### 1. Verify Input Files
Check that all required raw data are complete and uncorrupted:
- `step0_experiment_data.py`
- `step0_snr_model.h`

### 2. Modify Configuration
Edit the `SAVE_DIR` in `step0_config.h` to specify your preferred directory for saving `intermediate data` files.

Edit the `DURATION_init`, `DURATION_test`, `NUM_conductance`, `NUM_current` for your preferred experiment design.
### 3. [Optional] Generate Shell Commands
Run `step0_generate_commands.py` to create `.sh` shell scripts for use in later steps.

# *Step 1*

### 4. Compile and Run Data Preparation
Compile and execute `step1_prepare.c`. This step performs a grid search over the $g_{HCN}\ \times\ I_{app} $ space and generates following `intermediate data` files:

- `prepared_g.bin`: Range of $g_{HCN}$
- `prepared_I.bin`: Range of $\ I_{app} $
- `prepared_r_0.bin`: Firing rates without HCN channels for each grid
- `prepared_r_den.bin`: Firing rates with HCN channels in dendritic compartments for each grid
- `prepared_r_som.bin`: Firing rates with HCN channels in soma compartments for each grid

>**Execution Time**: ~1hr for a grid of $32\ g_{HCN}\ \times\ 300\ I_{app} $.

# *Step 2*

### 5. Find Optimal Conductance $g_{HCN}$ for Simulation
Run `step2_find_proper_g_value.py` to create following `intermediate data` files:

- `g_HCN_den.bin`: Optimal $g_{HCN}$ of dendritic HCN channel simulation for every prepared $I_{app}$ currents.
- `g_HCN_som.bin`: Optimal $g_{HCN}$ of somatic HCN channel simulation for every prepared $I_{app}$ currents.
- `g_HCN_zero.bin`: Optimal $g_{HCN}$ of simulation without HCN channel for every prepared $I_{app}$ currents.

and a figure in `figures` directory:

- `g_HCN_dist.jpg`: Grid search result for all three simulation types.

<img src="figures\g_HCN_dist.jpg" alt="g_HCN_distribution" style="width:600px;"/>

# *Step 3*
### 6. Simulation ~ Good Luck! :)
Compile `step3_main.c`. Then you can either

- Run `run_all.sh`, this will run all the `.sh` file you generate in pipeline 3.

or

- Execute `step3_main`. With following arguments implemented:
  - `-GPe`: The GPe input weight. See `W_GPe` in `step0_snr_model.h`.
  - `-Str`: The Str input weight. See `W_Str` in `step0_snr_model.h`.
  - `-tau`: The time constant for GPe/Str input. See `tau_GABA_som` and `tau_GABA_den` in `step0_snr_model.h`.
  - `-GPe_stim`: The time of GPe stim, in milliseconds, -1 for no stim.
  - `-Str_stim`: The time of Str stim, in milliseconds, -1 for no stim.
  - `-HCN`: Chose from `den`, `som`, `zero`, for HCN inserted on dendrite, soma, and nowhere.
  - `-o`: Task_id for you saved result filename.

The simulation result (rasters for every $I_{app}$ and its optimal $g_{HCN}$, see `step1_prepare.c` and `step0_config.h`) 
would in **one row**. It has `NUM_current` blocks, the first number `num_spikes` of each block is the number of spikes in this block, and the following `num_spikes` numbers if the timepoints of each spike in milliseconds.

>**Execution Time**: ~2hr for `run_all.sh`.


### 7. [Optional] Plot the Raster

Run `step3_plotting.py` to plot your raster in `Result` directory.

<img src="figures\example_raster.jpg" alt="example_raster" style="width:600px;"/>

---

## Contact
 <yag2@andrew.cmu.edu>

---
