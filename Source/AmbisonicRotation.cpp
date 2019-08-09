/*	THIS CODE WAS PRETTY MUCH COPIED FROM:
	https://git.iem.at/audioplugins/IEMPluginSuite/blob/master/SceneRotator/
*/

#include "AmbisonicRotation.h"


AmbisonicRotation::AmbisonicRotation()
{
	orderMatrices.add(new Matrix<float>(0, 0)); // 0th
	orderMatricesCopy.add(new Matrix<float>(0, 0)); // 0th

	for (int l = 1; l <= 7; ++l)
	{
		const int nCh = (2 * l + 1);
		auto elem = orderMatrices.add(new Matrix<float>(nCh, nCh));
		elem->clear();
		auto elemCopy = orderMatricesCopy.add(new Matrix<float>(nCh, nCh));
		elemCopy->clear();
	}
}

AmbisonicRotation::~AmbisonicRotation()
{}

void AmbisonicRotation::process(AudioSampleBuffer& buffer)
{
	int numberOfChannels = buffer.getNumChannels();
	const int actualOrder = floor(sqrt(numberOfChannels)) - 1;


	int bufferLength = buffer.getNumSamples();

	bool newRotationMatrix = false;

	if (rotationParamsHaveChanged.get())
	{
		newRotationMatrix = true;
		calcRotationMatrix(actualOrder);
	}

	// prepare duplicated buffer
	copyBuffer.clear();
	copyBuffer.setSize(numberOfChannels, bufferLength);

	// make copy of the buffer
	for (int ch = 0; ch < numberOfChannels; ++ch)
		copyBuffer.copyFrom(ch, 0, buffer, ch, 0, bufferLength);

	// clear all channels except first
	for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
		buffer.clear(ch, 0, bufferLength);

	// rotate buffer
	for (int l = 1; l <= actualOrder; ++l)
	{
		const int offset = l * l;
		const int nCh = 2 * l + 1;
		auto R = orderMatrices[l];
		auto Rcopy = orderMatricesCopy[l];
		for (int o = 0; o < nCh; ++o)
		{
			const int chOut = offset + o;
			for (int p = 0; p < nCh; ++p)
			{
				buffer.addFromWithRamp(chOut, 0, copyBuffer.getReadPointer(offset + p), bufferLength, Rcopy->operator() (o, p), R->operator() (o, p));
			}
		}
	}

	// make copies for fading between old and new matrices
	if (newRotationMatrix)
		for (int l = 1; l <= actualOrder; ++l)
			*orderMatricesCopy[l] = *orderMatrices[l];
}

double AmbisonicRotation::P(int i, int l, int a, int b, Matrix<float>& R1, Matrix<float>& Rlm1)
{
	double ri1 = R1(i + 1, 2);
	double rim1 = R1(i + 1, 0);
	double ri0 = R1(i + 1, 1);

	if (b == -l)
		return ri1 * Rlm1(a + l - 1, 0) + rim1 * Rlm1(a + l - 1, 2 * l - 2);
	else if (b == l)
		return ri1 * Rlm1(a + l - 1, 2 * l - 2) - rim1 * Rlm1(a + l - 1, 0);
	else
		return ri0 * Rlm1(a + l - 1, b + l - 1);
};

double AmbisonicRotation::U(int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1)
{
	return P(0, l, m, n, Rone, Rlm1);
}

double AmbisonicRotation::V(int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1)
{
	if (m == 0)
	{
		auto p0 = P(1, l, 1, n, Rone, Rlm1);
		auto p1 = P(-1, l, -1, n, Rone, Rlm1);
		return p0 + p1;
	}
	else if (m > 0)
	{
		auto p0 = P(1, l, m - 1, n, Rone, Rlm1);
		if (m == 1) // d = 1;
			return p0 * sqrt(2);
		else // d = 0;
			return p0 - P(-1, l, 1 - m, n, Rone, Rlm1);
	}
	else
	{
		auto p1 = P(-1, l, -m - 1, n, Rone, Rlm1);
		if (m == -1) // d = 1;
			return p1 * sqrt(2);
		else // d = 0;
			return p1 + P(1, l, m + 1, n, Rone, Rlm1);
	}
}

double AmbisonicRotation::W(int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1)
{
	if (m > 0)
	{
		auto p0 = P(1, l, m + 1, n, Rone, Rlm1);
		auto p1 = P(-1, l, -m - 1, n, Rone, Rlm1);
		return p0 + p1;
	}
	else if (m < 0)
	{
		auto p0 = P(1, l, m - 1, n, Rone, Rlm1);
		auto p1 = P(-1, l, 1 - m, n, Rone, Rlm1);
		return p0 - p1;
	}

	return 0.0;
}


