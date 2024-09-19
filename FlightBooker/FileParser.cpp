#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cctype>
#include <chrono>
#include <sstream>
#include <iostream>
using namespace std;

class FileParser
{
public:
	FileParser(const wstring& fileName)
		:	fileName(fileName)
	{
		fileHandle = CreateFile(this->fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			throw runtime_error("Could not open the file\n");
		}
	}

	~FileParser()
	{
		CloseHandle(fileHandle);
	}
	struct FlightConfig
	{
		chrono::year_month_day date;
		string flightNumber;
		size_t seatPerRow;
		vector <pair<string, int>> rowsNumsToPrice;
	};
	vector<shared_ptr<FlightConfig>> GetFlightsConfig()  // get all the configs
	{
		unique_ptr<char[]> buffer(new char[CHUNK_SIZE]);
		DWORD bytesRead;
		string line;
		vector<shared_ptr<FlightConfig>> result;

		while (ReadFile(fileHandle, buffer.get(), CHUNK_SIZE, &bytesRead, NULL) && bytesRead > 0) {  // error reading symbols

			for (DWORD index = 0; index < bytesRead; ++index)
			{
				char ch = buffer[index];

				if (ch == '\n')
				{
					rowCounter++;

					stringstream lineStream(line);  // turning string into a stream to parse it easily
					string dateStr, token, priceStr;

					shared_ptr<FlightConfig> currentConfig = make_unique<FlightConfig>();

					lineStream >> chrono::parse("%d.%m.%Y", currentConfig->date)
						>> currentConfig->flightNumber
						>> currentConfig->seatPerRow;

					while (lineStream >> token)
					{
						pair <string, int> rowsToPrice;
						rowsToPrice.first = token;  // get rows range  
						lineStream >> priceStr;
						rowsToPrice.second = stoi(priceStr.substr(0, priceStr.size() - 1));  // get rid of $ 
						currentConfig->rowsNumsToPrice.push_back(move(rowsToPrice));
					}
					result.push_back(move(currentConfig));

					if (!line.empty())
					{
						line.clear();  // Clear the line buffer
					}
				}
				else if (ch != '\r') {
					line += ch;  // Append character to line
				}
			}
		}
		return result;
	}

private:

	vector<shared_ptr<FlightConfig>> flightsConfigVector;
	unsigned int rowCounter = 1; 
	wstring fileName;
	HANDLE fileHandle;
	int const CHUNK_SIZE = 1024;
	FlightConfig lineStruct;
};