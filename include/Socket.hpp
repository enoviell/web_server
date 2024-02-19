#ifndef WEB_SERVER_SOCKET_HPP
#define WEB_SERVER_SOCKET_HPP

#include "Location.hpp"
#include "include_wrapper.hpp"

class Socket {
private:
    // socket essentials
    int socket_id;
    int _socket_fd;
    struct sockaddr_in _address;
    int _new_socket;
    int _queue_length;
    std::vector<std::string> allowed_methods;
    std::string _server_name;
    std::string index;

    // indexing params
    uint32_t tmp_addr;
    uint16_t tmp_port;

    std::string root;
    int max_body_size;
    Location *location;
    //TODO var removal
    // it may be correct to remove the 2 var below
    // and set the _address var params int the
    // WebServer::configure_server() function


public:
    int *client_socket;
    //constructors
    Socket();
    ~Socket();

    //socket functions
    void    initialize_socket();
    void    bind_socket(uint16_t port, uint32_t addr);
    void    listening();
    void    accept_connection();
    void    close_socket(int max_clients);
    void    string_clear();

    // getters
    uint16_t    get_tmp_port() const;
    uint32_t    get_tmp_host() const;
    int         get_fd() const;
    int         get_new_fd() const;
    int         *get_client_socket() const;
    bool        check_method_validity(std::string method);
    std::string get_index() const;

    // setters
    void set_tmp_port(int port);
    void set_tmp_host(std::string host);
    void set_server_name(std::string name);
    void set_root(std::string root_path);
    void set_max_body_size(int size);
    void set_location(Location *new_location);
    void alloc_clients(int n);
    void set_methods(std::string methods);
    void set_index(std::string index);

    // getters
    Location *get_location() const;
    std::string get_root() const;
    int get_max_body_size() const;
    struct sockaddr_in get_addr_struct() const;
    std::vector<std::string> get_methods() const;
    std::string get_server_name() const;

    // exceptions
    class BadSocketFdException : public std::exception {
    public:
        virtual const char *what() const throw() {
            return ("BadSocketException");
        }
    };

    class BindException : public std::exception {
    public:
        virtual const char *what() const throw() {
            return ("BindException");
        }
    };

    class FailedToListen : public std::exception {
    public:
        virtual const char *what() const throw() {
            return ("FailedToListen");
        }
    };

    class UnableToAcceptConnection : public std::exception {
    public:
        virtual const char *what() const throw() {
            return ("UnableToAcceptConnection");
        }
    };
};

class FailedToSelectSocketFd : public std::exception {
public:
    virtual const char *what() const throw() {
        return ("FailedToSelectSocketFd");
    }
};

class FailedToReadDataException : public std::exception {
public:
    virtual  const char *what() const throw() {
        return ("FailedToReadDataException");
    }
};

class InvalidIpAddress : public std::exception {
public:
    virtual  const char *what() const throw() {
        return ("InvalidIpAddress");
    }
};

class NotIpv4Address : public std::exception {
public:
    virtual  const char *what() const throw() {
        return ("NotIpv4Address");
    }
};

#endif //WEB_SERVER_SOCKET_HPP
