import numpy as np
from step0_experiment_data import tau_D1, tau_GPe, g_D1, g_GPe
import os
import os.path as path


tau_Str = np.around(np.array(tau_D1) * 1000., decimals=8)  # s -> ms
tau_GPe = np.around(np.array(tau_GPe) * 1000., decimals=8)  # s -> ms
W_Str = np.around(np.array(g_D1) / 100, decimals=8)  # nS -> nS/pF
W_GPe = np.around(np.array(g_GPe) / 800, decimals=8)  # nS -> nS/pF


def generate_sh_file(HCN_choice, stimulus_choice):
    file_path = path.join(path.dirname(__file__), "sh_commands", f"run_HCN_{HCN_choice}_{stimulus_choice}.sh")
    assert HCN_choice in ("zero", "den", "som")
    assert stimulus_choice in ("none", "GPe", "Str")
    try:
        with open(file_path, 'w') as file:
            file.write('#!/bin/bash\n')
            init_block = f"./step3_main.exe -HCN {HCN_choice} "

            if stimulus_choice == "GPe":
                for i, (tmp_tau, tmp_W) in enumerate(zip(tau_GPe, W_GPe)):
                    file.write(init_block + f"-GPe {tmp_W} -tau {tmp_tau} -GPe_stim 1000 -Str_stim -1 "
                                            f"-o raster_HCN_{HCN_choice}_Stim_{stimulus_choice}_{i} \n")
            elif stimulus_choice == "Str":
                for i, (tmp_tau, tmp_W) in enumerate(zip(tau_Str, W_Str)):
                    file.write(init_block + f"-Str {tmp_W} -tau {tmp_tau} -Str_stim 1000 -GPe_stim -1 "
                                            f"-o raster_HCN_{HCN_choice}_Stim_{stimulus_choice}_{i} \n")
            elif stimulus_choice == "none":
                file.write(init_block + f"-Str_stim -1 -GPe_stim -1 "
                                        f"-o raster_HCN_{HCN_choice}_Stim_{stimulus_choice}_0 \n")

        print(f'Data written to the {file_path} successfully.')
    except Exception as e:
        print(f'Error: {e}')


for i in ("zero", "den", "som"):
    for j in ("none", "GPe", "Str"):
        generate_sh_file(i, j)