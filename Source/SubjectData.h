#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

extern StringArray strGenderOptions;

struct SubjectData
{
	enum eGenderOptions
	{
		kMale,
		kFemale,
		kNonBinary,
		kPreferNotToSay
	};

	void reset()
	{
		m_id = 0;
		m_name = String();
		m_age = 0;
		m_gender = String();
	}

	int m_id;
	String m_name;
	int m_age;
	String m_gender;
};
