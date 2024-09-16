#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>
#include "Ticket.cpp"
#include "FileParser.cpp"
using namespace std;
    



int main ()
{

	
	TicketManager* ticketmanager = new TicketManager();
	chrono::year_month_day date{ chrono::year(2024) / chrono::month(9) / chrono::day(14) };
	
	ticketmanager->AddTicketType(date, string("A100"), ticketmanager->E, 20, 100);


	delete ticketmanager;
	return 0;
}
