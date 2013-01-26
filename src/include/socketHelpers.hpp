#ifndef SOCKETHELPERS_HPP
#define SOCKETHELPERS_HPP

//#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

ssize_t socket_read(int fd, char *buf, size_t len, time_t timeout) {
    fd_set sockSet;
    struct timeval tv;
    
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    //std::cerr << "socket_read: timeout = " << tv.tv_sec << std::endl;

    ssize_t ret;
    size_t tot_read = 0;
    do {
	FD_ZERO(&sockSet);
	FD_SET(fd, &sockSet);
	ret = select(fd + 1, &sockSet, NULL, NULL, &tv);
	//std::cerr << "select() returned " << ret << "..." << std::endl;
	if (ret == -1) {
	    if (errno == EINTR) {
		// interrupted by signal...
		return -1;
	    }
	    perror("select() error");
	    exit(1);
	} else if (ret == 0) {
	    /*
	    std::cerr << "select() timeout..." << std::endl;
	    exit(1);
	    */
	    return tot_read;
	}

	if (!FD_ISSET(fd, &sockSet)) {
	    //std::cerr << "Weird...there was 1 descriptor in the read_set, select returned " << ret << ", but that descriptor is not in the returned set...";
	    exit(1);
	}

	ret = recv(fd, buf + tot_read, len - tot_read, 0);
	//std::cerr << "recv() returned " << ret << "..." << std::endl;
	if (ret > 0) {
	    tot_read += ret;
	} else if (ret == 0) {
	    // connection reset
	    return -1;
	} else {
	    if (errno == EINTR) {
		// interrupted by signal...
		return -1;
	    }
	    perror("recv() error");
	    exit(1);
	}
    } while (tot_read != len);

    return tot_read;
}

ssize_t socket_write(int fd, const char *buf, size_t len, time_t timeout) {
    fd_set sockSet;
    struct timeval tv;
    
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    //std::cerr << "socket_write: timeout = " << tv.tv_sec << std::endl;
    //std::cerr << "socket_write: len = " << len << std::endl;

    ssize_t ret;
    size_t tot_wrote = 0;
    do {
	FD_ZERO(&sockSet);
	FD_SET(fd, &sockSet);
	ret = select(fd + 1, NULL, &sockSet, NULL, &tv);
	//std::cerr << "select() returned " << ret << "..." << std::endl;
	if (ret == -1) {
	    if (errno == EINTR) {
		// interrupted by signal...
		return -1;
	    }
	    perror("select() error");
	    exit(1);
	} else if (ret == 0) {
	    /*
	    std::cerr << "select() timeout..." << std::endl;
	    exit(1);
	    */
	    return tot_wrote;
	}

	if (!FD_ISSET(fd, &sockSet)) {
	    //std::cerr << "Weird...there was 1 descriptor in the write_set, select returned " << ret << ", but that descriptor is not in the returned set...";
	    exit(1);
	}

	ret = send(fd, buf + tot_wrote, len - tot_wrote, MSG_NOSIGNAL);
	//std::cerr << "send() returned " << ret << "..." << std::endl;
	if (ret > 0) {
	    tot_wrote += ret;
	} else if (ret == 0) {
	    // connection reset
	    return -1;
	} else {
	    if (errno == EINTR || errno == EPIPE) {
		// interrupted by signal...or broken pipe
		return -1;
	    }
	    perror("send() error");
	    exit(1);
	}
    } while (tot_wrote != len);

    return tot_wrote;
}

#endif
