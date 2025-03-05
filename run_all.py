import numpy as np
from bio_data.EphysMeasurement import tau_D1, tau_GPe, g_D1, g_GPe
import os
import os.path as path
import time
import subprocess
from datetime import datetime


tau_Str = np.around(np.array(tau_D1) * 1000., decimals=8)  # s -> ms
tau_GPe = np.around(np.array(tau_GPe) * 1000., decimals=8)  # s -> ms
W_Str = np.around(np.array(g_D1) / 100, decimals=8)  # nS -> nS/pF
W_GPe = np.around(np.array(g_GPe) / 800, decimals=8)  # nS -> nS/pF

script_dir = path.join(path.dirname(__file__), "sh_commands")
result_dir = path.join(path.dirname(__file__), "simulation_result")


def generate_full_simulation_sh_file(HCN_choice):
    file_path = path.join(script_dir, f"run_HCN_{HCN_choice}.sh")
    task_id = datetime.now().strftime("%Y%m%d_%H%M%S") + f"_{HCN_choice}"
    os.makedirs(path.join(result_dir, task_id))
    assert HCN_choice in ("zero", "den", "som")
    try:
        with open(file_path, 'w') as file:
            file.write('#!/bin/bash\n')
            init_block = f"./step3_simulation.exe -HCN {HCN_choice} "
            for stimulus_choice in ("none", "GPe", "Str"):
                if stimulus_choice == "GPe":
                    for i, (tmp_tau, tmp_W) in enumerate(zip(tau_GPe, W_GPe)):
                        file.write(init_block + f"-GPe {tmp_W} -tau {tmp_tau} -GPe_stim 1000 -Str_stim -1 "
                                                f"-o {task_id}/raster_HCN_{HCN_choice}_Stim_{stimulus_choice}_{str(i).zfill(2)} \n")
                elif stimulus_choice == "Str":
                    for i, (tmp_tau, tmp_W) in enumerate(zip(tau_Str, W_Str)):
                        file.write(init_block + f"-Str {tmp_W} -tau {tmp_tau} -Str_stim 1000 -GPe_stim -1 "
                                                f"-o {task_id}/raster_HCN_{HCN_choice}_Stim_{stimulus_choice}_{str(i).zfill(2)} \n")
                elif stimulus_choice == "none":
                    file.write(init_block + f"-Str_stim -1 -GPe_stim -1 "
                                            f"-o {task_id}/raster_HCN_{HCN_choice}_Stim_{stimulus_choice}_00 \n")

        print(f'Data written to the {file_path} successfully.')
    except Exception as e:
        print(f'Error: {e}')
    return file_path


def main():
    for i in ("zero", "den", "som"):
    # for i in ("den", ):
        file_path = generate_full_simulation_sh_file(i)
        script_path = file_path.replace("\\", "/")
        print(f"Running {script_path}...")
        subprocess.run([script_path], shell=True)
        time.sleep(10)


if __name__ == "__main__":
    main()