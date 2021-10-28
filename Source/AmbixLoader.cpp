#include "AmbixLoader.h"

AmbixLoader::AmbixLoader(const File& file)
{
	parseFile(file);
}

void AmbixLoader::getSourcePositions(std::vector<float>& azi, std::vector<float>& ele)
{
	azi = m_azi;
	ele = m_ele;
}

void AmbixLoader::getDecodeMatrix(std::vector<float>& decodeMatrix)
{
	decodeMatrix = m_decodeMatrix;
}

int AmbixLoader::getAmbiOrder()
{
	return m_order;
}

int AmbixLoader::getNumAmbiChans()
{
	return m_numAmbiChans;
}

int AmbixLoader::getNumLsChans()
{
	return m_numLsChans;
}

int AmbixLoader::getNumHrirs()
{
	return static_cast<int>(m_hrirs.size());
}

void AmbixLoader::getHrir(int index, AudioBuffer<float>& hrirs)
{
	hrirs = m_hrirs[index];
}

void AmbixLoader::parseFile(const File& file)
{
	FileInputStream fis(file);

	if (!fis.openedOk())
		return;

	while (!fis.isExhausted())
	{
		String line = fis.readNextLine();

		if (line.contains("#GLOBAL"))
		{
			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				if (line.startsWithIgnoreCase("/debug_msg"))
				{
					String res = line.fromFirstOccurrenceOf("/debug_msg ", false, true);
				}
				else if (line.startsWithIgnoreCase("/dec_mat_gain"))
				{
					String res = line.fromFirstOccurrenceOf("/dec_mat_gain ", false, true);
				}
				else if (line.startsWithIgnoreCase("/coeff_scale"))
				{
					String res = line.fromFirstOccurrenceOf("/coeff_scale ", false, true);
				}
				else if (line.startsWithIgnoreCase("/coeff_seq"))
				{
					String res = line.fromFirstOccurrenceOf("/coeff_seq ", false, true);
				}
				else if (line.startsWithIgnoreCase("/flip"))
				{
					String res = line.fromFirstOccurrenceOf("/flip ", false, true);
				}
				else if (line.startsWithIgnoreCase("/flop"))
				{
					String res = line.fromFirstOccurrenceOf("/flop ", false, true);
				}
				else if (line.startsWithIgnoreCase("/flap"))
				{
					String res = line.fromFirstOccurrenceOf("/flap ", false, true);
				}
				else if (line.startsWithIgnoreCase("/global_hrtf_gain"))
				{
					String res = line.fromFirstOccurrenceOf("/global_hrtf_gain ", false, true);
				}
				else if (line.startsWithIgnoreCase("/invert_condon_shortley"))
				{
					String res = line.fromFirstOccurrenceOf("/invert_condon_shortley ", false, true);
				}
			}
		}
		else if (line.contains("#HRTF"))
		{
			m_hrirs.clear();

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				parseDirection(line);

				String path = file.getParentDirectory().getFullPathName();

				File hrirFile(path + File::getSeparatorString() + line);

				if (hrirFile.existsAsFile())
				{
					AudioFormatManager formatManager;
					formatManager.registerBasicFormats();
					std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(hrirFile));

					AudioBuffer<float> inputBuffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));

					reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

					m_hrirs.push_back(inputBuffer);
				}
			}
		}
		else if (line.contains("#DECODERMATRIX"))
		{
			m_decodeMatrix.clear();
			int rows = 0;

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				juce::String::CharPointerType charptr = line.getCharPointer();
				rows++;
				int columns = 0;

				while (charptr != charptr.findTerminatingNull())
				{
					float nextval = static_cast<float>(CharacterFunctions::readDoubleValue(charptr));
					m_decodeMatrix.push_back(nextval);
					columns++;
				}
				m_numAmbiChans = columns;
				m_order = sqrt(m_numAmbiChans) - 1;
			}
			m_numLsChans = rows;
		}
	}
}

void AmbixLoader::parseDirection(const String& filename)
{
	String workingCopy = filename.replace(",", ".");

	String aziString = workingCopy.fromFirstOccurrenceOf("azi_", false, true).upToFirstOccurrenceOf("_ele", false, true);
	juce::String::CharPointerType aziCharPtr = aziString.getCharPointer();

	String eleString = workingCopy.fromFirstOccurrenceOf("ele_", false, true).upToFirstOccurrenceOf(".wav", false, true);
	juce::String::CharPointerType eleCharPtr = eleString.getCharPointer();

	m_azi.push_back(aziCharPtr.getDoubleValue());
	m_ele.push_back(eleCharPtr.getDoubleValue());
}
