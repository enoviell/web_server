#include  "../../include/WebServer.hpp"

/*
 * Constructors
 */
WebServer::WebServer() {}

WebServer::~WebServer() {}

/*
 * Setters
 */
void WebServer::set_max_clients(int n_clients) {
    this->max_clients = n_clients;
}

void WebServer::add_error_page(std::string key, std::string path) {
    error_pages.insert(std::pair<std::string, std::string>(key, path));
}

static bool check_socket_params(Socket *socket) {
    if (socket->get_tmp_host() == 0)
        return false;
    if (socket->get_tmp_port() == 0)
        return false;
    return true;
}

void WebServer::add_socket(Socket *new_sock) {
    if (!check_socket_params(new_sock))
        throw InvalidSocketParameters();

    if(Location *l = dynamic_cast<Location *>(new_sock->get_location())) {
        std::string str = new_sock->get_root();
        if (str.at(str.size() - 1) == '/')
            str.erase(str.size() - 1);
        l->set_path(str + l->get_path());
        this->server_sockets.push_back(new_sock);
    }
}

void WebServer::alloc_clients_socket() {
    for (std::vector<Socket *>::iterator it = server_sockets.begin(); it != server_sockets.end(); ++it) {
        (*it)->alloc_clients(this->max_clients);
    }
}

/*
 * Getters
 */
int WebServer::get_max_clients() const {
    return this->max_clients;
}

fd_set WebServer::get_server_fd_set() const {
    return this->server_fd_set;
}

std::map<std::string, std::string> WebServer::get_error_pages() const {
    return this->error_pages;
}

std::vector<Socket *> &WebServer::get_sock_vec() {
    return this->server_sockets;
}
