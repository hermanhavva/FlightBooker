#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>
#include <sstream>
#include <string>
#include "Ticket.cpp"
#include "FileParser.cpp"

using namespace std;

HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

class Program
{ 
public:
	string token = "";
	stringstream userInputSS;
	void PrintMainMenu()
	{
		string output = format("----------------------------------------------------------------------------\nEnter your command: \ncheck <date> <flightNum>\tbook <date> <flightNum> <seatPos> <userName>\n"
							   "return <id>\t\t\tview id <id>\n"
							   "view username <username>\tview flight <date> <flightNum>\n"
							   "----------------------------------------------------------------------------\n");
		cout << output;
	}
	void GetUserCommand()
	{
		string userInputSTR;
		getline(cin, userInputSTR);
		userInputSS = stringstream(userInputSTR);
	}

	int ExecuteCommand()
	{
		shared_ptr <Arguments> args = ParseInputGetStrategy();  //  got to test this 

		pair<size_t, TicketManager::SeatInRowEnum> seatPosPair;
		pair <int, string> priceAndNamePair;

		int id;
		string output;
		switch (strategy)
		{
		case CHECK_BY_DATE_FLIGHTNUM:
			output = ticketManager.CheckAvailable(args->date, args->flightNumber);
			cout << output << endl;
			break;

		case BOOK_BY_DATE_FLIGHTNUM_USERNAME:
			try
			{
				seatPosPair = ParseSeatPos(args->seatPos);
			}
			catch (const exception& e)
			{
				cout << e.what();
				strategy = UNDEFINED;
				return COMMAND_FAILURE;
			}
			id = ticketManager.BookTicket(args->date, args->flightNumber, seatPosPair.first, seatPosPair.second, args->userName);
			
			if (id == COMMAND_FAILURE)
			{
				cout << format("Could not book a flight {} for {}\n", args->flightNumber, args->userName);
				return - 1;
			}
			cout << format("Success, flight {} booked for {} with ID {}\n", args->flightNumber, args->userName, id);
			break;

		case RETURN_BY_ID:
			priceAndNamePair = ticketManager.ReturnTicketWithRefund(args->id);
			if (priceAndNamePair.first == -1)
			{
				cout << format("Could not return ticket with ID {}\n", args->id);
				return COMMAND_FAILURE;
			}
			cout << format("Success, {}$ for {}\n", priceAndNamePair.first, priceAndNamePair.second);
			break;

		case VIEW_BY_ID:
			output = ticketManager.ViewById(args->id);
			if (output == "")
			{
				cout << format("Could not find a ticket with ID {}\n", args->id);
				return COMMAND_FAILURE;
			}
			cout << format("Success {}", output);
			break;

		case VIEW_BY_USERNAME:
			output = ticketManager.ViewTicketsOfPassanger(args->userName);
			if (output == "")
			{
				cout << format("Could not find userName: {}\n", args->userName);
				return COMMAND_FAILURE;
			} 
			cout << format("Success, \n{}\n", output);  
			break;
			 
		case VIEW_BY_DATE_FLIGHTNUM: 
			output = ticketManager.ViewByDateAndNumber(args->date, args->flightNumber);
			if (output == "")
			{
				cout << format("Could not find any tickets for date {}, flightNumber {}", args->date, args->flightNumber);
				return COMMAND_FAILURE;
			}
			cout << format("Success, \n{}", output);
			break;

		case UNDEFINED:
			return COMMAND_FAILURE;
		}
		
		return COMMAND_SUCCESS;
	}
	 
	int LoadConfig(wstring fileName)
	{
		unique_ptr<FileParser> fileParser;
		try
		{
			fileParser = make_unique<FileParser>(fileName);
		}
		catch (const exception& e)
		{
			cout << format("{}\n", e.what());
			return COMMAND_FAILURE;
		}

		auto eachFlightConfigVector = fileParser->GetFlightsConfig();

		for (auto& flightConfigStruct : eachFlightConfigVector)
		{
			for (auto& rowToPricePair : flightConfigStruct->rowsNumsToPrice)
			{
				pair<size_t, size_t> seatRangePair = ExtractSeatRange(rowToPricePair.first);

				ticketManager.AddTicketType(flightConfigStruct->date,
					flightConfigStruct->flightNumber,
					static_cast<TicketManager::SeatInRowEnum>(flightConfigStruct->seatPerRow),
					seatRangePair.first,
					seatRangePair.second,
					rowToPricePair.second);
			}
		}
	}

private:
	enum StrategyEnum
	{
		CHECK_BY_DATE_FLIGHTNUM = 1,
		BOOK_BY_DATE_FLIGHTNUM_USERNAME = 2,
		RETURN_BY_ID = 3,
		VIEW_BY_ID = 4,
		VIEW_BY_USERNAME = 5,
		VIEW_BY_DATE_FLIGHTNUM = 6,
		UNDEFINED = 7
	};

