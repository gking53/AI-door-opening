load("//bzl:module.bzl", "isaac_app", "isaac_cc_module", "isaac_component", "isaac_subgraph")

isaac_cc_module(
    name = "camera_module",
    visibility = ["//visibility:public"],
    deps = [
        ":cuda_detection",
	":synchronizer",
	":recog_decoder",
    ],
)

isaac_component(
    name = "cuda_detection",
    visibility = ["//visibility:public"],
)

isaac_component(
    name = "synchronizer",
    visibility = ["//visibility:public"],
    deps = ["@com_nvidia_isaac_engine//engine/core/image"],
)

isaac_component(
    name = "recog_decoder",
    visibility = ["//visibility:public"],
    deps = [
	"@com_nvidia_isaac_engine//engine/core/tensor",
        "@com_nvidia_isaac_engine//engine/gems/tensor",
],
)

isaac_cc_module(
    name = "tracker_module",
    visibility = ["//visibility:public"],
    deps = [
        ":tracker",
	      ":tracker_object",
    ],
)

isaac_component(
    name = "tracker",
    visibility = ["//visibility:public"],
    deps = [
	":tracker_object",
	":udp_client",
    ],
)

isaac_component(
    name = "udp_client",
    visibility = ["//visibility:public"],
)


isaac_component(
    name = "tracker_object",
    visibility = ["//visibility:public"],
)


filegroup(
    name = "photo",
    srcs = glob([
        "faces.jpg",
    ]),
)

isaac_cc_module(
    name = "cloud_module",
    visibility = ["//visibility:public"],
    deps = [
        ":cloud_storage",
    ],
)

isaac_component(
    name = "cloud_storage",
    visibility = ["//visibility:public"],
    deps = ["@com_github_googleapis_google_cloud_cpp//:storage"],
)

filegroup(
    name = "facial_files",
    srcs = glob([
        "logical-signer-319519-93814db53e97.json",
	"version-RFB-640.onnx",
	"arcfaceresnet100-8.onnx",
    ]),
)

isaac_app(
    name = "facial",
    app_json_file = "facial.app.json",
    data = [
	    ":camera_subgraph",
	    ":facial_files",
    ],
    modules = [
      "viewers",
	    "//apps/eclipseworkspace/facial:tracker_module",
	    "//apps/eclipseworkspace/facial:cloud_module",
    ],
)

isaac_subgraph(
    name = "camera_subgraph",
    modules = [
        "realsense",
        "ml",
        "sight",
        "utils",
        "//apps/eclipseworkspace/facial:camera_module"
    ],
    subgraph = "camera.subgraph.json",
    visibility = ["//visibility:public"],
)

isaac_app(
    name = "test",
    data = [
    ],
    modules = [
        "viewers",
	"message_generators",
	"sight",
    ],
)

cc_binary(
    name = "quickstart",
    srcs = [
        "quickstart.cc",
    ],
    deps = [
        "@com_github_googleapis_google_cloud_cpp//:storage",
    ],
)
