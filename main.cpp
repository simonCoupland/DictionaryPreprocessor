#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

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

}