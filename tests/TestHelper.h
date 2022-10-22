#pragma once

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <vector>
#include <string>
#include <fstream>

void make_test_files(const std::vector<std::string>& texts, std::vector<std::string>& acceptor);

void remove_test_files(std::vector<std::string>& filenames);