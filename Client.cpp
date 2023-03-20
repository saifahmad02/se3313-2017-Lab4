#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

class ClientThread : public Thread
{
private:
	//conncected socket
	Socket& socket;
	bool &active;
	//data
	ByteArray data;
	std::string data_str;
public:
	ClientThread(Socket& socket, bool &active)
	: socket(socket), active(active)
	{}

	~ClientThread()
	{}

	virtual long ThreadMain()
	{
		while(true) {
			try {
				std::cout << "Please input your data (done to exit): ";
				std::cout.flush();

				
				std::getline(std::cin, data_str);
				data = ByteArray(data_str);

				if(data_str == "done") {
					active = false;
					break;
				}

				// Write to the server
				socket.Write(data);

				// Get the response
				int connection = socket.Read(data);

				if(connection == 0) {
					active = false;
					break;
				}

				std::cout<<"Server Response: " << data.ToString() << std::endl;
			} catch (std::string err)
            {
                std::cout<<err<<std::endl;
            }
		}

		return 0;
	}
};

int main(void)
{
	
	std::cout << "SE3313 Lab 3 Client" << std::endl;
	bool active = true;

	// Create our socket
	Socket socket("127.0.0.1", 3000);
	ClientThread clientThread(socket, active);
	socket.Open();

	while(active) {
		sleep(1);
	}

	socket.Close();	
	
	return 0;
}
