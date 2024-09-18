#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <vector>
#include <format> 
#include <iostream>
#include <utility>

using namespace std;

class TicketManager
{
public:
	TicketManager() {}
	enum BookingStatusEnum
	{
		BOOKED,
		AVAILABLE
	};
	enum SeatInRowEnum
	{
		A = 1,
		B = 2,
		C = 3,
		D = 4,
		E = 5,
		F = 6,
		G = 7,
		H = 8,
		I = 9,
		J = 10,
		K = 11,
		L = 12,
		M = 13,
		N = 14,
		O = 15,
		P = 16,
		Q = 17,
		R = 18,
		S = 19,
		T = 20,
		U = 21,
		V = 22,
		W = 23,
		X = 24,
		Y = 25,
		Z = 26
	};


	void AddTicketType(const chrono::year_month_day& date, 
					   const string&				 flightNumber, 
					   const SeatInRowEnum&			 seatsInRowTotal, 
					   const size_t&				 startRow,
					   const size_t&				 finishRow,
					   const unsigned int&			 price)
	{

		//const size_t seatsTotal = seatsInRowTotal * rowsTotal;
		unique_ptr<TicketType> newType (new TicketType(date, flightNumber, startRow, finishRow, seatsInRowTotal, price));
		
		if (!ticketTypeMap.contains(newType->GetKey()))  // if key not present
		{
			ticketTypeMap[newType->GetKey()] = vector<unique_ptr<TicketType>>();  // need to put a vector here 
		}
		ticketTypeMap[newType->GetKey()].push_back(move(newType)); 
	
	}
	
	int BookTicket(const chrono::year_month_day& date,
					const string&				 flightNumber,
					const size_t&				 rowNum,
					const SeatInRowEnum&		 seat,
					const string&				 passangerName)
	{
		idCount++;
		const unsigned int id = idCount;

		const string& groupKey = CalculateGroupKey(date, flightNumber);

		// check for a ticketType
		if (!ticketTypeMap.contains(groupKey))  // if no such type
		{
			idCount--;
			return -1;  // failure code
		}
		
		for (auto& ticketType:ticketTypeMap[groupKey])
		{
			if (ticketType->GetStartRow() <= rowNum &&
				ticketType->GetFinishRow() >= rowNum &&
				ticketType->GetSeatInRowAmount() >= seat)
			{	
				shared_ptr<Ticket> newTicket = make_shared<Ticket>(date, flightNumber, passangerName, seat, rowNum, ticketType->GetPrice(), id);
				if (!ticketsBought.contains(*(newTicket.get()))) 
				{
					ticketsBought.insert(*(newTicket.get()));
					idToTicketMap[idCount] = newTicket;
					
					if (!passangerToTicketsMap.contains(passangerName))  // if not present -> create a vector
					{
						passangerToTicketsMap[passangerName] = vector<shared_ptr<Ticket>>({ newTicket });
					}
					else 
					{
						passangerToTicketsMap[passangerName].push_back(newTicket);
					}
					if (!groupKeyToTicketsMap.contains(groupKey))  // if not present -> create a vector
					{
						groupKeyToTicketsMap[groupKey] = vector<shared_ptr<Ticket>>({ newTicket });
					}
					else
					{
						groupKeyToTicketsMap[groupKey].push_back(newTicket);
					}
					return id;  // return id of the ticket
				}
			}
		}
		cout << format("No such ticket(might be already booked) to match date {}, flightNum {}, seat position {}{}\n", date, flightNumber, rowNum, seatEnumMap[seat]);
		return -1;
	}

	pair<int, string> ReturnTicketWithRefund(const unsigned int id)
	{
		pair<int, string> outputPair;
		if (!idToTicketMap.contains(id))
		{
			outputPair.first = -1;

			return outputPair;
		}
			
		auto ticket = idToTicketMap[id];  // copy in order not to be dependent on a (possibly) erased objet
			
		if (ticketsBought.contains(*(ticket.get())))
		{
			ticketsBought.erase(*(ticket.get()));  // erase from set of all tickets
			idToTicketMap.erase(id);  // erase from id map 

			auto& flightsOfPassangerVector = passangerToTicketsMap[ticket->GetPasssengerName()];  
			flightsOfPassangerVector.erase(find(flightsOfPassangerVector.begin(), flightsOfPassangerVector.end(), ticket));  // erase from passanger to tickets map
			
			auto& flightsSpecifiedByKeyVector = groupKeyToTicketsMap[ticket->GetUniqueKey()];
			flightsSpecifiedByKeyVector.erase(find(flightsSpecifiedByKeyVector.begin(), flightsSpecifiedByKeyVector.end(), ticket));  // erase from group key to tickets map
			
			outputPair.first = ticket->GetPrice();
			outputPair.second = ticket->GetPasssengerName();
				
			return outputPair;
		}		
	}

