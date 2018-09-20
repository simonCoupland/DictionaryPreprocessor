#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <array>
#include <algorithm>

using namespace std;

/*
Implements the data cleaning approach
Simon Coupland, J. M. Mendel and Dongrui Wu, “Enhanced Interval Approach for Encoding Words into
Interval Type-2 Fuzzy Sets and Convergence of the Word FOUs,” IEEE World Congress on Computational
Intelligence, Barcelona, Spain, July 2010.

Based on Dongrui Wu's matlab implementation

Creates histograms of remaining intervals and fits Gausian type-1 MF to these.

Simon Coupland

See GITHub for dates
*/

float stdDev(vector<float> data, float & mean)
{
	float sum = 0.0, standardDeviation = 0.0;

	for (int i = 0; i < data.size(); i++)
	{
		sum += data[i];
	}

	mean = sum / (float)data.size();

	for (int i = 0; i < data.size(); i++)
	{
		standardDeviation += powf(data[i] - mean, 2.0f);
	}

	return sqrt(standardDeviation / (float)data.size());
}

vector<pair<float, float>> dataClean(vector<pair<float, float>> intervals)
{
	// Outlier processing
	vector<float> L, R, intLeng;
	
	for (int i = 0; i < intervals.size(); i++)
	{
		L.push_back(intervals[i].first);
		R.push_back(intervals[i].second);
		intLeng.push_back(R[i] - L[i]);
	}

	sort(L.begin(), L.end());
	sort(R.begin(), R.end());
	sort(intLeng.begin(), intLeng.end());

	int nums = intervals.size();
	int NN1 = floor((float)nums * 0.25f);
	int NN2 = floor((float)nums * 0.75f);

	// Compute Q(0.25), Q(0.75) and IQR for left - ends
	float QL25, QL75, LIQR;
	QL25 = L[NN1];
	QL75 = L[NN2];
	LIQR = QL75 - QL25;

	// Compute Q(0.25), Q(0.75) and IQR for right - ends.
	float QR25, QR75, RIQR;
	QR25 = R[NN1];
	QR75 = R[NN2];
	RIQR = QR75 - QR25;
	
	// outlier processing for L and R
	for (int i = 0; i < intervals.size(); )
	{
		if (L[i] < QL25 - 1.5f * LIQR || L[i] > QL75 + 1.5f * LIQR || R[i] < QR25 - 1.5f * RIQR || R[i] > QR75 + 1.5f * RIQR)
		{
			// Delete interval & L, R and intLeng
			intervals.erase(intervals.begin() + i);
			L.erase(L.begin() + i);
			R.erase(R.begin() + i);
			intLeng.erase(intLeng.begin() + i);
		}
		else
		{
			i++;
		}
	}

	// Compute Q(0.25), Q(0.75) and IQR for interval length.
	int n1 = intervals.size();
	NN1 = floor((float)n1 * 0.25f);
	NN2 = floor((float)n1 * 0.75f);
	
	float QLeng25 = intLeng[NN1] + intLeng[NN1 + 1];
	float QLeng75 = intLeng[NN2] + intLeng[NN2 + 1];
	float lengIQR = QLeng75 - QLeng25;

	// outlier processing for interval length
	for (int i = 0; i < intervals.size(); )
	{
		if (intLeng[i] < QLeng25 - 1.5f * lengIQR || intLeng[i] > QLeng75 * 1.5f * lengIQR)
		{
			// Delete interval & L, R and intLeng
			intervals.erase(intervals.begin() + i);
			L.erase(L.begin() + i);
			R.erase(R.begin() + i);
			intLeng.erase(intLeng.begin() + i);
		}
		else
		{
			i++;
		}
	}

	//Tolerance limit processing, see Equation(3) in paper
	float meanL;
	float stdL = stdDev(L, meanL);
	float meanR;
	float stdR = stdDev(R, meanR);

	float K[25] = { 32.019, 32.019, 8.380, 5.369, 4.275, 3.712, 3.369, 3.136, 2.967, 2.839, 2.737, 2.655, 2.587, 2.529, 2.48, 2.437, 2.4, 2.366, 2.337, 2.31, 2.31, 2.31, 2.31, 2.31, 2.208 };
	float k = K[min((int)intervals.size()-1, 24)];

	//Tolerance limit processing for L and R
	for (int i = 0; i < intervals.size(); )
	{
		if (L[i] < meanL - k * stdL || L[i] > meanL + k * stdL || R[i] < meanR - k * stdR || R[i] > meanR + k  * stdR)
		{
			// Delete interval & L, R and intLeng
			intervals.erase(intervals.begin() + i);
			L.erase(L.begin() + i);
			R.erase(R.begin() + i);
			intLeng.erase(intLeng.begin() + i);
		}
		else
		{
			i++;
		}
	}

	// Tolerance limit processing for interval length
	float meanLeng;
	float stdLeng = stdDev(intLeng, meanLeng);

	k = min(min(K[min((int)intervals.size() - 1, 24)], meanLeng / stdLeng), (10.f - meanLeng) / stdLeng);
	for (int i = 0; i < intervals.size(); )
	{
		if (intLeng[i] < meanLeng - k * stdLeng || intLeng[i] > meanLeng + k * stdLeng)
		{
			// Delete interval & L, R and intLeng
			intervals.erase(intervals.begin() + i);
			L.erase(L.begin() + i);
			R.erase(R.begin() + i);
			intLeng.erase(intLeng.begin() + i);
		}
		else
		{
			i++;
		}
	}

	// Reasonable interval processing, see Equation(4) - (6) in paper
	stdL = stdDev(L, meanL);
	stdR = stdDev(R, meanR);

	// Determine sigma*
	float barrier;

	if (stdL == stdR)
		barrier = (meanL + meanR) / 2.0f;
	else if (stdL == 0)
		barrier = meanL + 0.01f;
	else if (stdR == 0)
		barrier = meanR - 0.01f;
	else
	{
		float barrier1, barrier2;
		float stdL2 = powf(stdL, 2.0f);
		float stdR2 = powf(stdR, 2.0f);

		barrier1 = (meanR * stdL2 - meanL * stdR2 + stdL * stdR * sqrtf(powf((meanL - meanR), 2.0f) + 2.0f * (stdL2 - stdR2) * log(stdL / stdR))) / (stdL2 - stdR2);
		barrier2 = (meanR * stdL2 - meanL * stdR2 - stdL * stdR * sqrtf(powf((meanL - meanR), 2.0f) + 2.0f * (stdL2 - stdR2) * log(stdL / stdR))) / (stdL2 - stdR2);
		if  (barrier1 >= meanL && barrier1 <= meanR)
			barrier = barrier1;
		else
			barrier = barrier2;
	}
	// Reasonable interval processing
	for (int i = 0; i < intervals.size(); )
	{
		if (L[i] >= barrier || R[i] <= barrier || L[i] < 2.0f * meanL - barrier || R[i] > 2.0f * meanR - barrier)
		{
			// Delete interval & L, R and intLeng
			intervals.erase(intervals.begin() + i);
			L.erase(L.begin() + i);
			R.erase(R.begin() + i);
			intLeng.erase(intLeng.begin() + i);
		}
		else
		{
			i++;
		}
	}


	return intervals;
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

	// Input file stream to read the file
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
				// Populate the words in correct (unsorted) order
				words.push_back(line);
			}
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
						// Bad data processing, see Equation (1) in paper
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
	else 
	{
		cout << "Unable to open " << dataFileName << endl; 
		exit(0);
	}

	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		it->second = dataClean(it->second);
	}

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
				if (i == binCount - 1)
				{
					currentValue = 10.f; // FP number fix
				}
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

	// Now calculate the MOM and standard deviation for each histogram

	// Data structure to hold mean and stDev for each word
	map<string, pair<float, float>> momAndStDev;

	// For each word in the survey data
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		// Calculate max

		float max = -1.0f;
		float currentValue = start;
		for (int i = 0; i < binCount; i++)
		{
			if (normalisedHistograms[it->first].at(i) > max) max = normalisedHistograms[it->first].at(i);
			currentValue += interval;
		}

		// Calculate MOM
		float mom = 0.f;
		float sum = 0.f;
		float count = 0.f;

		currentValue = start;
		for (int i = 0; i < binCount; i++)
		{
			if (normalisedHistograms[it->first].at(i) == max)
			{
				sum += currentValue;
				count++;
			}
			currentValue += interval;
		}
		mom = sum / count;

		//Calculate StDev
		currentValue = start;
		count = 0.f;
		float stdDev = 0;
	
		for (int i = 0; i < binCount; i++)
		{
			stdDev += normalisedHistograms[it->first].at(i) * powf(currentValue - mom, 2.0f);
			count++;
			currentValue += interval;
		}

		stdDev = sqrt(stdDev / (count - 1.0f));
		momAndStDev[it->first] = pair<float, float>(mom, stdDev);
	}

	// Write results out to file
	
	ofstream cleanDataFile("cleanedIntervals.csv");

	// Write out data header
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		cleanDataFile << it->first << ",,";
	}
	cleanDataFile << endl;

	// Write mean and Std Dev
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		cleanDataFile << "C," << momAndStDev[it->first].first << ",";
	}
	cleanDataFile << endl;
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		cleanDataFile << "sigma," << momAndStDev[it->first].second << ",";
	}
	cleanDataFile << endl;
	cleanDataFile << endl;

	cleanDataFile << "Cleaned intervals" << endl;
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		cleanDataFile << it->first << ",,";
	}cleanDataFile << endl;

	// Find max number of intervals
	int maxSize = -1;
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it) maxSize = max(maxSize, (int)it->second.size());

	// Write intervals to file
	for (int i = 0; i < maxSize; i++)
	{
		for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
		{
			if (i < it->second.size())
				cleanDataFile << it->second[i].first << "," << it->second[i].second << ",";
			else 
				cleanDataFile << ",,";
		}
		cleanDataFile << "\n";
	}

	cleanDataFile.close();

	ofstream histoFile("histograms.csv");

	// Write out data header
	histoFile << ",";
	for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
	{
		histoFile << it->first << ",";
	}
	histoFile << endl;

	// Write out histogram data
	float currentValue = start;
	for (int i = 0; i < binCount; i++)
	{
		histoFile << currentValue << ",";
		for (auto it = surveyData.begin(); it != surveyData.end(); ++it)
		{
			histoFile << normalisedHistograms[it->first].at(i) << ",";
		}
		histoFile << endl;
		currentValue += interval;
	}
	
	histoFile.close();

}