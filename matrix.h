#pragma once

#include <tuple>
#include <vector>
#include "common.h"

#define BUFFER_SIZE 64
namespace {
extern std::vector<int*> ptrs;
}

std::tuple<ptr_reg, ptr_reg, ptr_reg, ptr_reg> init(int m, int n, int p);

bool correct_check(ptr_reg A, ptr_reg B, ptr_reg C, int m, int n, int p);

void destroy();