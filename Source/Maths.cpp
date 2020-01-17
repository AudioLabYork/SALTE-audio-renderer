#include "Maths.h"

void mat_trans(float* outmtx, float* inmtx, int rows, int cols)
{
	int i, j;

	for (i = 0; i < rows; ++i)
	{
		for (j = 0; j < cols; ++j)
		{
			outmtx[(j * rows) + i] = inmtx[(i * cols) + j];
		}
	}
}

void mat_mult(float* out, const float* A, const float* B, int n, int m, int m2, int p)
{
	int i, j, k;
	float s;

	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < p; ++j)
		{
			s = 0.0f;

			for (k = 0; k < m; ++k)
				s += A[(i * m) + k] * B[(k * p) + j];

			out[(i * p) + j] = s;
		}
	}
}

float legendreP(const int n, const float x)
{
	if (n == 0)
		return 1.0f;
	else if (n == 1)
		return x;
	else
		return ((((2 * (n - 1)) + 1) * x * legendreP(n - 1, x)) - ((n - 1) * legendreP(n - 2, x))) / n;
}
