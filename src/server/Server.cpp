#include "Server.hpp"
#include "TBucket.hpp"
#include "Request.hpp"

volatile sig_atomic_t Server::got_signal = 0;

void Server::signal_handler(int sig)
{
	(void)sig;
    got_signal = 1;
}

Server::Server(): server_fd(0), running(false), MAX_CLIENTS(1024), rateLimiter(16000, 1600)
{
    ports.push_back(8080);
    ports.push_back(8081);
    ports.push_back(9090);
}

Server::~Server()
{
	if (running)
	{
		stop();
	}
}

void Server::stop()
{
    std::cout << "Stopping server." << std::endl;

    std::set<int>::iterator it;
    for(it = active_clients.begin(); it != active_clients.end(); ++it) 
    {
        int client_fd = *it;
        close(client_fd);
    }
    active_clients.clear();

    if (server_fd != -1) 
    {
        close(server_fd);
        server_fd = -1;
    }
    running = false;
}

std::string Server::generateHttpResponse(int statusCode, const std::string& statusMessage, const std::string& content)
{
	std::ostringstream response;
	response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
	response << "Content-Length: " << content.size() << "\r\n";
	response << "\r\n";  // End of headers
	response << content;
	return response.str();
}

std::string Server::handleHttpRequest(const std::string& method, const std::string& path, const std::string& protocol)
{
	(void)protocol;
	if (method == "GET")
	{
		if (path == "/" || path == "/index.html")
		{
			std::ifstream file("./src/server/index.html", std::ios::in | std::ios::binary);
			if (file.is_open())
			{
				std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();
				return generateHttpResponse(200, "OK", content);
			}
			else
			{
				return generateHttpResponse(404, "Not Found", "File not found");
			}
		}
		else
		{
			return generateHttpResponse(404, "Not Found", "File not found");
		}
	}
	// ... Handle other HTTP methods here

	return generateHttpResponse(501, "Not Implemented", "This method is not implemented");
}

void Server::start()
{
    int kq = kqueue();
    if (kq < 0) 
    {
        perror("Error creating kqueue");
        return ;
    }

    size_t NUM_SERVERS = ports.size();

    for (size_t i = 0; i < NUM_SERVERS; ++i)
    {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) 
        {
            perror("Error creating socket");
            return ;
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(ports[i]);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("Error binding socket");
            return ;
        }

        if (listen(serverSocket, 1024) < 0) {
            perror("Error listening");
            return ;
        }

        std::cout << "Server " << i + 1 << " listening on port " << ports[i] << std::endl;

        fcntl(serverSocket, F_SETFL, O_NONBLOCK | FD_CLOEXEC);

        serverSockets.push_back(serverSocket);

        struct kevent kev;
        EV_SET(&kev, serverSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(kq, &kev, 1, NULL, 0, NULL) < 0)
        {
            perror("Error registering server socket with kqueue");
            return ;
        }
    }

    int currentClientCount = 0;
    running = true;
    
    while (running)
    {
        struct kevent events[NUM_SERVERS + 1];
        int numEvents = kevent(kq, NULL, 0, events, NUM_SERVERS + 1, NULL);
        if (numEvents < 0)
        {
            perror("Error in kevent");
            return ;
        }
        
        for (int i = 0; i < numEvents; ++i)
        {
            struct kevent event = events[i];

            if (event.ident == static_cast<uintptr_t>(server_fd))
            {
                if (currentClientCount < MAX_CLIENTS)
                {
                    struct sockaddr_in client_address;
                    socklen_t client_addrlen = sizeof(client_address);
                    int new_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
                    if (new_socket < 0)
                    {
                        std::cerr << "Failed to accept new connection" << std::endl;
                    }

                    if (fcntl(new_socket, F_SETFL, O_NONBLOCK | FD_CLOEXEC) == -1) 
                    {
                        perror("fcntl");
                        close(new_socket);
                        continue;
                    }

                    if (new_socket >= 0)
                    {
                        struct kevent client_change;
                        EV_SET(&client_change, new_socket, EVFILT_READ | EVFILT_WRITE, EV_ADD, 0, 0, NULL);
                        if (kevent(kq, &client_change, 1, NULL, 0, NULL) == -1)
                        {
                            std::cerr << "Could not add client event to kqueue" << std::endl;
                        }
                        currentClientCount++;
                    }
                }   
                else
                {
                    std::cerr << "Max clients reached. Cannot accept more." << std::endl;
                    std::string httpResponse = generateHttpResponse(503, "Service Unavailable", "Server is overloaded");

                    struct sockaddr_in client_address;
                    socklen_t client_addrlen = sizeof(client_address);
                    int new_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
                    if (new_socket >= 0)
                    {
                        active_clients.insert(new_socket);
                        send(new_socket, httpResponse.c_str(), httpResponse.size(), 0);
                        close(new_socket);
                    }
                } 
            }
            else if (event.filter == EVFILT_READ)
            {
                char buffer[4096];
                ssize_t bytesRead = recv(event.ident, buffer, sizeof(buffer) - 1, 0);
                if (bytesRead < 0) 
                {
                    std::cerr << "b < 0" << std::endl;
                    perror("recv error");
                }
                else if (bytesRead == 0)
                {
                    active_clients.erase(event.ident);

                    struct kevent client_change;
                    EV_SET(&client_change, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    if (kevent(kq, &client_change, 1, NULL, 0, NULL) == -1)
                    {
                        perror("Error deleting client event from kqueue");
                        std::cerr << "Error: " << strerror(errno) << std::endl;
                        std::cerr << "Could not delete client event from kqueue" << std::endl;
                    }
                    client_write_queues.erase(event.ident);
                    close(event.ident);
                    currentClientCount--;
                }
                else
                {
                    std::cout << "Received " << bytesRead << " bytes of data" << std::endl;
                    buffer[bytesRead] = '\0';
                    std::string request(buffer);
                    std::cout << "---------\n" << buffer << '\n' << "---------\n" << std::endl;
                    if (bytesRead > static_cast<ssize_t>(sizeof(buffer) - 1))
                    {
                        bytesRead = sizeof(buffer) - 1;
                    }
                    if (rateLimiter.consume())
                    {
                        Request requestString(request);
                        std::string httpResponse = handleHttpRequest(requestString.getMethod(), requestString.getPath(), requestString.getProtocol());

                        client_write_queues[event.ident].push(httpResponse);

                        struct kevent client_change;
                        EV_SET(&client_change, event.ident, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
                        kevent(kq, &client_change, 1, NULL, 0, NULL);
                    }
                    else
                    {
                        std::string httpResponse = generateHttpResponse(429, "Too Many Requests", "Rate limit exceeded");
                        send(event.ident, httpResponse.c_str(), httpResponse.size(), MSG_NOSIGNAL);  
                    }
                    if (got_signal)
                    {
                        stop();
                        break;
                    }
                }
            }
            else if (event.filter == EVFILT_WRITE)
            {
                // Your new code to handle write events
                std::queue<std::string>& write_queue = client_write_queues[event.ident];
                if (!write_queue.empty())
                {
                    const std::string& message = write_queue.front();
                    send(event.ident, message.c_str(), message.size(), MSG_NOSIGNAL);
                    write_queue.pop();
                }

                if (write_queue.empty())
                {
                    // Remove write filter if nothing to write
                    struct kevent client_change;
                    EV_SET(&client_change, event.ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
                    kevent(kq, &client_change, 1, NULL, 0, NULL);
                }
            }
        }
    }
    close(kq);
    return ;
}