	struct Arguments
	{
		string flightNumber;
		string userName;
		string seatPos;
		chrono::year_month_day date;
		unsigned int id;

	};

	shared_ptr <Arguments> ParseInputGetStrategy()
	{
		shared_ptr <Arguments> args = make_shared<Arguments>();
		userInputSS >> token;
		if (token == "check")
		{
			strategy = CHECK_BY_DATE_FLIGHTNUM;
			string date, num;
			userInputSS >> date >> num;
			stringstream streamDate(date); stringstream streamNum(num);
			streamDate >> chrono::parse("%d.%m.%Y", args->date);
			streamNum >> args->flightNumber;
		}
		else if (token == "book")
		{
			strategy = BOOK_BY_DATE_FLIGHTNUM_USERNAME;
			string date, num, seat, user;
			userInputSS >> date
				>> num
				>> seat
				>> user;
			stringstream dateStream(date);
			dateStream >> chrono::parse("%d.%m.%Y", args->date);
			args->flightNumber = num;
			args->seatPos = seat;
			args->userName = user;  // check it

		}
		else if (token == "return")
		{
			strategy = RETURN_BY_ID;
			userInputSS >> args->id;
		}
		else if (token == "view")
		{
			if (DetermineViewStrategy() == COMMAND_FAILURE)
			{
				cout << "\nProblem parsing input\n";
				strategy = UNDEFINED;
			}
			// getting args
			switch (strategy)
			{
			case VIEW_BY_ID:
				userInputSS >> args->id;
				break;
			case VIEW_BY_USERNAME:
				userInputSS >> args->userName;
				break;
			case VIEW_BY_DATE_FLIGHTNUM:
				string s1, s2;
				userInputSS >> s1 >> s2;
				stringstream streamDate(s1); stringstream streamNum(s2);
				streamDate >> chrono::parse("%d.%m.%Y", args->date);
				streamNum >> args->flightNumber;
				break;
			}
		}
		else
		{
			cout << "\nProblem parsing input\n";
			strategy = UNDEFINED;
		}

		userInputSS.clear();
		token.clear();
		return args;
	}
	int DetermineViewStrategy()
	{
		userInputSS >> token;
		if (token == "id")
		{
			strategy = VIEW_BY_ID;
		}
		else if (token == "username")
		{
			strategy = VIEW_BY_USERNAME;
		}
		else if (token == "flight")
		{
			strategy = VIEW_BY_DATE_FLIGHTNUM;
		}
		else
		{
			cout << "\nProblem parsing the view args\n";
			return COMMAND_FAILURE;  // could not parse
		}
		return COMMAND_SUCCESS;
	}
	pair <size_t, TicketManager::SeatInRowEnum> ParseSeatPos(const string& seatPosStr)
	{
		size_t rowNum = 0;
		char seatLetter;

		auto it = seatPosStr.data();  // pointer to [0] element of char arr
		auto end = seatPosStr.data() + seatPosStr.size();

		// Use from_chars to extract the numeric part
		auto result = from_chars(it, end, rowNum);
		if (result.ec != std::errc() || result.ptr == it) {
			throw std::invalid_argument("Invalid numeric part in seat string");
		}

		// Check if the remaining character is a valid uppercase letter
		seatLetter = *result.ptr;
		if (isupper(seatLetter))
		{
			TicketManager::SeatInRowEnum seatEnum = static_cast<TicketManager::SeatInRowEnum>(seatLetter - 'A' + 1);  // ASCII code manipulation
			return { rowNum, seatEnum };
		}
		TicketManager::SeatInRowEnum seatEnum = static_cast<TicketManager::SeatInRowEnum>(seatLetter - 'a' + 1);  // ASCII code manipulation

		return { rowNum, seatEnum };
	}
	pair<size_t, size_t> ExtractSeatRange(const string& rangeStr) {
		size_t delimiterPos = rangeStr.find('-');
		if (delimiterPos == std::string::npos) {
			throw std::invalid_argument("Invalid format. Expected 'number-number'.");
		}

		size_t firstNumber = std::stoull(rangeStr.substr(0, delimiterPos));
		size_t secondNumber = std::stoull(rangeStr.substr(delimiterPos + 1));

		return make_pair(firstNumber, secondNumber);
	}

	const int COMMAND_SUCCESS = 0;
	const int COMMAND_FAILURE = -1;

	TicketManager ticketManager;
	StrategyEnum strategy;
};




int main ()
{

	SetConsoleTextAttribute(hout, BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN);
	system("cls");
	Program programDriver;

	wstring fileName = L"C:\\c++ projects\\FlightBooker\\FlightBooker\\config.txt";

	programDriver.LoadConfig(fileName);

	while(true)
	{
		programDriver.PrintMainMenu();
		programDriver.GetUserCommand();
		programDriver.ExecuteCommand();

	}

	return 0;
}
