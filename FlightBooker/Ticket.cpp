#include <string>
#include <unordered_map>
#include <chrono>
#include <unordered_set>
#include <vector>
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
		J = 8,
		K = 9,
		L = 10,
		M = 11
	};
	
	void AddTicketType(const chrono::year_month_day& date, 
					   const string&				 flightNumber, 
					   const SeatInRowEnum&			 seatsInRowTotal, 
					   const size_t&				 rowsTotal, 
					   const unsigned int&			 price)
	{

		const size_t seatsTotal = seatsInRowTotal * rowsTotal;
		unique_ptr<TicketType> newType (new TicketType(date, flightNumber, seatsTotal, rowsTotal, seatsInRowTotal, price));
		
		if (ticketTypeMap.find(newType->GetKey()) == ticketTypeMap.end())  // if key not present
		{
			ticketTypeMap[newType->GetKey()] = vector<unique_ptr<TicketType>>();  // need to put a vector here 
		}
		ticketTypeMap[newType->GetKey()].push_back(move(newType)); 
	
	}

private:

	
	
	class SeatPosition
	{
	public:

		SeatPosition(const SeatInRowEnum seatInRow, const size_t rowNumber)
			:seatInRow(seatInRow), rowNumber(rowNumber) {}
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
		bool operator ==(const SeatPosition& other) 
		{
			return this->seatInRow == other.seatInRow &&
				   this->rowNumber == other.rowNumber;
		}
		SeatPosition& operator =(const SeatPosition& other)
		{
			if (this != &other)  // Check for self-assignment
			{
				this->rowNumber = other.rowNumber;
				this->seatInRow = other.seatInRow;
			}
			return *this;
		}


	private:
		SeatInRowEnum seatInRow;
		size_t rowNumber;
	};

	class TicketType
	{
	public:
		TicketType(const chrono::year_month_day& date, 
				   const string&				 flightNumber, 
				   const size_t&				 seatsTotal, 
				   const size_t&				 rowsTotal, 
				   const SeatInRowEnum&			 seatsInRowTotal, 
				   const unsigned int&			 price)
			:	date(date), 
				flightNumber(flightNumber),
				seatsTotal(seatsTotal), 
				rowsTotal(rowsTotal), 
				seatsInRowTotal(seatsInRowTotal),
				price(price) {}

		size_t GetSeatTotal() const
		{
			return this->seatsTotal;
		}
		size_t GetRowTotal() const
		{
			return this->rowsTotal;
		}
		SeatInRowEnum GetSeatInRowAmount() const
		{
			return this->seatsInRowTotal;
		}
		string GetKey() const
		{
			return this->uniqueKey;
		}

	private:
		size_t				   seatsTotal = 0;
		size_t				   rowsTotal  = 0;
		SeatInRowEnum		   seatsInRowTotal;
		string				   flightNumber;
		chrono::year_month_day date;
		unsigned int		   price;
		string				   uniqueKey = flightNumber + 
							   to_string(int(date.year())) + 
							   to_string(unsigned(date.month())) + 
							   to_string(unsigned(date.day()));
	};
	
	unordered_map<string, vector<unique_ptr<TicketType>>> ticketTypeMap;

	class Ticket
	{
	public:
		
		Ticket(const string&				 flightNumber,   // default constructor
			   const string&				 passangerName, 
			   const chrono::year_month_day& date, 
			   const BookingStatusEnum&		 status, 
			   const SeatInRowEnum&			 seatEnum,
			   const size_t&				 rowNum,
			   unsigned int&				 id)
			:	flightNumber(flightNumber),
				passangerName(passangerName),
				date(date),  
				status(status),
				seatPos(new SeatPosition(seatEnum, rowNum)),
				id(id) {}

		Ticket(const Ticket& other) // copy constructor
			:	flightNumber(other.flightNumber),
				passangerName(other.passangerName),
				date(other.date),  
				status(other.status),
				seatPos(new SeatPosition(*(other.seatPos))),
				id(other.id) {}
				
		void AddBoughtTicket()
		{
			ticketsBought.insert(this);
		}

		chrono::year_month_day GetDate() const
		{
			return this->date;
		}
		bool operator == (const Ticket& other)
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
		
		unique_ptr<SeatPosition> seatPos;
		
		BookingStatusEnum status;
		
	};

	static unordered_set<Ticket*> ticketsBought;
};


