import os
import sys
import matplotlib.pyplot as plt
import numpy as np
import datetime
import pathlib
import pickle
import tensorflow as tf
from tensorflow import keras
from keras import layers
from sklearn.model_selection import train_test_split

from keras.layers import Input, Dense, Conv2D, MaxPooling2D, Flatten, Activation, add, Concatenate, BatchNormalization
from keras.models import Model


class DataGeneratorIntraQP(keras.utils.Sequence):
    'Generates data for Keras'

    def __init__(self, list_files, dataset, batch_size=32, dim=(68, 68), output_dim=480, n_channels=1, shuffle=True):
        'Initialization'
        self.dim = dim
        self.output_dim = output_dim
        self.batch_size = batch_size
        self.list_files = list_files
        self.n_channels = n_channels
        self.shuffle = shuffle
        self.dataset = dataset
        self.on_epoch_end()

    def __len__(self):
        'Denotes the number of batches per epoch'
        # return int(np.floor(len(self.list_files) / self.batch_size))
        return len(self.list_files) // self.batch_size

    def __getitem__(self, index):
        'Generate one batch of data'
        # Generate indexes of the batch
        indexes = self.indexes[index * self.batch_size:(index + 1) * self.batch_size]

        # Find list of IDs
        list_files_temp = [self.list_files[k] for k in indexes]

        # Generate data
        X, y = self.__data_generation(list_files_temp)

        return X, y

    def on_epoch_end(self):
        'Updates indexes after each epoch'
        self.indexes = np.arange(len(self.list_files))
        if self.shuffle == True:
            np.random.shuffle(self.indexes)

    def __data_generation(self, list_files_temp):
        'Generates data containing batch_size samples'  # X : (n_samples, *dim, n_channels)
        # Initialization
        X = np.empty((self.batch_size, self.dim[0], self.dim[1], self.n_channels))
        y = np.empty((self.batch_size, self.output_dim, 1))
        QP = np.empty((self.batch_size, (1)))

        # Generate data
        # filename is in the form of "qp/file.npy"
        for i, filename in enumerate(list_files_temp):
            # Store sample
            X[i,] = np.reshape(np.load(os.path.join(self.dataset, 'images_npy/luma', filename)),
                               (self.dim[0], self.dim[1], self.n_channels))
            # Store class
            y[i,] = np.reshape(np.load(os.path.join(self.dataset, 'ground_truth_npy/luma', filename)), (self.output_dim, 1))
            # Store QP
            QP[i,] = [int(filename.split("/")[0])]
        return [X, QP], y


def blockMV():
    qp_input = Input(shape=(1,), name="qp_input")
    luma_input = Input(shape=(128, 128, 1))
    X1_input = Input(shape=(128, 128, 1))
    X2_input = Input(shape=(128, 128, 1))
    base_model = tf.keras.applications.MobileNetV2(input_shape=(128, 128, 3), weights=None)

def conv_block(input_tensor,
               kernel_size,
               nb_filter,
               name
               ):
    x = Conv2D(nb_filter, (kernel_size, kernel_size), padding="same", activation="relu", name=name + "_conv1")(
        input_tensor)

    x = Conv2D(nb_filter, (kernel_size, kernel_size), padding="same", name=name + "_conv2")(x)

    # Compute one convolution to resize correctly the input_tensor as to compute the add operation

    input_tensor = Conv2D(nb_filter, (1, 1), padding="same")(input_tensor)

    x = add([x, input_tensor])

    x1 = Activation('relu')(x)

    x = Conv2D(nb_filter, (kernel_size, kernel_size), padding="same", activation="relu", name=name + "_conv3")(x1)

    x = Conv2D(nb_filter, (kernel_size, kernel_size), padding="same", name=name + "_conv4")(x)

    x = add([x, x1])

    x = Activation('relu')(x)

    return x


def model_technicolor_vector_multi_qp():
    img_input = Input(shape=(68, 68, 1))
    qp_input = Input(shape=(1,), name='qp_input')

    x = Conv2D(16, (3, 3), padding="same", activation="relu", name='init_conv')(img_input)

    x = MaxPooling2D((2, 2), padding="same", strides=(2, 2))(x)

    x = conv_block(x, 3, 24, name='block1')

    x = MaxPooling2D((3, 3), strides=(3, 3), padding="same")(x)

    x = conv_block(x, 3, 32, name='block2')

    x = MaxPooling2D((3, 3), strides=(3, 3), padding="same")(x)

    x = conv_block(x, 3, 48, name='block3')

    x = MaxPooling2D((3, 3), strides=(3, 3), padding="same")(x)

    x = Flatten()(x)

    x = Concatenate()([x, qp_input])

    output_luma = Dense(480, activation='sigmoid', name='fc480')(x)

    # Add QP to X here before creating the model

    model = Model(inputs=[img_input, qp_input], outputs=output_luma, name='archiGlobal')

    return model



def train_intra(dataset, output_folder):
    params = {'batch_size': 32,
              'shuffle': True,
              'dataset': dataset
              }
    pathlib.Path(output_folder).mkdir(parents=True, exist_ok=True)

    image_path = os.path.join(dataset, "images_npy", "luma")
    # load all data
    qp_list = [d for d in os.listdir(image_path) if os.path.isdir(os.path.join(image_path, d))]
    filenames = []
    for qp in qp_list:
        filenames.extend([qp + "/" + f for f in os.listdir(os.path.join(image_path, qp))])

    # Generators
    X_train, X_test, y_train, y_test = train_test_split(filenames, filenames, test_size=0.33, random_state=7)
    train_generator = DataGeneratorIntraQP(X_train, **params)
    test_generator = DataGeneratorIntraQP(X_test, **params)

    model = model_technicolor_vector_multi_qp()

    model.compile(optimizer='Adam', loss="mean_squared_error", metrics=['binary_accuracy'])
    history = model.fit(train_generator, epochs=20, validation_data=test_generator, workers=6,
                        use_multiprocessing=True)

    network_name = "{}_{}".format(str(datetime.datetime.now())[:-7].replace(" ", "_").replace('-', "").replace(":", ""),
                                  "technicolor_slide")
    pathlib.Path(os.path.join(output_folder, "models")).mkdir(parents=True, exist_ok=True)
    pathlib.Path(os.path.join(output_folder, "figs")).mkdir(parents=True, exist_ok=True)
    model.save(os.path.join(output_folder, "models", f"{network_name}.h5"))

    loss_train_valid = '_' + str(round(history.history['loss'][-1], 3)) + '_' + str(
        round(history.history['val_loss'][-1], 3))

    fig, ax = plt.subplots(1, 2, figsize=(20, 5))

    ax[0].plot(history.history['binary_accuracy'])
    ax[0].plot(history.history['val_binary_accuracy'])
    ax[0].set_title('Model accuracy')
    ax[0].set_ylabel('Accuracy')
    ax[0].set_xlabel('Epoch')
    ax[0].legend(['Train', 'Test'], loc='upper left')

    ax[1].plot(history.history['loss'])
    ax[1].plot(history.history['val_loss'])
    ax[1].set_title('Model loss')
    ax[1].set_ylabel('Loss')
    ax[1].set_xlabel('Epoch')
    ax[1].legend(['Train', 'Test'], loc='upper right')

    filename = os.path.join(output_folder, "figs", 'accloss_'+ network_name + loss_train_valid + '.jpg')
    plt.savefig(filename, dpi=300)

if __name__ == '__main__':
    usage = 'usage: [dataset folder] [output folder]'

    if len(sys.argv)!=3:
        print(usage)
        sys.exit(1)

    train_intra(sys.argv[1], sys.argv[2])