#pragma once
#include <string>
#include "Airplane.h"

class TicketManager
{
private:
	class TicketType
	{
	private:
		size_t seatAmount = 0;
		size_t rowAmount = 0;
		size_t seatInRowAmount = 0; 
	};

};



class Ticket 
{
private:
	string flightNumber = nullptr;
	string passangerName = nullptr;
	unsigned int *seatNumber = nullptr;
	
	enum BookingStatus
	{
		BOOKED,
		AVAILABLE
	};

};
