#pragma once

void mat_trans(float* outmtx, float* inmtx, int rows, int cols);
void mat_mult(float* out, const float* A, const float* B, int n, int m, int m2, int p);

float legendreP(const int n, const float x);
