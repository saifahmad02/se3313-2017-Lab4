#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>


using namespace Sync;


class SocketThread : public Thread
{
private:
    
    Socket& socket;
    
    ByteArray data;

    bool &terminate;

    std::vector<SocketThread*> &sockThrHolder;
public:
    SocketThread(Socket& socket, bool &terminate, std::vector<SocketThread*> &clientSockThr)
    : socket(socket), terminate(terminate), sockThrHolder(clientSockThr)
    {}

    ~SocketThread()
    {this->terminationEvent.Wait();}

    Socket& GetSocket()
    {
        return socket;
    }

    virtual long ThreadMain() {

        try
        {
            while(!terminate)
            {
                
        

                socket.Read(data);

                // Perform operations on the data
                std::string res = data.ToString();
                std::for_each(res.begin(), res.end(), [](char & res){
                    res = ::toupper(res);
                    });
                

                if (res=="DONE") {
                    sockThrHolder.erase(std::remove(sockThrHolder.begin(), sockThrHolder.end(), this), sockThrHolder.end());
                    terminate = true; 
                    break;      
                }
                
                
                res.append("-received");
                // Send it back
                socket.Write(ByteArray(res));

            }
            
        }catch (std::string &s) {
            std::cout<<s<<std::endl;
        }
		
        catch (std::string err)
        {
            std::cout<<err<<std::endl;
        }
        std::cout<<"Client Disconnected" <<std::endl;
        return 0;
    }
};

// This is for server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    bool terminate = false;
    std::vector<SocketThread*> sockThrHolder;
public:
    ServerThread(SocketServer& server)
    : server(server)
    {}

    ~ServerThread()
    {
        // Cleanup
        for(auto thread: sockThrHolder) {
            try{
                Socket& toClose = thread->GetSocket();
                toClose.Close();
            } catch (...) {

            }
        } std::vector<SocketThread*>().swap(sockThrHolder);
        std::cout<<"Closing client from server"<<std::endl;
        terminate = true;
        
    }

    virtual long ThreadMain()
    {
        while (1) {
            try {
                // Wait for a client socket 
                Socket* newConnection = new Socket(server.Accept());

                ThreadSem serverBlock(1);

                // Pass a reference to this pointer 
                Socket& socketReference = *newConnection;
                sockThrHolder.push_back(new SocketThread(socketReference, terminate, std::ref(sockThrHolder)));
            } catch (std::string error)
            {
                std::cout << "ERROR: " << error << std::endl;
				
                return 1;
            }
			
			catch (TerminationException terminationException)
			{
				std::cout << "Server has shut down!" << std::endl;
				
				return terminationException;
			}
        }
        return 0;
    }
};


int main(void)
{
    std::cout << "I am a server." << std::endl;
	std::cout << "Press enter to terminate the server...";
    std::cout.flush();
	
    // Create server
    SocketServer server(3000);    

    // thread to perform server operations
    ServerThread serverThread(server);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();

    // Shut down and clean up the server
    server.Shutdown();

}
