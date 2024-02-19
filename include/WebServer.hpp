/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   web_server.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: riccardobordin <riccardobordin@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/04 14:06:55 by riccardobor       #+#    #+#             */
/*   Updated: 2024/01/10 16:18:30 by riccardobor      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Socket.hpp"

class WebServer {
private:
    std::vector<Socket *> server_sockets;
    int max_clients;
    std::map<std::string, std::string> error_pages;
public:

    // constructors
    WebServer();
    ~WebServer();
    fd_set server_fd_set;

    // server functions
    void configure_server();
    void server_loop();
    void close_server();

    // setters
    void set_max_clients(int n_clients);
    void add_error_page(std::string key, std::string path);
    void add_socket(Socket *new_sock);
    void alloc_clients_socket();

   // getters
    int get_max_clients() const;
    std::vector<Socket *> &get_sock_vec();
    int *get_clients_socket() const;
    fd_set get_server_fd_set() const;
//    std::string get_index() const;

    // exceptions
    std::map<std::string, std::string> get_error_pages() const;
    class badReading : public std::exception {
    public:
        const char *what(void) const throw() {
            return ("Error: something went wrong reading\n");
        }
    };
    class InvalidSocketParameters : public std::exception {
    public:
        const char *what(void) const throw() {
            return ("InvalidSocketParameters\n");
        }
    };
};

class ContentLengthMismatch : public std::exception {
public:
    const char *what(void) const throw() {
        return ("ContentLengthMismatch\n");
    }
};

#endif