	string CheckAvailable(chrono::year_month_day date, string flightNumber)
	{
		string groupKey = CalculateGroupKey(date, flightNumber);
		if (!ticketTypeMap.contains(groupKey))
		{
			return "";
		}
		string result = "";

		unordered_set<SeatPosition, SeatPosition::SeatPositionHasher> seatsOccupied;

		for (auto& ticket : groupKeyToTicketsMap[groupKey])
		{
			seatsOccupied.insert(ticket->GetSeat());  
		}

		for (auto& ticketType : ticketTypeMap[groupKey]) 
		{
			size_t startRow = ticketType->GetStartRow();
			size_t finishRow = ticketType->GetFinishRow();
			SeatInRowEnum totalSeatsInRow = ticketType->GetSeatInRowAmount();

			for (size_t row = startRow; row <= finishRow; row++)
			{
				size_t counter = 0;
				for (size_t seat = SeatInRowEnum::A; seat <= totalSeatsInRow; seat++)
				{
					SeatPosition curSeatPos(static_cast<SeatInRowEnum>(seat), row);

					if (!seatsOccupied.contains(curSeatPos))
					{
						counter++;
						result += format("{}{}, {}$ - price|", curSeatPos.GetRowNumber(), 
																 seatEnumMap[curSeatPos.GetSeatInRow()],
																 ticketType->GetPrice());
						if (counter%3 != 0)
						{
							result += '\t';
						}
						else 
						{
							result += '\n';
						}
					}
				}
			}
		}
		
		return result;
	}

	string ViewTicketsOfPassanger(const string& passangerName)
	{
		if (!passangerToTicketsMap.contains(passangerName))
			return "";
		auto& ticketsVector = passangerToTicketsMap[passangerName]; 
		string output = "";
		size_t counter = 0;
		for (auto& ticket : ticketsVector)
		{
			counter++;
			output += format("{}. Fligth {}, {}, seat{}{}, price {}$\n", counter, 
																		 ticket->GetFlightNum(), 
																	     ticket->GetDate(),
																		 ticket->GetSeat().GetRowNumber(),
																		 seatEnumMap[ticket->GetSeat().GetSeatInRow()],
																		 ticket->GetPrice());
		}
		return output;
	}

	string ViewById(const unsigned int& id)
	{
		if (!idToTicketMap.contains(id))
			return "";
		string output = "";
		auto& ticket = idToTicketMap[id];
		output = format("Flight {}, {}, seat {}{}, price {}$, {}\n", ticket->GetFlightNum(),
																	   ticket->GetDate(),
																	   ticket->GetSeat().GetRowNumber(),
																	   seatEnumMap[ticket->GetSeat().GetSeatInRow()],
																	   ticket->GetPrice(),
																	   ticket->GetPasssengerName());
		return output;
	}	

	string ViewByDateAndNumber(const chrono::year_month_day& date, const string& flightNumber)
	{
		string groupKey = CalculateGroupKey(date, flightNumber);

		if (!groupKeyToTicketsMap.contains(groupKey))
		{
			return "";
		}
		string output = "";
		size_t counter = 0;
		for (auto& ticket : groupKeyToTicketsMap[groupKey])
		{
			counter++;
			output += format("{} Seat {}{}, {}, {}$\n", counter,
													ticket->GetSeat().GetRowNumber(),
													seatEnumMap[ticket->GetSeat().GetSeatInRow()],
													ticket->GetPasssengerName(),
													ticket->GetPrice());
		}

		return output;
	}

private:
	unsigned int idCount = 0;
	
	unordered_map<SeatInRowEnum, char> seatEnumMap =
	{
		{ SeatInRowEnum::A, 'A' },
		{ SeatInRowEnum::B, 'B' },
		{ SeatInRowEnum::C, 'C' },
		{ SeatInRowEnum::D, 'D' },
		{ SeatInRowEnum::E, 'E' },
		{ SeatInRowEnum::F, 'F' },
		{ SeatInRowEnum::G, 'G' },
		{ SeatInRowEnum::H, 'H' },
		{ SeatInRowEnum::I, 'I' },
		{ SeatInRowEnum::J, 'J' },
		{ SeatInRowEnum::K, 'K' },
		{ SeatInRowEnum::L, 'L' },
	};

	string CalculateGroupKey(const chrono::year_month_day& date, const string& flightNumber)
	{
		return	flightNumber +
				to_string(int(date.year())) +
				to_string(unsigned(date.month())) +
				to_string(unsigned(date.day()));
	}
	
	class SeatPosition
	{
	public:

		SeatPosition(const SeatInRowEnum seatInRow, const size_t rowNumber)
			:	seatInRow(seatInRow), 
				rowNumber(rowNumber) {}
		SeatPosition(const SeatPosition& other)  // copy constructor
		{
			this->seatInRow = other.seatInRow;
			this->rowNumber = other.rowNumber;
		}
		SeatInRowEnum GetSeatInRow() const
		{
			return this->seatInRow;
		}
		size_t GetRowNumber() const
		{
			return this->rowNumber;
		}
		bool operator ==(const SeatPosition& other) const
		{
			return this->seatInRow == other.seatInRow &&
				   this->rowNumber == other.rowNumber;
		}
		SeatPosition& operator = (const SeatPosition& other) 
		{
			if (this != &other)  // Check for self-assignment
			{
				this->rowNumber = other.rowNumber;
				this->seatInRow = other.seatInRow;
			}
			return *this;
		}
		struct SeatPositionHasher
		{
			size_t operator ()(const SeatPosition& seatPos) const
			{
				return (hash<size_t>()(seatPos.GetRowNumber()) ^
						hash<int>()(seatPos.GetSeatInRow()));
			}
		};


