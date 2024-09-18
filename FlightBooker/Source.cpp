#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>
#include <sstream>
#include <string>

#include "Ticket.cpp"
#include "FileParser.cpp"
using namespace std;
    

class Program;
HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

int main ()
{

	SetConsoleTextAttribute(hout, BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN);
	system("cls");
	TicketManager* ticketmanager = new TicketManager();
	chrono::year_month_day date{ chrono::year(2024) / chrono::month(9) / chrono::day(14) };
	
	ticketmanager->AddTicketType(date, string("A100"), ticketmanager->E, 20, 100);


	delete ticketmanager;
	return 0;
}

class Program
{
public:
	string token = "";
	stringstream userInputSS;
	void PrintMainMenu()
	{
		string output = format("Enter your command: check <date> <flightNum>\tbook <date> <flightNum> <seatPos> <userName>\n"
												"\t\treturn <id>\t\tview id <id>\n"
												"\t\tview useraname <username>\tview flight <date> <flightNum>\n");
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
		unique_ptr <Arguments> args = move(ParseInputGetStrategy());  //  got to test this 
		
		pair<size_t, TicketManager::SeatInRowEnum> seatPosPair;
		pair <int, string> priceAndNamePair;
	
		string output;
		switch (strategy)
		{
		case CHECK_BY_DATE_FLIGHTNUM:
			ticketManager.CheckAvailable(args->date, args->flightNumber);
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
			return ticketManager.BookTicket(args->date, args->flightNumber, seatPosPair.first, seatPosPair.second, args->userName);
			
		case RETURN_BY_ID:
			priceAndNamePair = ticketManager.ReturnTicketWithRefund(args->id);
			if (priceAndNamePair.first == -1)
			{
				cout << format("Could not return ticket with is {}\n", args->id);
				return COMMAND_FAILURE;
			}
			cout << format("Success, {}$ for {}\n", priceAndNamePair.first, priceAndNamePair.second);
			break;

		case VIEW_BY_ID:
			output = ticketManager.ViewById(args->id);
			if (output == "")
			{
				cout << format("Could not find a ticket with id {}\n", args->id);
				return COMMAND_FAILURE;
			}
			cout << format("Success, {}", output);
			break;

		case VIEW_BY_USERNAME:
			output = ticketManager.ViewTicketsOfPassanger(args->userName);
			if (output == "")
			{
				cout << format("Could not find userName {}\n", args->userName);
				return COMMAND_FAILURE;
			}
			cout << format("Success, {}\n", output);
			break;

		case VIEW_BY_DATE_FLIGHTNUM:
			output = ticketManager.ViewByDateAndNumber(args->date, args->flightNumber);
			if (output == "")
			{
				cout << format("Could not find any tickets for date {}, flightNumber {}", args->date, args->flightNumber); 
				return COMMAND_FAILURE;
			}
			cout << format("Success, {}", output);
			break;

		case UNDEFINED:
			return COMMAND_FAILURE;
		}
		return COMMAND_SUCCESS;
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

	unique_ptr <Arguments>& ParseInputGetStrategy()
	{
		unique_ptr <Arguments> args = make_unique<Arguments>();
		userInputSS >> token;
		if (token == "check")
		{
			strategy = CHECK_BY_DATE_FLIGHTNUM;
			userInputSS >> chrono::parse("%d.%m.%Y", args->date) 
						>> args->flightNumber;
		}
		else if (token == "book")
		{
			strategy = BOOK_BY_DATE_FLIGHTNUM_USERNAME;
			userInputSS >> chrono::parse("%d.%m.%Y", args->date)
						>> args->flightNumber
						>> args->seatPos
						>> args->userName;
 
		}
		else if (token == "return")
		{
			strategy = RETURN_BY_ID;
			userInputSS >> args->id;
		}
		else if (token == "view")
		{
			if (DetermineViewStrategy() == -1)
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
				userInputSS >> chrono::parse("%d.%m.%Y", args->date) >> args->flightNumber;
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

	const int COMMAND_SUCCESS = 0;
	const int COMMAND_FAILURE = -1;

	TicketManager ticketManager;
	StrategyEnum strategy;
};

