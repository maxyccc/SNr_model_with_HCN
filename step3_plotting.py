import pandas as pd
import os
import os.path as path
import matplotlib.pyplot as plt
import matplotlib.patches as ptchs
import numpy as np
plt.rcParams["font.family"] = "Arial"
plt.rcParams['font.size'] = 8
TIME_BIN = 10


def plot_raster(spike_times, num_trials):
    fig, axs = plt.subplots(1, 3, sharex='col', sharey='row', width_ratios=[2, 1, 1])

    for trial_id in range(num_trials):
        tmp_trial = spike_times[trial_id]
        for spike_time in tmp_trial:
            axs[0].scatter(spike_time, trial_id, facecolor='black', marker='.', s=25, alpha=0.4,
                              edgecolor='none')
            axs[1].scatter(spike_time, trial_id, facecolor='black', marker='.', s=25, alpha=0.4,
                              edgecolor='none')
            axs[2].scatter(spike_time, trial_id, facecolor='black', marker='.', s=25, alpha=0.4,
                              edgecolor='none')
    for i in range(3):
        axs[i].spines[['right', 'top']].set_visible(False)
        axs[i].add_patch(ptchs.Rectangle((1000, -0.5), 10, num_trials,
                                            facecolor='blue', alpha=0.3, edgecolor='none', ))
    axs[0].set_ylabel("Trial ID")

    axs[0].set_xlabel("Time (ms)")
    axs[1].set_xlabel("-200ms~+200ms - Zoom In")
    axs[2].set_xlabel("-50ms~+50ms - Zoom In")
    axs[1].set_xlim(800, 1200)
    axs[2].set_xlim(950, 1050)
    fig.set_size_inches(10, 4)

    fig.savefig(path.join(path.dirname(__file__), "figures", "example_raster.jpg"), bbox_inches='tight', dpi=300)
    plt.close(fig)


def main():
    raw_data = pd.read_csv(path.join("result", "example_result.csv"), header=None).to_numpy()[0]

    spike_times = []
    num_trials = 0
    cnt = 0
    while cnt < raw_data.shape[0] and raw_data[cnt] != "END":
        num_trials += 1
        spike_times.append([])
        num_spike = int(raw_data[cnt])
        print(f"Trial start at entry #{cnt}, number of spikes: {num_spike}")
        cnt += 1
        for i in range(num_spike):
            spike_times[-1].append(raw_data[cnt])
            cnt += 1
    print(f"number of trials :{num_trials}")
    # # For sort trials by its spike number
    # spike_cnts = [len(tmp_trial) for tmp_trial in spike_times]
    # _, sorted_spike_times = zip(*sorted(zip(spike_cnts, spike_times)))
    plot_raster(spike_times[::5], int(num_trials/5))


if __name__ == "__main__":
    main()