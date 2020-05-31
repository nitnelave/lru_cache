load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "lamerman_adapter",
    hdrs = ["lamerman_adapter.h"],
    deps = ["@com_github_lamerman//:cpp_lru_cache"],
)

cc_library(
    name = "mohaps_adapter",
    hdrs = ["mohaps_adapter.h"],
    deps = ["@com_github_mohaps//:lrucache11"],
)

cc_library(
    name = "vpetrigo_adapter",
    hdrs = ["vpetrigo_adapter.h"],
    deps = ["@com_github_vpetrigo//:caches"],
)

cc_library(
    name = "goldsborough_adapter",
    hdrs = ["goldsborough_adapter.h"],
    deps = ["@com_github_goldsborough//:lru-cache"],
)

cc_library(
    name = "adapters",
    hdrs = ["adapters.h"],
    deps = [
        ":lamerman_adapter",
        ":mohaps_adapter",
        ":vpetrigo_adapter",
        ":goldsborough_adapter",
    ],
)

cc_library(
    name = "catch_util",
    hdrs = ["catch_util.h"],
    deps = ["@com_github_catchorg_catch2//:catch2"],
)

cc_library(
    name = "bench_main",
    srcs = ["bench_main.cc"],
    deps = ["@com_github_catchorg_catch2//:catch2"],
    visibility = ["//benchmarks:__subpackages__"],
)

cc_test(
    name = "itoa",
    srcs = ["itoa.cc"],
    deps = [
        ":adapters",
        ":catch_util",
        "//:lru_cache",
        ":bench_main",
    ],
)

cc_test(
    name = "fibo",
    srcs = ["fibo.cc"],
    deps = [
        ":adapters",
        ":catch_util",
        "//:lru_cache",
        ":bench_main",
    ],
)
