#include "server.hpp"
#include <sys/time.h>
#include <netinet/tcp.h>

int main(void)
{
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in socket_address = sockaddr_in();

	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons(8080);
	socket_address.sin_addr.s_addr = INADDR_ANY;

	if (socket_fd < 0){
		cerr << "Error: Socket creation - " << errno << endl;
		return (-1);
	}
	/* Pour réutiliser le même port a la suite*/
	int socket_option = 1;

	// socket fd: le socket sur lequel on agit
	// SOL SOCKET : indique qu;on agit sur une topion au niveau du socket
	// SO REUSEADDR : option qu'on veut activer
	// &opt " un pointeur vers la valeur de l'option - 1 pour l'activer"
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option)) < 0) {
		cerr << "Error: socket option - Reusable address - " << errno << endl;
		close(socket_fd);
		return (-1);
	}
	
	if (bind(socket_fd, (struct sockaddr *) &socket_address, sizeof(socket_address)) < 0)
	{
		cerr << "Error: Bind: Address already used - " << errno << endl;
		return (-1);
	}

	if (listen(socket_fd, SOMAXCONN) < 0){
		cerr << "Error: Listening mode - " << errno << endl;
		return (-1);
	}

	int connected_socket_fd;
	c_response response_handler;

	while (true)
	{
		cout << "Waiting for connections..." << endl;

		socklen_t addrlen = sizeof(socket_address);
		connected_socket_fd = accept(socket_fd, (struct sockaddr *) &socket_address, &addrlen);
	
		if (connected_socket_fd < 0){
			cerr << "Error: Accepting mode - " << errno << endl;
			return (-1);
		}
		cout << "New client connected !" << endl;

		bool keep_alive = true;
	
		/* On applique un timeout pour accélérer le chargement des images */
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		setsockopt(connected_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		
		while (keep_alive)
		{
			string request;
			char buffer[BUFFER_SIZE];
			bool headers_complete = false;

			/* Lire les données jusqu'à avoir les headers complets */
			while (!headers_complete && keep_alive)
			{
				int receivedBytes = recv(connected_socket_fd, buffer, sizeof(buffer) - 1, 0);

				if (receivedBytes <= 0) {
					if (receivedBytes == 0) {
						cout << "Client closed connection" << endl;
					} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
						cout << "Error: Timeout - no data received" << endl;
					} else {
						cerr << "Error: message not received - " << errno << endl;
					}
					keep_alive = false;
					break;
				}
				
				buffer[receivedBytes] = '\0';
				request.append(buffer, receivedBytes);

				if (request.find("\r\n\r\n") != string::npos)
					headers_complete = true;
			}
			
			if (!keep_alive || request.empty()) {
				break;
			}
			
			string first_line = request.substr(0, request.find('\r'));
			c_request my_request(request);

			try {
				string connection_header = my_request.get_header_value("Connection");
				if (connection_header.find("close") != string::npos) {
					keep_alive = false;
					cout << "Client requested connection close" << endl;
				}
			} catch (const std::exception &e) {
				cerr << "Exception caught: " << e.what() << endl;
			}
			
			response_handler.define_response_content(my_request);

			const string &response = response_handler.get_response();
			if (send(connected_socket_fd, response.data(), response.size(), 0) == -1)
			{
				cerr << "Error: Message not sent - " << errno << endl;
				keep_alive = false;
				break;
			}
			if (!keep_alive)
				break;
		}
		close(connected_socket_fd);		
	}
	close(socket_fd);
	return (0);
}
