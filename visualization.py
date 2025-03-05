import matplotlib.pyplot as plt
import os
import os.path as path
import numpy as np
import matplotlib.patches as ptchs
import argparse
from utils import *


plt.rcParams["font.family"] = "Arial"
plt.rcParams['font.size'] = 10
STEP_SIZE = 10
WINDOW_SIZE = 20


def plot_raster(ax, cells_of_trials, color=None, ax_title=None, cell_num=10):
    for row_id, (cell, trials) in enumerate(cells_of_trials.items()):
        sample_trial = trials[np.random.choice(len(trials))]
        ax.scatter(sample_trial, row_id*np.ones_like(sample_trial), facecolor='black', marker='.', s=25, alpha=0.7,
                   edgecolor='none')
        if row_id >= cell_num:
            break
    ax.spines[['right', 'top']].set_visible(False)
    if color is not None:
        ax.add_patch(ptchs.Rectangle((1000, -0.5), 10, len(cells_of_trials.keys()),
                                     facecolor=color, alpha=0.3, edgecolor='none', ))
    ax.set_xticks((900, 1000, 1100), ("-100", "0", "100"))
    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Cell #")
    ax.set_xlim(800, 1200)
    ax.set_ylim(-0.5, cell_num+0.5)
    if ax_title is not None:
      ax.set_title(ax_title, fontsize=5)


def calculate_firing_rate(ts, window_size, spike_series, baseline_range=None):
    firing_rates = []
    for t in ts:
        count = np.sum((spike_series >= (t - window_size / 2)) & (spike_series < (t + window_size / 2)))
        firing_rates.append(count / window_size)
    if baseline_range is not None:
        baseline_fr = np.sum((spike_series >= baseline_range[0]) &
                                (spike_series < baseline_range[1]))/(baseline_range[1]-baseline_range[0])
        return np.array(firing_rates)/baseline_fr
    else:
        return np.array(firing_rates)


def plot_firing_rate(ax, GPe_cells_of_trials, Str_cells_of_trials, ax_title=None):
    ts = np.arange(800, 1200, STEP_SIZE)

    for data_cells_of_trials, data_color in zip((GPe_cells_of_trials, Str_cells_of_trials), ("Blue", "Orange")):
        all_fr = np.stack([calculate_firing_rate(ts, WINDOW_SIZE, np.concatenate(trials), baseline_range=(100, 1000))
                           for trials in data_cells_of_trials.values()], axis=0)
        mean_fr, sem_fr = np.mean(all_fr, axis=0), np.std(all_fr, axis=0)/np.sqrt(all_fr.shape[0])
        ax.plot(ts, mean_fr, color=data_color, lw=0.5)
        ax.fill_between(ts, mean_fr + sem_fr, mean_fr - sem_fr, color=data_color, alpha=0.2, lw=0)
    ax.spines[['right', 'top']].set_visible(False)
    ax.axhline(y=1, ls='--', color='gray', alpha=0.4, lw=1)
    ax.axvline(x=1000, ls='--', color='gray', alpha=0.4, lw=1)
    ax.set_xticks((900, 1000, 1100), ("-100", "0", "100"))
    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Norm. FR")
    ax.set_xlim(900, 1150)
    ax.set_ylim(0, 1.6)
    if ax_title is not None:
      ax.set_title(ax_title, fontsize=5)


def calculate_cdf(cells_of_trials, start_point, inf_value=10000):
    first_spikes = []
    for trials in cells_of_trials.values():
        trial_first_spikes = []
        for trial in trials:
            if np.sum(trial >= start_point) > 0:
                trial_first_spikes.append(np.min(trial[trial >= start_point]))
            else:
                trial_first_spikes.append(inf_value)
        first_spikes += trial_first_spikes
        # first_spikes.append(np.mean(trial_first_spikes))
    first_spikes = np.array(sorted(first_spikes))
    p = 1. * np.arange(len(first_spikes)) / (len(first_spikes) - 1)
    return first_spikes, p


def plot_time2recover(ax, baseline_GPe, baseline_Str, target_GPe, target_Str,
                      label_baseline, label_target,
                      ax_title=None):
    start_point_baseline = 1004 if label_baseline == "exp" else 1000
    start_point_target = 1000
    ax.plot(*calculate_cdf(baseline_GPe, start_point_baseline), color='Blue', lw=1, ls='--', alpha=0.4,
            label=f"GPe-"+label_baseline)
    ax.plot(*calculate_cdf(baseline_Str, start_point_baseline), color='Orange', lw=1, ls='--', alpha=0.4,
            label=f"Str-"+label_baseline)
    ax.plot(*calculate_cdf(target_GPe, start_point_target), color='Blue', lw=1, alpha=0.4,
            label=f"GPe-"+label_target)
    ax.plot(*calculate_cdf(target_Str, start_point_target), color='Orange', lw=1, alpha=0.4,
            label=f"Str-"+label_target)
    ax.legend(frameon=False)
    ax.spines[['right', 'top']].set_visible(False)
    ax.set_xticks((1000, 1020, 1040), ("0", "20", "40"))
    ax.set_yticks((0, 0.2, 0.4, 0.6, 0.8), ("0", "20", "40", "60", "80"))
    ax.set_xlabel("Time after Stim (ms)")
    ax.set_ylabel("% cell recovered")
    ax.set_xlim(1000, 1050)
    ax.set_ylim(0, 0.8)
    if ax_title is not None:
      ax.set_title(ax_title)


