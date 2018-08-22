#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <array>

using namespace std;


float stdDev(float data[])
{
	float sum = 0.0, mean, standardDeviation = 0.0;

	int i;

	for (i = 0; i < 10; ++i)
	{
		sum += data[i];
	}

	mean = sum / 10;

	for (i = 0; i < 10; ++i)
		standardDeviation += pow(data[i] - mean, 2);

	return sqrt(standardDeviation / 10);
}

int main()
{
	map <string, vector<pair<float, float>>> surveyData;
	vector <string> words;

	// Name of the data file we'll be reading
	string dataFileName = "WEBdatacopy.csv";

	// C style string so we can read from a string stream
	const int lineLength = 2048;
	char line[lineLength];

	// Inout file stream to read the file
	ifstream dataFile(dataFileName);
	// String stream to read each line from the file
	stringstream lineProcessor;

	// Make sure the file has opened correctly before proceeding
	if (dataFile.is_open())
	{
		// Read header line
		if (!dataFile.getline(line, lineLength))
		{
			cout << "invalid header line" << endl;
			exit(0);
		}

		// Set line in the line processor
		lineProcessor << line;

		// Process header line adding an entry for each word

		// Iterate between all commas
		while (lineProcessor.getline(line,lineLength,','))
		{
			// Check there is something between the commas
			if (line[0] != '\0')
			{
				// Add an empty vector for each word
				surveyData[line] = vector<pair<float, float>>();
			}
		}
		
		// Populate the words in correct (sorted) order
		for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
		{
			words.push_back(it->first);
		}

		// Read the remaining lines
		while (dataFile.getline(line, lineLength))
		{
			// Set line in the line processor
			lineProcessor.clear();
			lineProcessor << line;

			int counter = 0;
			float leftEndPoint = 0.f;
			float rightEndPoint = 0.f;

			// read in each pair of values
			while (lineProcessor.getline(line, lineLength, ','))
			{
				// Check there is something between the commas
				if (line[0] != '\0')
				{
					if (!(counter % 2)) // Odd number so left endpoint
					{
						leftEndPoint = atof(line);
					}
					else // Even number so right endpoint
					{
						rightEndPoint = atof(line);
						int index = counter / 2;

						// Add inteval if it is not [0,10]
						if (!(leftEndPoint == 0.f && rightEndPoint == 10.f))
						{
							surveyData[words[index]].push_back(pair<float, float>(leftEndPoint, rightEndPoint));
						}
					}
				}
				counter++;
			}
		}
		dataFile.close();
	}
	else cout << "Unable to open " << dataFileName << endl;

	// Find the mean and std for each word based on the intervals
	// Using a histogram approach so bin size is sensitive

	const int binCount = 201;
	float start = 0.f;
	float end = 10.f;
	float interval = (end - start) / (float(binCount - 1));

	// Data structure to hold histogram for each word
	map<string, array<int, binCount>> histograms;

	// Data structure to hold normalised histogram for each word
	map<string, array<float, binCount>> normalisedHistograms;

	// Data structure to hold max frequency for each word
	map<string, int> maxFreq;

	// For each word in the survey data
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		// Add empty histogram arrays
		histograms[it->first] = array<int, binCount>();
		normalisedHistograms[it->first] = array<float, binCount>();
		// Add zero max freq
		maxFreq[it->first] = 0;

		// Go through each bin value
		float currentValue = start;
		for (int i = 0; i < binCount; i++)
		{
			// and for each data interval add 1 to histogram if the cuurent value is inside it
			for (int j = 0; j < it->second.size(); j++)
			{
				if (it->second[j].first <= currentValue && currentValue <= it->second[j].second)
				{
					// Add one to the histogram
					histograms[it->first].at(i)++;
					// finally check if we need to increase max frequency
					if (histograms[it->first].at(i) > maxFreq[it->first])
					{
						maxFreq[it->first] = histograms[it->first].at(i);
					}
				}
			}
			currentValue += interval;
		}

		// Normalise histogram
		for (int i = 0; i < binCount; i++)
		{
			normalisedHistograms[it->first].at(i) = (float)histograms[it->first].at(i) / (float)maxFreq[it->first];
		}
	}

	// Now calculate the mean and standard deviation for each histogram

	// Data structure to hold mean and stDev for each word
	map<string, pair<float, float>> meanAndStDev;

	// For each word in the survey data
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
				// Calculate mean

		float mean = 0.f;
		float sum = 0.f;
		float count = 0.f;

		float currentValue = start;
		for (int i = 0; i < binCount; i++)
		{
			sum += histograms[it->first].at(i) * currentValue; 
			count += histograms[it->first].at(i);
			currentValue += interval;
		}
		mean = sum / count;

		currentValue = start;
		count = 0.f;
		float stdDev = 0;
	
		for (int i = 0; i < binCount; i++)
		{
			stdDev += powf(histograms[it->first].at(i) * currentValue - mean, 2.0f);
			count += histograms[it->first].at(i);
			currentValue += interval;
		}

		stdDev = sqrt(stdDev / (count - 1.0f));
		meanAndStDev[it->first] = pair<float, float>(mean, stdDev);
	}

	// Write results out to file
	ofstream out("histograms.csv");

	// Write out data header
	out << ",";
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		out << it->first << ",";
	}
	out << endl;

	// Write out histogram data
	float currentValue = start;
	for (int i = 0; i < binCount; i++)
	{
		out << currentValue << ",";
		for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
		{
			out << normalisedHistograms[it->first].at(i) << ",";
		}
		out << endl;
		currentValue += interval;
	}
	
	out.close();

}