#include "server.hpp"

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

	if (bind(socket_fd, (struct sockaddr *) &socket_address, sizeof(socket_address)) < 0)
	{
		cerr << "Error: Bind mode - " << errno << endl;
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
	
		while (keep_alive)
		{

			string      request;
			char        buffer[BUFFER_SIZE];
			int         receivedBytes;
	
			while (request.find("\r\n\r\n") == string::npos)
			{
				receivedBytes = recv(connected_socket_fd, buffer, sizeof(buffer) - 1, 0);
				if (receivedBytes <= 0)
				{
					if (receivedBytes == 0) {
						cout << "Client closed connection" << endl;
					} else {
						cerr << "Error: message not received - " << errno << endl;
					}
					keep_alive = false;
					break;
				}
				buffer[receivedBytes] = '\0';
				request.append(buffer);
				fill(buffer, buffer + sizeof(buffer), '\0');
			}
			
			if (!keep_alive || request.empty()) {
				break;
			}
				
			cout << "\nClient request:\n" << request << endl;

			c_request my_request(request);

			try {
				string connection_header = my_request.get_header_value("Connection");
				if (connection_header.find("close") != string::npos) {
					keep_alive = false;
					cout << "Client requested connection close" << endl;
				}
			} catch (...) {

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