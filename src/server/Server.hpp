#ifndef SERVER_HPP
# define SERVER_HPP

# include "../stdlib.hpp"
# include "HttpRequestHandle.hpp"
# include "Response.hpp"
# include "../parsing/Conf.hpp"

# ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
# endif

extern volatile sig_atomic_t got_signal;

class Server
{
	public:
		Server();
		Server(const Conf& config);
		~Server();
		void start();
		void stop();
		std::map<int, std::queue<std::string> > client_write_queues;
		static volatile sig_atomic_t got_signal;

    private:
		Conf 				_config;
		std::vector<int> 	_serverSockets;
        bool				_running;
		int 				_currentClientCount;
        int					_server_fd;
        std::set<int> 		active_clients;
		std::map<int, int> 	client_to_port_map;
	// 	// struct kevent events;
	// 	int 	MAX_CLIENTS;
	// 	// std::vector<int> ports;
	// 	std::map<int, std::string> client_buffers;
	// 	// std::vector<std::pair<int, std::string> > serverPortNamePairs;
	// 	std::set<int> checked_hostnames;

		static void 	signal_handler(int sig);
		void 			setupSignalHandlers();
		int  			setupKqueue();
		void			setupServerSockets(int kq, size_t NUM_SERVERS);
		void 			mainEventLoop(int kq, size_t NUM_SERVERS);
		void 			closeServerSocket();
		void 			handleEventError(struct kevent& event);
		void			handleNewClientConnection(int kq, int eventIdent);
		void 			closeActiveClients();
		void 			handleRead(int kq, struct kevent& event);
		std::string 	handleHttpRequest(const std::string& method, const std::string& url, Request& request, int client_socket);
		void 			handleWrite(int kq, int eventIdent);
		void 			handleClientDisconnection(int kq, int eventIdent);
	// 	void setNonBlocking();
	// 	std::string extractHostHeader(const std::string& request);
	// 	std::string handleFileDownloadRequest();
		std::string 	createSimpleHttpResponse();
};

#endif
