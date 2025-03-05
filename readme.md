# SNr Model with HCN Channel

A combined Python and C repository for multi-compartment simulations (2 compartments: soma and dendrite) of substantia nigra pars reticulata (SNr) neuron with HCN channel. 
The HCN channel can be optionally inserted on the soma or dendrites. The model incorporates inputs from the globus pallidus externus (GPe) and the striatum (Str).

### Model Reference

This SNr neuron model is adapted from:

- **Phillips RS, Rosner I, Gittis AH, & Rubin JE (2020)**. *The effects of chloride dynamics on substantia nigra pars reticulata responses to pallidal and striatal inputs.* eLife. [View the article](https://elifesciences.org/articles/55592)

Below is the model schematic diagram from the paper:  

[//]: # (&#40;*Note: The KCC2 channel is not implemented in this repository so far.*&#41;)

<img src="figures\model_schematics.png" alt="model schematic" style="width:600px;"/>

The pipeline was adapted and simplified from this [repository](https://github.com/JiananJian/SNr_Model_ACC).

---

### Prerequisites

- **Python**: Version 3.8 or higher is recommended.
  - required Python packages:
    - `numpy`
    - `pandas`
    - `matplotlib`
- **C Compiler**: A compatible C compiler is required for certain functionalities.

---

# 🚀*Quick Start* Manual 

## *Step 0* - prepare

---

### 0.1 Check file integrity
Check that all required raw data are complete and uncorrupted under directory `bio_data`:
- `EphysMeasurement.py`: ephys measurement of GPe/Str input conductance and time constants.
- `OnePulse.xlsx`: spike series from electrode recording.
- `SNrModel.h`: head file for the SNr neuron model. (see [reference](https://elifesciences.org/articles/55592) for better understanding)

### 0.2 Modify Configuration 
Edit the `SAVE_DIR` and `RESULT_DIR` in `step0_config.h` to specify your preferred directory for saving `intermediate data` files and `simulation result` files.

You can also edit the hyperparameters for [step1: grid search](#step-1---grid-search) and 
[step3: simulation](#step-3---simulation) here based on your preferred experiment design.

---
## *Step 1* - grid search

Compile and execute `step1_grid_search_g_HCN.c`. This step performs a grid search over the 
$g_{HCN}\ \times \ I_{app}$ space and generates following `intermediate_result` files:

- `prepared_g.bin`: searched series of $g_{HCN}$
- `prepared_I.bin`: searched series of $I_{app}$
- `prepared_r_0.bin`: firing rates without HCN channels for each grid
- `prepared_r_den.bin`: firing rates with HCN channels in dendritic compartments for each grid
- `prepared_r_som.bin`: firing rates with HCN channels in somatic compartments for each grid

---
You can edit `step0_config.h` to change the search space. See `step1_grid_search_g_HCN.c: line 63` for further details.

---
To run this step:
```bash
clang -o step1_grid_search_g_HCN.exe step1_grid_search_g_HCN
step1_grid_search_g_HCN.exe
```

>**ETA**: ~10min for a grid of $32\ g_{HCN}\ \times\ 32\ I_{app} $.

---
## *Step 2* - sample HCN conductance


Run `step2_find_proper_g_value.py` to sample the ($I_{app}, g_{HCN}$) pairs based on the grid search results.  

Based on our data, we expect a **32% reduction in firing rate** when the HCN channel is blocked. 
Thus, given the target firing rate, we sample $g_{HCN}$ and its corresponding $I_{app}$ following this criterion.  

---

This step creates the following files in `intermediate_data`:

- `selected_g_HCN_den.bin`: selected $g_{HCN}$ of dendritic HCN channel simulation
- `selected_g_HCN_som.bin`: selected $g_{HCN}$ of somatic HCN channel simulation
- `selected_g_HCN_zero.bin`: selected $g_{HCN}$ of simulation without HCN channel
- `selected_I_HCN_den.bin`: corresponding $I_{app}$ for each $g_{HCN}$ in `selected_g_HCN_den.bin`
- `selected_r_HCN_den.bin`: corresponding firing rate for each $g_{HCN}$-$I_{app}$ pairs in `selected_g_HCN_den.bin` and `selected_I_HCN_den.bin`
- `...`

and two figures in `figures` directory:

- `grid_search_result.jpg`: grid search result for all 3 simulation types.  
*viridis*: somatic HCN. *cool*: dendritic HCN. *gray*: no HCN. Two black line indicate the optimal $g_{HCN}$-$I_{app}$ pairs that satisfy the **32% reduction** criterion.  

- `frequency_histogram.jpg`: histogram for target baseline firing rate distribution, see `step2_generate_I_g_pairs.py: line 104` for further details. Also includes histograms for electrode-recorded data and final sampled results.


<p float="left">
<img src="figures\grid_search_result.jpg" alt="grid_search_result" style="width:300px;"/>
<img src="figures\frequency_histogram.jpg" alt="frequency_histogram" style="width:300px;"/>

</p>

To run this step:
```bash
python step2_generate_I_g_pairs.py
```
---
## *Step 3* - simulation



This step will run simulation based on [step2](#step-2---sample-hcn-conductance) sampled dataset.

### 3.1 Single simulation
Compile and execute `step3_simulation.c`
- With following arguments:
  - `-GPe`: the GPe input weight. See `W_GPe` in `bio_data/SNrModel.h`.
  - `-Str`: the Str input weight. See `W_Str` in `bio_data/SNrModel.h`.
  - `-tau`: the time constant for GPe/Str input. See `tau_GABA_som` and `tau_GABA_den` in `bio_data/SNrModel.h`.
  - `-GPe_stim`: the onset time of GPe stim, in milliseconds. Set it as `-1` for no stim.
  - `-Str_stim`: the onset time of Str stim, in milliseconds. Set it as `-1` for no stim.
  - `-HCN`: chose from `den`, `som`, `zero`, for HCN inserted on dendrite, soma, and nowhere.
  - `-o`: task_id for you saved result.
  - `-num`: number of sampled simulation.
- You can activate additional arguments, by set `-num` to 1. This will enable you to record multiple intermediate state variable during simulation 
(See `step3_simulation.c: line 114` for further details).
  - `g_HCN`: specify the $g_{HCN}$
  - `I_app`: specify the $I_{app}$

Example code (see `run_single.sh`):

```bash
clang -o step3_simulation.exe step3_simulation
step3_simulation.exe -HCN den -GPe 0.03047575 -tau 8.38447 -GPe_stim 1000 -Str_stim -1 -o mitten -num 1 -g_HCN 1.5 -I_app -50
```

---

The `step3_simulation` will simulate SNr cell for every / specified $I_{app}$ - $g_{HCN}$ pairs.
Simulation results (rasters) are saved in `.csv` format with **one row** of `num` blocks. Each block contains following elements consecutively:
- `num_spikes`: the number of spikes in this trial
- `spike_times`: `num_spikes` number of spike timestamps in milliseconds. 

---
### 3.2 Multiple simulation

You can also run simulation based on the parameters we measured in ephys experiments.
To do this:

```bash
clang -o step3_simulation.exe step3_simulation
python run_all.py
```
This will generate multiple `.sh` shell scripts in directory `sh_commands`, and then automatically run all of them.
The simulation result will be stored in directory `simulation_result` named by the executing datetime.

---


### 3.3 Visualization

To visualize your result:
```bash
python visualization.py --task_id mitten
```
This scripts will automatically detect whether your simulation is `single-type` or `multiple-type`.

- If this is a single simulation, the visualization of each intermediate variable will be plotted.
<img src="figures\mitten_single.jpg" alt="single simulation" style="width:600px;"/>

 
- If this is a multiple simulation, the comparison between the simulation and actual bio-data will be plotted.
Also includes an aggregated `.xlsx` data sheet stored in directory `simulation_result`. The aggregated `.xlsx` file
include multiple columns. Each column represents one trial/simulation and different settings will be marked at first row.
<img src="figures\20250305_131941_den_HCN_den.jpg" alt="multiple simulation" style="width:600px;"/>

---

# Contact
For any questions, please contact:
 <yag2@andrew.cmu.edu>

---