void AmbisonicRotation::calcRotationMatrix(const int order)
{
	const auto yawRadians = degreesToRadians(yaw) * -1;
	const auto pitchRadians = degreesToRadians(pitch);
	const auto rollRadians = degreesToRadians(roll);

	auto ca = std::cos(yawRadians);
	auto cb = std::cos(pitchRadians);
	auto cy = std::cos(rollRadians);

	auto sa = std::sin(yawRadians);
	auto sb = std::sin(pitchRadians);
	auto sy = std::sin(rollRadians);


	Matrix<float> rotMat(3, 3);

	if (rotationSequence == true) // roll -> pitch -> yaw (extrinsic rotations)
	{
		rotMat(0, 0) = ca * cb;
		rotMat(1, 0) = sa * cb;
		rotMat(2, 0) = -sb;

		rotMat(0, 1) = ca * sb * sy - sa * cy;
		rotMat(1, 1) = sa * sb * sy + ca * cy;
		rotMat(2, 1) = cb * sy;

		rotMat(0, 2) = ca * sb * cy + sa * sy;
		rotMat(1, 2) = sa * sb * cy - ca * sy;
		rotMat(2, 2) = cb * cy;
	}
	else // yaw -> pitch -> roll (extrinsic rotations)
	{
		rotMat(0, 0) = ca * cb;
		rotMat(1, 0) = sa * cy + ca * sb * sy;
		rotMat(2, 0) = sa * sy - ca * sb * cy;

		rotMat(0, 1) = -sa * cb;
		rotMat(1, 1) = ca * cy - sa * sb * sy;
		rotMat(2, 1) = ca * sy + sa * sb * cy;

		rotMat(0, 2) = sb;
		rotMat(1, 2) = -cb * sy;
		rotMat(2, 2) = cb * cy;
	}



	auto Rl = orderMatrices[1];

	Rl->operator() (0, 0) = rotMat(1, 1);
	Rl->operator() (0, 1) = rotMat(1, 2);
	Rl->operator() (0, 2) = rotMat(1, 0);
	Rl->operator() (1, 0) = rotMat(2, 1);
	Rl->operator() (1, 1) = rotMat(2, 2);
	Rl->operator() (1, 2) = rotMat(2, 0);
	Rl->operator() (2, 0) = rotMat(0, 1);
	Rl->operator() (2, 1) = rotMat(0, 2);
	Rl->operator() (2, 2) = rotMat(0, 0);



	for (int l = 2; l <= order; ++l)
	{
		auto Rone = orderMatrices[1];
		auto Rlm1 = orderMatrices[l - 1];
		auto Rl = orderMatrices[l];
		for (int m = -l; m <= l; ++m)
		{
			for (int n = -l; n <= l; ++n)
			{
				const int d = (m == 0) ? 1 : 0;
				double denom;
				if (abs(n) == l)
					denom = (2 * l) * (2 * l - 1);
				else
					denom = l * l - n * n;

				double u = sqrt((l * l - m * m) / denom);
				double v = sqrt((1.0 + d) * (l + abs(m) - 1.0) * (l + abs(m)) / denom) * (1.0 - 2.0 * d) * 0.5;
				double w = sqrt((l - abs(m) - 1.0) * (l - abs(m)) / denom) * (1.0 - d) * (-0.5);

				if (u != 0.0)
					u *= U(l, m, n, *Rone, *Rlm1);
				if (v != 0.0)
					v *= V(l, m, n, *Rone, *Rlm1);
				if (w != 0.0)
					w *= W(l, m, n, *Rone, *Rlm1);

				Rl->operator() (m + l, n + l) = u + v + w;
			}
		}
	}

	rotationParamsHaveChanged = false;
}

void AmbisonicRotation::updateEulerRPY(float r, float p, float y)
{
	if (roll != r || pitch != p || yaw != y)
	{
		rotationSequence = true;
		roll = r;
		pitch = p;
		yaw = y;
		rotationParamsHaveChanged = true;
	}
}

void AmbisonicRotation::updateEulerYPR(float y, float p, float r)
{
	if (yaw != y || pitch != p || roll != r)
	{
		rotationSequence = false;
		yaw = y;
		pitch = p;
		roll = r;
		rotationParamsHaveChanged = true;
	}
}