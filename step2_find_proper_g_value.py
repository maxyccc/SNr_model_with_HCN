import numpy as np
import matplotlib.pyplot as plt
import os
import os.path as path


def get_data(filename):
    return np.fromfile(path.join(path.dirname(__file__), "intermediate_data", filename), dtype=np.float64)


def write_data(filename, data):
    data.astype(np.float64).tofile(path.join(path.dirname(__file__), "intermediate_data", filename))


def main():
    # Load data
    g = get_data('prepared_g.bin')
    I = get_data('prepared_I.bin')
    r0 = get_data('prepared_r_0.bin')
    order_I = np.argsort(I)
    print("g: ", g.shape, g[:10])
    print("I: ", I.shape, I[:10])
    print("r0: ", r0.shape, r0[:10])
    gg, II = np.meshgrid(g, I[order_I], indexing='ij')

    # # Plot I vs r0  For debug
    # plt.figure(1)
    # plt.clf()
    # plt.plot(I, r0)
    # plt.xlabel(r'$I_{app}$ (pA)')
    # plt.ylabel('FR (Hz)')
    # plt.show()

    fig = plt.figure(figsize=(6, 6))
    h = plt.axes(projection='3d')
    surf = h.plot_surface(gg, II, np.tile(r0[np.newaxis, order_I], (len(g), 1))/0.68,
                          color='gray', edgecolor='none', alpha=0.3)

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

        write_data(f"g_HCN_{keyword}.bin", g[optim_i])


    # Somatic HCN
    optimal_g_value("prepared_r_som.bin", 'som', 'viridis')
    # Dendritic HCN
    optimal_g_value("prepared_r_den.bin", 'den', "cool")

    fig.savefig(path.join(path.dirname(__file__), "figures", "g_HCN_dist.jpg"), dpi=300)

    # Blocked HCN
    write_data("g_HCN_zero.bin", np.zeros_like(I))


if __name__ == "__main__":
    main()