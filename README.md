# VTM_intra_CNN_LGBM_patch
### Machine Learning based Efficient QT-MTT Partitioning Scheme for VVC Intra Encoders


This patch makes the necessary changes to VTM10.2 to reproduce the results in the paper [1] .
CNN and ML models used in this contribution are provided in this project in the needed format for the encoder to work. The scripts to reproduce the training are also provided here. 

## Usage

### VTM10.2 + Complexity reduction:

 - Install frugally deep following this: https://github.com/Dobiasd/frugally-deep/blob/master/INSTALL.md
 - Download and build LightGBM 2.3.2 from Microsoft's git:
 ```sh
git clone https://github.com/microsoft/LightGBM.git
cd LightGBM/
git checkout 483a9bbad23adecf8db9b77c9f2caa69080ecf7e
mkdir build
cd build
cmake ..
make
```
- For LightGBM to work with VTM the correct directory needs to be specified in the CMakeLists.txt of the VTM project. The directory is currently set locally to the VTM directory (../LightGBM), change it if needed. 
- Build the VTM project as shown in its readme and test it out, To use the complexity reduction in intra you can either add "-pp" or add "PredictPartition: 1" in the "encoder_intra_vtm.cfg". The folder containing the models should be added as a parameter, either by using -mf [folder_path] or by adding it in the same cfg file "ModelFolder : [folder_path]".


### Training the DL model:
The file DLTraining.py can be used to train the DL model by providing the dataset folder and an output folder, dataset folder should contain 2 folders: "images_npy", each folder contains a folder named "luma", and each luma folder contains 4 folders: "22", "27", "32" and "37", each folders refers to a QP value. These QP folders would contain the data in the form of .npy files (numpy arrays saved using np.save). Files in "images_npy" QP folders are Luma CTUs (68x68) whereas files in "ground_truth_npy" QP folders are ground truth vectors of size 480. Each ground_truth should have the same name as its corresponding CTU. 
The script is used with the following arguments: [dataset folder] [output folder]
After training the model, it can be exported as a Json file using fragally deep in order to use it in the encoder, filename should be "model.json"

### Training the LightGBM model:
The file MLDataPrepAndTraining.py can be used for the data preparation and the training of the model. A csv file containing a list of the .npy files that would be used for the training should be prepared beforehead, it should contain 2 columns: "filename" and "qp".
This script is used with the following arguments:
- For data preparation: [file list csv] [dataset_path] [DL .h5 model_file] [output_folder]
- For training: [prepared data] [output folder]
- For testing models only: [prepared data] [training output folder] --test
Dataset should have the same format as the previous one, (could be the same, or a part of it...)


## Reference
<a id="1">[1]</a> 
Alexandre Tissier, Wassim Hamidouche, Souhaiel Belhadj Dit Mdalsi, Jarno Vanne, Franck Galpin and Daniel Menard.\
Machine Learning based Efficient QT-MTT Partitioning Scheme for VVC Intra Encoders
