#ifndef QUERYCLIENT_HPP
#define QUERYCLIENT_HPP

#include <string>
#include <sys/socket.h>
#include <sys/un.h>

class QueryClient {
  public:
    virtual ~QueryClient();

    bool establishConnection();

  protected:
    std::string sockPath;
    size_t sockTimeout;
    int conn_fd;
};

inline QueryClient::~QueryClient()
{
    close(conn_fd);
}

inline bool QueryClient::establishConnection() {
    sockaddr_un conn;

    conn_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (conn_fd == -1) {
	// Error creating socket
	perror("NameResolution::establishConnection(): socket() error");
	return false;
    }
    
    // connect the socket.
    conn.sun_family = AF_LOCAL;
    strcpy(conn.sun_path, sockPath.c_str());
    socklen_t len = sizeof(conn.sun_len) + sizeof(conn.sun_family) + strlen(conn.sun_path);
    if (connect(conn_fd, (struct sockaddr *) &conn, len) == -1) {
	// error...
	perror("NameResolution::establishConnection(): connect() error");
	return false;
    }

    return true;
}

#endif
