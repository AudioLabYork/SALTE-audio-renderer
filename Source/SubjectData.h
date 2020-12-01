#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct SubjectData
{
	void reset()
	{
		m_id = String();
		m_name = String();
		m_age = 0;
		m_email = String();
	}

	String m_id;
	String m_name;
	int m_age;
	String m_email;
};