	private:
		SeatInRowEnum seatInRow;
		size_t rowNumber;
	};

	class TicketType
	{
	public:
		TicketType(const chrono::year_month_day& date, 
				   const string&				 flightNumber, 
				   const size_t&				 startRow, 
				   const size_t&				 finishRow, 
				   const SeatInRowEnum&			 seatsInRowTotal, 
				   const int&					 price)
			:	date(date), 
				flightNumber(flightNumber),
				startRow(startRow), 
				finishRow(finishRow), 
				seatsInRowTotal(seatsInRowTotal),
				price(price) {}


		size_t GetStartRow() const
		{
			return this->startRow;
		}
		size_t GetFinishRow() const
		{
			return this->finishRow;
		}
		int GetPrice() const
		{
			return this->price;
		}
		/*size_t GetSeatTotal() const
		{
			return this->seatsTotal;
		}
		size_t GetRowTotal() const
		{
			return this->rowsTotal;
		}*/
		SeatInRowEnum GetSeatInRowAmount() const
		{
			return this->seatsInRowTotal;
		}
		string GetKey() const
		{
			return this->groupKey;
		}

	private:
		size_t				   startRow;
		size_t				   finishRow;
		SeatInRowEnum		   seatsInRowTotal;
		string				   flightNumber;
		chrono::year_month_day date;
		int					   price;
		string				   groupKey = flightNumber + 
							   to_string(int(date.year())) + 
							   to_string(unsigned(date.month())) + 
							   to_string(unsigned(date.day()));
	};
	
	unordered_map<string, vector<unique_ptr<TicketType>>> ticketTypeMap;
	
	class Ticket
	{
	public:

		Ticket(const chrono::year_month_day& date,
			const string& flightNumber,   // default constructor
			const string& passangerName,
			const SeatInRowEnum& seatEnum,
			const size_t& rowNum,
			const unsigned int& price,
			const unsigned int& id)
			: flightNumber(flightNumber),
			passangerName(passangerName),
			date(date),
			price(price),
			//status(status),
			seatPos(new SeatPosition(seatEnum, rowNum)),
			id(id) {}

		Ticket(const Ticket& other) // copy constructor
			: flightNumber(other.flightNumber),
			passangerName(other.passangerName),
			date(other.date),
			//status(other.status),
			seatPos(new SeatPosition(*(other.seatPos))),
			price(other.price),
			id(other.id) {}

		chrono::year_month_day GetDate() const
		{
			return this->date;
		}
		SeatPosition GetSeat() const
		{
			return *(this->seatPos);
		}
		string GetFlightNum() const
		{
			return this->flightNumber;
		}
		string GetPasssengerName() const
		{
			return this->passangerName;
		}
		int GetPrice() const
		{
			return this->price;
		}

		string GetUniqueKey()
		{
			return this->groupKey;
		}

		bool operator == (const Ticket& other) const
		{
			return this->flightNumber == other.flightNumber &&
				this->passangerName == other.passangerName &&
				(*(this->seatPos)).operator == (*(other.seatPos)) &&  // syntax to prevent c2666 error
				this->date == other.date;
		}

	private:

		chrono::year_month_day date;
		string flightNumber;
		string passangerName;
		unsigned int id;
		int price;
		string groupKey = flightNumber +
						  to_string(int(date.year())) +
						  to_string(unsigned(date.month())) +
						  to_string(unsigned(date.day()));

		unique_ptr<SeatPosition> seatPos;

		//BookingStatusEnum status;

	};

	struct TicketHasher 
	{
		size_t operator()(const Ticket& ticket) const
		{

			return ((hash<string>()(ticket.GetFlightNum()) ^
					(hash<string>()(ticket.GetPasssengerName()) << 1)) >> 1) ^
					(hash<int>()(int(ticket.GetDate().year())) << 1) ^
					(hash<unsigned>()(unsigned(ticket.GetDate().month())) << 1) ^
					(hash<unsigned>()(unsigned(ticket.GetDate().day())) << 1) ^
					(hash<size_t>()(ticket.GetSeat().GetRowNumber()) << 1) ^
					(hash<int>()(ticket.GetSeat().GetSeatInRow()));
		}
	};
	unordered_set<Ticket, TicketHasher> ticketsBought;  // CNANGE TO JUST OBJECTS(NOT POINTERS)
	unordered_map<unsigned int, shared_ptr<Ticket>> idToTicketMap;
	unordered_map<string, vector<shared_ptr<Ticket>>> passangerToTicketsMap;
	unordered_map<string, vector<shared_ptr<Ticket>>> groupKeyToTicketsMap;

};


