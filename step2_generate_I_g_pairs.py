import numpy as np
import matplotlib.pyplot as plt
import os
import os.path as path
from utils import *

plt.rcParams["font.family"] = "Arial"
plt.rcParams['font.size'] = 10


def generate_I_g_pairs(required_fr):
    # Load data
    g = get_data('prepared_g.bin')
    I = get_data('prepared_I.bin')
    r0 = get_data('prepared_r_0.bin')
    order_I = np.argsort(I)
    print("g: ", g.shape, g[:10])
    print("I: ", I.shape, I[:10])
    print("r0: ", r0.shape, r0[:10])
    gg, II = np.meshgrid(g, I[order_I], indexing='ij')

    fig = plt.figure(figsize=(6, 6))
    h = plt.axes(projection='3d')
    # no HCN baseline
    h.plot_surface(gg, II, np.tile(r0[np.newaxis, order_I], (len(g), 1)),
                   color='gray', edgecolor='none', alpha=0.3)
    required_fr_zero = required_fr*0.68
    required_I = np.interp(required_fr_zero, r0, I)
    required_r = np.interp(required_fr_zero, r0, r0)
    write_data(f"selected_I_HCN_zero.bin", required_I)
    write_data(f"selected_g_HCN_zero.bin", np.zeros_like(required_I))
    write_data(f"selected_r_HCN_zero.bin", required_r)

    def optimal_g_value(file_name, keyword, cmap):
        rx = get_data(file_name)
        rx = rx.reshape((len(g), len(I)))

        surf = h.plot_surface(gg, II, rx[:, order_I], cmap=cmap, edgecolor='none', alpha=0.7, )
        cbar = fig.colorbar(surf, shrink=0.5, aspect=10)
        cbar.set_label(keyword)
        plt.xlabel(r'$g_{HCN}$ (nS/pF)')
        plt.ylabel(r'$I_{app}$ (pA)')
        plt.title(f'rate (Hz)')

        # Compute tar and find indices
        tar = r0 / 0.68
        optim_i = np.argmin(np.abs(tar[np.newaxis, :] - rx), axis=0)
        h.plot(g[optim_i][order_I], I[order_I], tar[order_I], 'k')
        # h.plot(g[optim_i][order_I], I[order_I], rx[optim_i, order_I], 'red')
        optimal_g = g[optim_i][order_I]
        optimal_I = I[order_I]
        optimal_rx = rx[optim_i, order_I]
        required_g = np.interp(required_fr, optimal_rx, optimal_g)
        required_I = np.interp(required_fr, optimal_rx, optimal_I)
        required_r = np.interp(required_fr, optimal_rx, optimal_rx)

        write_data(f"selected_g_HCN_{keyword}.bin", required_g)
        write_data(f"selected_I_HCN_{keyword}.bin", required_I)
        write_data(f"selected_r_HCN_{keyword}.bin", required_r)

    # Somatic HCN
    optimal_g_value("prepared_r_som.bin", 'som', 'viridis')
    # Dendritic HCN
    optimal_g_value("prepared_r_den.bin", 'den', "cool")
    fig.tight_layout()
    fig.savefig(path.join(path.dirname(__file__), "figures", "grid_search_result.jpg"), dpi=300)
    plt.show()


def plot_frequency_summary(required_fr):
    def compute_naive_frequency(cells_of_trials):
        total_frequency = []
        for row_id, (cell, trials) in enumerate(cells_of_trials.items()):
            total_frequency += [np.sum(trial <= 1000) for trial in trials]
        return total_frequency

    baseline_GPe = xlsx_reader(path.join('bio_data', 'OnePulse.xlsx'), "GPe naive")
    baseline_Str = xlsx_reader(path.join('bio_data', 'OnePulse.xlsx'), "D1 naive")
    naive_GPe_fr = compute_naive_frequency(baseline_GPe)
    naive_Str_fr = compute_naive_frequency(baseline_Str)
    generated_som_fr = get_data(f'selected_r_HCN_som.bin')
    generated_den_fr = get_data(f'selected_r_HCN_den.bin')
    generated_zero_fr = get_data(f'selected_r_HCN_zero.bin')

    bins = np.linspace(0, 100, 101)
    fig, axs = plt.subplots(3, 2, sharex='all', figsize=(6, 6))
    for i, (data_value, data_name) in enumerate(zip(
            [required_fr, naive_GPe_fr, naive_Str_fr, generated_som_fr, generated_den_fr, generated_zero_fr],
            ["target", "GPe_bio_data", "Str_bio_data", "sampled_som_HCN", "sampled_den_HCN", "sampled_zero_HCN"])):
        row_id, col_id = i%3, int(i/3)
        axs[row_id, col_id].hist(data_value, bins)
        axs[row_id, col_id].spines[['right', 'top']].set_visible(False)
        axs[row_id, col_id].set_title(data_name)
    axs[-1, 0].set_xlabel("baseline firing rate [hz]")
    axs[-1, 1].set_xlabel("baseline firing rate [hz]")
    fig.tight_layout()
    fig.savefig(path.join(path.dirname(__file__), "figures", "frequency_histogram.jpg"), dpi=300)
    plt.show()


def main():
    ###################################################################
    ########## TO Change: Target firing rate distribution #############
    ###################################################################
    NUM_sample = 1000
    MAX_Fr = 60  # Hz
    required_fr = np.random.normal(loc=25, scale=12.0, size=NUM_sample*5)
    required_fr = required_fr[required_fr<MAX_Fr][:NUM_sample]
    print(required_fr.shape, np.mean(required_fr), np.std(required_fr))
    generate_I_g_pairs(required_fr)
    plot_frequency_summary(required_fr)


if __name__ == "__main__":
    main()