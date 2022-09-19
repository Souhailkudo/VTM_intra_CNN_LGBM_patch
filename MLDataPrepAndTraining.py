import sys
import pathlib
import numpy as np
import pandas as pd
import os
from tqdm import tqdm
import matplotlib.pyplot as plt
from datetime import datetime
from joblib import dump, load
# from keras.models import load_model
from tensorflow.keras.models import load_model
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, f1_score, mean_squared_error, roc_curve
from sklearn.utils import shuffle

import lightgbm as lgb
from sklearn.preprocessing import LabelEncoder


# %% partition checks
def is_QT(m):
    x, y = m.shape
    return x + 1 == np.sum(m[:, x // 2]) + np.sum(m[x // 2, :])


def is_BTH(m):
    x, y = m.shape
    return (y + 1) // 2 == np.sum(m[x // 2, :])


def is_BTV(m):
    x, y = m.shape
    return (x + 1) // 2 == np.sum(m[:, y // 2])


def is_TTV(m):
    x, y = m.shape
    return x + 1 == np.sum(m[:, y // 4]) + np.sum(m[:, y - 1 - y // 4])


def is_TTH(m):
    x, y = m.shape
    return y + 1 == np.sum(m[x // 4, :]) + np.sum(m[x - 1 - x // 4, :])


def QT_proba(v):
    h1 = np.mean(v[217:232:2])
    h2 = np.mean(v[233:248:2])
    v1 = np.mean(v[15:233:31])
    v2_indexes = [263, 294, 325, 356, 387, 418, 449, 472]
    v2 = np.mean(np.array([v[i] for i in v2_indexes]))
    result = np.min([h1, h2, v1, v2])
    # print(result)
    return result


def extract_indices(size="32x32"):
    if size == "32x32":
        mat = np.zeros((15, 15), dtype=int) - 1
        count = 1
        coin = False
        for i in range(15):
            for j in range(15):
                if coin:
                    mat[i, j] += count
                    count += 1
                    coin = False
                else:
                    coin = True
        # test = pd.DataFrame(mat)
        indices = {
            'BTH1': [int(i) for i in mat[7, :7] if i != -1],
            'BTH2': [int(i) for i in mat[7, 8:] if i != -1],
            'BTV1': [int(i) for i in mat[:7, 7] if i != -1],
            'BTV2': [int(i) for i in mat[8:, 7] if i != -1],
            'TTH1': [int(i) for i in mat[3, :] if i != -1],
            'TTH2': [int(i) for i in mat[11, :] if i != -1],
            'TTV1': [int(i) for i in mat[:, 3] if i != -1],
            'TTV2': [int(i) for i in mat[:, 11] if i != -1]
        }
        return indices

    if size == "16x16":
        mat = np.zeros((7, 7), dtype=int) - 1
        count = 1
        coin = False
        for i in range(7):
            for j in range(7):
                if coin:
                    mat[i, j] += count
                    count += 1
                    coin = False
                else:
                    coin = True
        # test = pd.DataFrame(mat)
        indices = {
            'BTH1': [int(i) for i in mat[3, :3] if i != -1],
            'BTH2': [int(i) for i in mat[3, 4:] if i != -1],
            'BTV1': [int(i) for i in mat[:3, 3] if i != -1],
            'BTV2': [int(i) for i in mat[4:, 3] if i != -1],
            'TTH1': [int(i) for i in mat[1, :] if i != -1],
            'TTH2': [int(i) for i in mat[5, :] if i != -1],
            'TTV1': [int(i) for i in mat[:, 1] if i != -1],
            'TTV2': [int(i) for i in mat[:, 5] if i != -1]
        }
        return indices

    if size == "16x32":
        mat = np.zeros((7, 15), dtype=int) - 1
        count = 1
        coin = False
        for i in range(7):
            for j in range(15):
                if coin:
                    mat[i, j] += count
                    count += 1
                    coin = False
                else:
                    coin = True
        # test = pd.DataFrame(mat)
        indices = {
            'BTH1': [int(i) for i in mat[3, :7] if i != -1],
            'BTH2': [int(i) for i in mat[3, 8:] if i != -1],
            'BTV1': [int(i) for i in mat[:3, 7] if i != -1],
            'BTV2': [int(i) for i in mat[4:, 7] if i != -1],
            'TTH1': [int(i) for i in mat[1, :] if i != -1],
            'TTH2': [int(i) for i in mat[5, :] if i != -1],
            'TTV1': [int(i) for i in mat[:, 3] if i != -1],
            'TTV2': [int(i) for i in mat[:, 11] if i != -1]
        }
        return indices

    if size == "32x16":
        mat = np.zeros((15, 7), dtype=int) - 1
        count = 1
        coin = False
        for i in range(15):
            for j in range(7):
                if coin:
                    mat[i, j] += count
                    count += 1
                    coin = False
                else:
                    coin = True
        # test = pd.DataFrame(mat)
        indices = {
            'BTH1': [int(i) for i in mat[7, :3] if i != -1],
            'BTH2': [int(i) for i in mat[7, 4:] if i != -1],
            'BTV1': [int(i) for i in mat[:7, 3] if i != -1],
            'BTV2': [int(i) for i in mat[8:, 3] if i != -1],
            'TTH1': [int(i) for i in mat[3, :] if i != -1],
            'TTH2': [int(i) for i in mat[11, :] if i != -1],
            'TTV1': [int(i) for i in mat[:, 1] if i != -1],
            'TTV2': [int(i) for i in mat[:, 5] if i != -1]
        }
        return indices


def check_class(m, QT=True, BTH=True, BTV=True, TTH=True, TTV=True):
    if QT:
        if is_QT(m):
            return "QT"
    if BTH:
        if is_BTH(m):
            return "BTH"
    if BTV:
        if is_BTV(m):
            return "BTV"
    if TTH:
        if is_TTH(m):
            return "TTH"
    if TTV:
        if is_TTV(m):
            return "TTV"
    return "NS"


# %% mat&vect
def matrix_to_vector(matrix):
    vector = [0 for _ in range(480)]
    val = 1
    for i in range(len(matrix)):
        if i % 2 == 1:
            val = val - 31
        elif i == 30:
            val = val - 1
        for j in range(len(matrix[i])):
            if i % 2 == 0 and j % 2 == 1:
                vector[val] = matrix[i][j]
                if i == 30:
                    val = val + 1
                else:
                    val = val + 2
            elif i % 2 == 1 and j % 2 == 0:
                vector[val] = matrix[i][j]
                val = val + 2
    return vector


def vector_to_matrix(vector):
    matrix = [[0] * 31 for _ in range(31)]
    # vector = vector.split(" ")[:480]
    x = 0
    y = 0
    hor_vert = 0
    for i in range(480):
        if i % 31 == 0 and i != 0:
            x = x + 2
            y = 0
            if hor_vert == 0:
                hor_vert = 1
            else:
                hor_vert = 0

        if i % 2 == 1 and hor_vert == 0 or i % 2 == 0 and hor_vert == 1 or i > 464:
            matrix[x][y] = 0
            matrix[x][y + 1] = vector[i]
        else:
            matrix[x + 1][y] = vector[i]
            if y % 30 != 0 or y == 0:
                matrix[x + 1][y + 1] = 0
        if i % 2 == 1 and hor_vert == 0 or i % 2 == 0 and hor_vert == 1 or i > 464:
            y = y + 2
    return np.array(matrix)


def extract_from_vector(v, x, y, w, h):
    start = y + 31 * (x // 2)
    subv = []
    for i in range(h // 2):
        for j in range(w):
            subv.append(v[start])
            start += 1
        start += 31 - w

    if x + h == 31:
        start = 465 + y // 2
        for i in range(w // 2):
            subv.append(v[start])
            start += 1
    else:
        start += 1
        for i in range(w // 2):
            subv.append(v[start])
            start += 2
    return list(subv)


## For not filtered data
def apply_network(csv_file):
    df = pd.read_csv(csv_file)
    df = df.head(60000)
    path = "/home/sbelhadj/allData/"
    model = load_model(
        '/home/sbelhadj/workspaces/cworkspace/Alex_stuff/model_cnn/intra/my_model_tech_0.065_0.66.h5')
    print("loading x...")
    x = []
    for i in tqdm(df[['file_name', 'qp']].values):
        x.append(np.load(path + "smaller_images_npy/luma/" + str(i[1]) + "/" + i[0]) / 255)
    print("loading qps...")
    qps = df['qp'].values
    print("prediction...")
    prediction = model.predict([x, qps])
    print('making dataframe...')
    prediction_df = pd.DataFrame(prediction)
    result = pd.concat([df, prediction_df], axis=1)
    result.to_csv(csv_file[:-4] + "_withpred.csv")
    print('all done')


def whtosize(w, h):
    return str((h + 1) * 2) + 'x' + str((w + 1) * 2)


def sizetowh(size):
    sizes = size.split('x')
    return int(size[1]) // 2 - 1, int(size[0]) // 2 - 1


def savetodsf(dfs, qp, split, v, x, y, w, h):
    size = whtosize(w, h)
    dfs[size] = dfs[size].append(pd.DataFrame([[qp, split] + extract_from_vector(v, x, y, w, h)],
                                              columns=['qp', 'split'] + [str(i) for i in range((w * h) // 2)]),
                                 ignore_index=True)
    if len(dfs[size]) >= 10:
        directory = dfs['dir'] + '/' + size
        if not os.path.isdir(directory):
            os.mkdir(directory)
        dfs[size].to_csv(directory + '/' + str(len(os.listdir(directory))) + '.csv')
        dfs[size] = pd.DataFrame(columns=['qp', 'split'] + [str(i) for i in range((w * h) // 2)])


def data_prep_from_vect(dfs, qp, v, gt, x, y, w, h):
    if h == 31:   # 64x64
        if is_QT(gt):
            savetodsf(dfs, qp, "QT", v, x, y, w, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y, w // 2, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x, y + w // 2 + 1, w // 2, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x + h // 2 + 1, y, w // 2, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x + h // 2 + 1, y + w // 2 + 1, w // 2, h // 2)
            return
        else:
            savetodsf(dfs, qp, "NS", v, x, y, w, h)
            return

    if h == w and h > 3:    # 32x32 16x16
        if is_QT(gt[x:x + h, y:y + w]):
            savetodsf(dfs, qp, "QT", v, x, y, w, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y, w // 2, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x, y + w // 2 + 1, w // 2, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x + h // 2 + 1, y, w // 2, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x + h // 2 + 1, y + w // 2 + 1, w // 2, h // 2)
            return

    if h > 3:      #  h=16 or h=32
        if is_TTH(gt[x:x + h, y:y + w]):
            savetodsf(dfs, qp, "TTH", v, x, y, w, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y, w, h // 4)
            data_prep_from_vect(dfs, qp, v, gt, x + h // 4 + 1, y, w, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x + h - h // 4, y, w, h // 4)
            return

    if w > 3:     # w=16 or w=32
        if is_TTV(gt[x:x + h, y:y + w]):
            savetodsf(dfs, qp, "TTV", v, x, y, w, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y, w // 4, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y + w // 4 + 1, w // 2, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y + w - w // 4, w // 4, h)
            return

    if h > 1:    # h= 8,16,32
        if is_BTH(gt[x:x + h, y:y + w]):
            savetodsf(dfs, qp, "BTH", v, x, y, w, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y, w, h // 2)
            data_prep_from_vect(dfs, qp, v, gt, x + h // 2 + 1, y, w, h // 2)
            return

    if w > 1:  # w=8,16,32
        if is_BTV(gt[x:x + h, y:y + w]):
            savetodsf(dfs, qp, "BTV", v, x, y, w, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y, w // 2, h)
            data_prep_from_vect(dfs, qp, v, gt, x, y + w // 2 + 1, w // 2, h)
            return
    if h > 1 or w > 1:  # h>4 or w>4
        savetodsf(dfs, qp, "NS", v, x, y, w, h)
    return


def main_data_prep(dir, file_list_csv, dataset_path, model_file):
    pathlib.Path(dir).mkdir(parents=True, exist_ok=True)

    dfs = {}
    dfs["dir"] = dir
    whs = [1, 3, 7, 15, 31]
    for w in whs:
        for h in whs:
            if w != 1 or h != 1:
                dfs[whtosize(w, h)] = pd.DataFrame(columns=['qp', 'split'] + [str(i) for i in range((w * h) // 2)])

    df = pd.read_csv(file_list_csv)
    shuffle(df)
    df = df[["file_name", 'qp']]
    model = load_model(model_file)
    # test_values = [["4014562.npy", 22]]
    for i in tqdm(df.values):
    # for i in test_values:
        qp = i[1]
        x = np.load(dataset_path + "smaller_images_npy/luma/" + str(qp) + "/" + i[0]) / 255
        x = x.reshape((1, 68, 68, 1)).astype('float32')
        gt = np.load(dataset_path + "smaller_ground_truth_matrix_npy/luma/" + str(qp) + "/" + i[0])
        prediction = model.predict([x, np.array([qp]).reshape(1, 1)])
        v = prediction[0]
        data_prep_from_vect(dfs, qp, v, gt, 0, 0, 31, 31)

    for w in whs:
        for h in whs:
            if w != 1 or h != 1:
                size = whtosize(w, h)
                if len(dfs[size]) > 0:
                    directory = dfs['dir'] + '/' + size
                    if not os.path.isdir(directory):
                        os.mkdir(directory)
                    dfs[size].to_csv(directory + '/' + str(len(os.listdir(directory))) + '.csv')

    print("DONE")


def general_train(dir, df_dir, test_only=True):
    if not os.path.isdir(dir):
        os.mkdir(dir)
    if not os.path.isdir(dir+"/joblib"):
        os.mkdir(dir+"/joblib")
    if not os.path.isdir(dir+"/txtmodel"):
        os.mkdir(dir+"/txtmodel")
    if not os.path.isdir(dir + "/Le"):
            os.mkdir(dir + "/Le")

    sizes = os.listdir(df_dir)
    # sizes = ["64x64", "32x32", "4x8", "8x16"]


    whole_test_data = []
    for size in sizes:
        print(size)
        small_test_data=[size]
        list_df = os.listdir(df_dir+"/" + size)
        df = pd.DataFrame()
        for csv_file in tqdm(list_df):
            df = df.append(pd.read_csv(df_dir+"/" + size + "/" + csv_file), ignore_index=True)
        y = df["split"]
        if test_only:
            le = load(dir + "/Le/Le_" + size)
        else:
            le = LabelEncoder()
            le.fit(y)
            dump(le, dir + "/Le/Le_" + size)
        class_number = len(le.classes_)
        small_test_data.append(class_number)
        y = le.transform(y)
        X = df.drop(columns=["Unnamed: 0", "split"])
        X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.33, random_state=0)
        train_data = lgb.Dataset(X_train, label=y_train)
        test_data = lgb.Dataset(X_test, label=y_test)
        if test_only:
            model = load(dir+"/joblib/lgbm_"+size)
        else:
            if class_number > 2:
                lgbm_params = {
                    'task': 'train',
                    'objective': 'multiclass', "num_class": class_number,
                    'metric': ['multi_logloss'],
                    'boosting': 'gbdt',
                    'verbose': 0
                }
            else:
                lgbm_params = {
                    'task': 'train',
                    'objective': 'binary',
                    'metric': ['rmse'],
                    # 'is_unbalance': 'true',
                    'boosting': 'gbdt',
                    'verbose': 0
                }
            model = lgb.train(lgbm_params,
                              train_data,
                              valid_sets=test_data,
                              num_boost_round=100000,
                              early_stopping_rounds=1500,
                              verbose_eval=1
                              )
            dump(model, dir+"/joblib/lgbm_"+size)
            model.save_model(dir+"/txtmodel/lgbm_" + size + '.txt', num_iteration=model.best_iteration)

        # test

        print("ML prediction: ", end=' ')
        prediction_time = datetime.now()
        predictions = model.predict(X_test)
        prediction_time = datetime.now() - prediction_time
        print(prediction_time)


        print("accuracy: ", end='')
        if class_number == 2:
            y_pred = (predictions > 0.5).astype(int)
        else:
            y_pred = [np.argmax(prediction) for prediction in predictions]
        acc = accuracy_score(y_test, y_pred)
        print(acc)
        small_test_data.append(acc)
        # general_roc(y_test, y_pred)

        if class_number>2:
            print("TOP2 accuracy: ", end='')
            trues = 0
            for i in range(len(y_test)):
                pred = np.argpartition(predictions[i], -2)[-2:]
                if y_test[i] in pred:
                    trues += 1
            acc2 = trues / len(y_test)
            print(acc2)
        else:
            acc2 = -1
        small_test_data.append(acc2)

        if class_number>3:
            print("TOP3 accuracy: ", end='')
            trues = 0
            for i in range(len(y_test)):
                pred = np.argpartition(predictions[i], -3)[-3:]
                if y_test[i] in pred:
                    trues += 1
            acc3 = trues / len(y_test)
            print(acc3)
        else:
            acc3 = -1
        small_test_data.append(acc3)
        whole_test_data.append(small_test_data)
    test_df = pd.DataFrame(whole_test_data, columns=["size", "class_number", "accuracy", "TOP2 accuracy", "TOP3 accuracy"])
    test_df.to_csv(dir+"/test.csv", index=False)


if __name__ == '__main__':

    usage = 'usage: [dataset folder] [output folder]'
    usage = """For data preparation: [file list csv] [dataset_path] [model_file] [output_folder]
    For training: [prepared data] [output folder]
    For testing models only: [prepared data] [training output folder] --test
    """

    if len(sys.argv) not in [3, 4, 5]:
        print(usage)
        sys.exit(1)
    if len(sys.argv)==5:
        main_data_prep(sys.argv[4], sys.argv[1], sys.argv[2], sys.argv[3])

    if len(sys.argv)==3:
        general_train(sys.argv[2], sys.argv[1], test_only=False)
    else:
        if sys.argv[3]=="--test":
            general_train(sys.argv[2], sys.argv[1], test_only=True)
        else:
            print(usage)
            sys.exit(1)

