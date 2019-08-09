/*	THIS CODE WAS PRETTY MUCH COPIED FROM:
	https://git.iem.at/audioplugins/IEMPluginSuite/blob/master/SceneRotator/
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

using namespace juce::dsp;


class AmbisonicRotation
{
public:
	//void init();
	AmbisonicRotation();
	~AmbisonicRotation();

	void process(AudioSampleBuffer& buffer);
	void calcRotationMatrix(const int order);
	void updateEulerRPY(float r, float p, float y);
	void updateEulerYPR(float y, float p, float r);


private:

	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;

	bool rotationSequence = true;

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