def plot_result_comparison(save_name, target, baseline="naive"):
    if baseline == "naive":
        baseline_GPe = xlsx_reader(path.join('bio_data', 'OnePulse.xlsx'), "GPe naive")
        baseline_Str = xlsx_reader(path.join('bio_data', 'OnePulse.xlsx'), "D1 naive")
    else:
        baseline_GPe = xlsx_reader(path.join('simulation_result', f'{baseline}.xlsx'), "GPe")
        baseline_Str = xlsx_reader(path.join('simulation_result', f'{baseline}.xlsx'), "Str")
    target_GPe = xlsx_reader(path.join('simulation_result', f'{target}.xlsx'), "GPe")
    target_Str = xlsx_reader(path.join('simulation_result', f'{target}.xlsx'), "Str")

    fig, axs = plt.subplots(2, 4, gridspec_kw={'width_ratios': [1, 1, 1.5, 3]}, figsize=(10, 4))
    fig.subplots_adjust(wspace=0.5, hspace=0.5)
    plot_raster(axs[0, 0], baseline_GPe, "blue",
                ("Experiment recording" if baseline == "naive" else baseline) + ": GPe stim")
    plot_raster(axs[0, 1], baseline_Str, "orange",
                ("Experiment recording" if baseline == "naive" else baseline) + ": Str stim")
    plot_firing_rate(axs[0, 2], baseline_GPe, baseline_Str, "Experiment recording" if baseline == "naive" else baseline)
    plot_raster(axs[1, 0], target_GPe, "blue", target + ": GPe stim")
    plot_raster(axs[1, 1], target_Str, "orange", target + ": Str stim")
    plot_firing_rate(axs[1, 2], target_GPe, target_Str, target)

    gs = axs[0, 3].get_gridspec()
    for ax in axs[:, 3]:
        ax.remove()
    ax_compare = fig.add_subplot(gs[:, 3])
    plot_time2recover(ax_compare, baseline_GPe, baseline_Str, target_GPe, target_Str,
                      "exp" if baseline == "naive" else baseline, "sim" if baseline == "naive" else target,
                      "Time to recover")
    fig.savefig(path.join(path.dirname(__file__), "figures", f"{save_name}.jpg"),  dpi=300)
    plt.close(fig)


def plot_single(save_name, task_id):
    metrics = ("Vs", "Vd", "I_app",
               "I_HCN_som", "m_HCN_som", "g_HCN_som",
               "I_HCN_den", "m_HCN_den", "g_HCN_den",
               "I_TRPC3",
               "I_GABA_som", "g_GABA_som", "E_GABA_som", "D",
               "I_GABA_den", "g_GABA_den", "E_GABA_den", "F",
               )
    spike_times = csv_reader_trials(path.join('simulation_result', task_id, f'single.csv'))
    fig, axs = plt.subplots(len(metrics)+1, 3, sharex='col', sharey='row', figsize=(16, len(metrics)),
                            width_ratios=[2, 1, 1])

    for col_id in range(3):
        axs[0, col_id].eventplot(spike_times, )
        axs[0, col_id].spines[['right', 'top']].set_visible(False)
        axs[0, col_id].set_ylabel("Spikes")
        axs[0, col_id].axvline(x=1000, ls='--', color='gray', alpha=0.4, lw=1)

    for i, metric_name in enumerate(metrics):
        tmp_value = csv_reader_single(path.join('simulation_result', task_id, f'single_{metric_name}.csv'))
        ts = np.linspace(0, 2000, len(tmp_value))
        for col_id in range(3):
            axs[i+1, col_id].plot(ts, tmp_value)
            axs[i+1, col_id].spines[['right', 'top']].set_visible(False)
            axs[i+1, col_id].set_ylabel(metric_name, rotation=45)
            axs[i+1, col_id].axvline(x=1000, ls='--', color='gray', alpha=0.4, lw=1)

    axs[0, 1].set_xlim(800, 1200)
    axs[0, 2].set_xlim(950, 1050)
    axs[-1, 0].set_xlabel("Time [ms]")
    axs[-1, 1].set_xlabel("Time [ms]")
    axs[-1, 2].set_xlabel("Time [ms]")
    fig.savefig(path.join(path.dirname(__file__), "figures", f"{save_name}.jpg"), dpi=300)
    plt.close(fig)


def parse_args():
    parser = argparse.ArgumentParser(description="postprocess")
    parser.add_argument("--task_id", type=str, required=True)
    return parser.parse_args()


def main(task_id):
    if not result_csv2xlsx(task_id):  # check if task is a single simulation
        for hcn_pos in ("som", "den", "zero"):
            if path.exists(path.join("simulation_result", f"{task_id}_HCN_{hcn_pos}.xlsx")):
                plot_result_comparison(f"{task_id}_HCN_{hcn_pos}",
                                       f"{task_id}_HCN_{hcn_pos}", baseline="naive")
    else:
        plot_single(f"{task_id}_single", task_id)



if __name__ == "__main__":
    args = parse_args()
    main(args.task_id)