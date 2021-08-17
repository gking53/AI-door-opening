# AI-door-opening
facial detection automatic door opener using the Nvidia isaac engine, two realsense cameras, with optional cloud connection to google cloud services

# Requirements
* Nvidia isaac 2021.1 installation (tested on Ubuntu 18.04 cuda 10.2, and Jetpack 4.5)
* 2 D435 Realsense cameras connected to the running computer
* Google Cloud Storage requires the credentials to be set in the path as in the example at https://github.com/googleapis/google-cloud-cpp/tree/main/google/cloud/storage/quickstart
* Resnet100 arcface as found https://github.com/deepinsight/insightface/tree/master/model_zoo (ONNX format)
* version-RFB-640.onnx as found https://github.com/Linzaer/Ultra-Light-Fast-Generic-Face-Detector-1MB

# Building
Some code paths are still dependant on structure, this project should be placed in `//apps/eclipseworkspace/facial`  
Build by calling `bazel build //apps/eclipseworkspace/facial:facial`  
Deploying to Jetson device (AGX Xavier recommended if using 2 cameras) can be found in the Nvidia Isaac documentation 

# TO-DO
* Clean up hardcoded deps, or point to ones that cant be changed
* Add embeddings to camera subgraph and keep track of local embedding space during runtime
* Change detector model for allignments
