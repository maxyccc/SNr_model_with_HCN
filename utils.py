import pandas as pd
import os
import os.path as path
import numpy as np


def get_data(filename):
    return np.fromfile(path.join(path.dirname(__file__), "intermediate_result", filename), dtype=np.float64)


def write_data(filename, data):
    data.astype(np.float64).tofile(path.join(path.dirname(__file__), "intermediate_result", filename))


def find_corresponding_metric(value_series, key_series, query_series):
    assert len(value_series) == len(key_series)
    return np.interp(query_series, key_series, value_series)


def xlsx_reader(table_dir, sheet_name):
    data_dict = {}
    df = pd.read_excel(table_dir, sheet_name=sheet_name)
    cur_cell = None
    for index, column_name in enumerate(df.columns):
        if isinstance(column_name, str) and "Unnamed" not in column_name:
            data_dict[column_name] = []
            cur_cell = column_name
        column_array = np.array(df[column_name])
        filtered_array = column_array[~np.isnan(column_array)]
        if "naive" in sheet_name:
            filtered_array *= 1000  # s to ms
        data_dict[cur_cell].append(filtered_array)
    return data_dict


def csv_reader_trials(table_dir):
    raw_data = pd.read_csv(table_dir, header=None).to_numpy()[0]
    spike_times = []
    num_trials = 0
    cnt = 0
    while cnt < raw_data.shape[0] and raw_data[cnt] != "END":
        num_trials += 1
        spike_times.append([])
        num_spike = int(raw_data[cnt])
        cnt += 1
        for i in range(num_spike):
            spike_times[-1].append(raw_data[cnt])
            cnt += 1
    return spike_times


def csv_reader_single(table_dir):
    raw_data = pd.read_csv(table_dir, header=None).to_numpy()[0]
    values = []
    cnt = 0
    while cnt < raw_data.shape[0] and raw_data[cnt] != "END":
        values.append(raw_data[cnt])
        cnt += 1
    return values


def sync_column(data_dict: dict):
    max_len = np.max([len(value) for value in data_dict.values()])
    for key in data_dict.keys():
        data_dict[key] = data_dict[key] + [np.nan for _ in range(max_len - len(data_dict[key]))]
    return data_dict


def remove_unnamed(data_dict: dict):
    tmp_key = list(data_dict.keys())
    for i in range(len(tmp_key)):
        if "Unname" in tmp_key[i]:
            tmp_key[i] = ""
    return tmp_key


def result_csv2xlsx(task_id):
    single_flag = True
    data_dict = {
        "den": {"Str": {}, "GPe": {}, "none": {}},
        "som": {"Str": {}, "GPe": {}, "none": {}},
        "zero": {"Str": {}, "GPe": {}, "none": {}}
    }
    for filename in os.listdir(path.join("simulation_result", task_id)):
        print(f"Reading {filename}...")
        if filename.endswith(".csv") and filename.startswith("raster_"):
            single_flag = False
            cell_id = filename.split("_")[-1][:-4].zfill(2)
            if "den" in filename:
                hcn_pos = "den"
            elif "som" in filename:
                hcn_pos = "som"
            elif "zero" in filename:
                hcn_pos = "zero"
            else:
                raise NotImplementedError(f"Parser error: {filename}")
            if "Str" in filename:
                stim_pos = "Str"
            elif "GPe" in filename:
                stim_pos = "GPe"
            elif "none" in filename:
                stim_pos = "none"
            else:
                raise NotImplementedError(f"Parser error: {filename}")

            spikes_time = csv_reader_trials(path.join("simulation_result", task_id, filename))
            data_dict[hcn_pos][stim_pos][f"Cell {cell_id}"] = spikes_time[0]
            for trial_id, spike_data in enumerate(spikes_time[1:]):
                data_dict[hcn_pos][stim_pos][f"Unnamed: {len(data_dict[hcn_pos][stim_pos].keys())}"] = spike_data

    for hcn_pos in ("som", "den", "zero"):
        if len(data_dict[hcn_pos]["Str"]) + len(data_dict[hcn_pos]["GPe"]) + len(data_dict[hcn_pos]["none"]) > 0:
            with pd.ExcelWriter(path.join("simulation_result", f"{task_id}_HCN_{hcn_pos}.xlsx")) as writer:
                if len(data_dict[hcn_pos]["GPe"]) > 0:
                    pd.DataFrame(sync_column(data_dict[hcn_pos]["GPe"])).to_excel(
                        writer, sheet_name='GPe', index=False, header=remove_unnamed(data_dict[hcn_pos]["GPe"]))
                if len(data_dict[hcn_pos]["Str"]) > 0:
                    pd.DataFrame(sync_column(data_dict[hcn_pos]["Str"])).to_excel(
                        writer, sheet_name='Str', index=False, header=remove_unnamed(data_dict[hcn_pos]["Str"]))
                if len(data_dict[hcn_pos]["none"]) > 0:
                    pd.DataFrame(sync_column(data_dict[hcn_pos]["none"])).to_excel(
                        writer, sheet_name='none', index=False, header=remove_unnamed(data_dict[hcn_pos]["none"]))
            print(path.join("simulation_result", f"{task_id}_HCN_{hcn_pos}.xlsx"), " generated!")

    return single_flag
