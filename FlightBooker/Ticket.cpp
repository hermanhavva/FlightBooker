#include <string>
#include <vector>
using namespace std;

class TicketManager
{
public:
	TicketManager() {}
	void AddTicketType(size_t seatsInRow, size_t rowTotal)
	{
		size_t seatTotal = seatsInRow * rowTotal;
		ticketTypes.push_back(TicketType(seatTotal, rowTotal, seatsInRow));  // this thing got to be a map 
	}
private:
	class TicketType
	{
	public:
		TicketType(const size_t& seatTotal, const size_t& rowTotal, const size_t& seatInRowAmount)
			: seatTotal(seatTotal), rowTotal(rowTotal), seatInRowAmount(seatInRowAmount) {}
		
		size_t GetSeatTotal() const
		{
			return this->seatTotal;
		}
		size_t GetRowTotal() const
		{
			return this->rowTotal;
		}
		size_t GetSeatInRowAmount() const
		{
			return this->seatInRowAmount;
		}

	private:
		size_t seatTotal = 0;
		size_t rowTotal = 0;
		size_t seatInRowAmount = 0;
	};
	// got to create the dictionary (unordered map of ticketTypes to store the diffrent types)
};



class Ticket
{
private:
	string flightNumber = nullptr;
	string passangerName = nullptr;
	unsigned int* seatNumber = nullptr;

	enum BookingStatus
	{
		BOOKED,
		AVAILABLE
	};

};