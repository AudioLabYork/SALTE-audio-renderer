#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

using namespace juce::dsp;


class AmbisonicRotation
{
public:
	void init();
	void process(AudioSampleBuffer& buffer);
	void calcRotationMatrix(const int order);



private:

	float* yaw;
	float* pitch;
	float* roll;
	//float* qw;
	//float* qx;
	//float* qy;
	//float* qz;
	float* invertYaw;
	float* invertPitch;
	float* invertRoll;
	//float* invertQuaternion;
	float* rotationSequence;

	//Atomic<bool> updatingParams{ false };
	Atomic<bool> rotationParamsHaveChanged{ true };

	AudioSampleBuffer copyBuffer;

	OwnedArray<Matrix<float>> orderMatrices;
	OwnedArray<Matrix<float>> orderMatricesCopy;

	double P(int i, int l, int a, int b, Matrix<float>& R1, Matrix<float>& Rlm1);
	double U(int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);
	double V(int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);
	double W(int l, int m, int n, Matrix<float>& Rone, Matrix<float>& Rlm1);

	float radiansToDegrees(float radians) { return radians * (float(180) / float(3.14159265358979323846264338327950288)); }
	float degreesToRadians(float degrees) { return degrees * (float(3.14159265358979323846264338327950288) / float(180)); }


};
