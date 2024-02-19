#include "../../include/Socket.hpp"

/*
 * Constructors
 */
Socket::Socket(){}

Socket::~Socket() {}

/*
 * Socket functions
 */
void Socket::initialize_socket() {
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd == -1 && errno != EINTR)
        throw BadSocketFdException();
}

void Socket::bind_socket(uint16_t port, uint32_t addr) {
    int const option_value = 1;

    setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));

    memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = htonl(addr);
    _address.sin_port = htons(port);
    memset(_address.sin_zero, '\0', sizeof(_address.sin_zero));

    if (bind(_socket_fd, (struct sockaddr *) &_address, sizeof(_address)) < 0 && errno != EINTR)
        throw BindException();
    std::cout << "\033[32mBind succesfull\033[0m\n";
    }


void Socket::listening() {
    this->_queue_length = 1;
    if (listen(_socket_fd, _queue_length) == -1 && errno != EINTR)
        throw FailedToListen();
    std::cout << "\033[32mSocket in listening state\033[0m\n";
}

void Socket::accept_connection() {
    try {
        socklen_t size;

        size = sizeof(_address);
        if((_new_socket = accept(_socket_fd, (struct sockaddr *) &_address, &size)) < 0 && errno != EINTR)
            throw UnableToAcceptConnection();
    } catch (std::exception &exception) {
        std::cerr << "\033[96m ↑ \033[0m Caught an exception while accepting a new connection: " << exception.what() << ": ";
        std::cerr << strerror(errno) << '\n';
    }
}

bool Socket::check_method_validity(std::string method) {
    for (std::vector<std::string>::iterator it = this->allowed_methods.begin(); it != this->allowed_methods.end(); ++it) {
        if (*it == method)
            return true;
    }
    return false;
}

/*
 * Clear functions
 */
void Socket::close_socket(int max_clients) {
    shutdown(this->_socket_fd, SHUT_RDWR);
    close(this->_socket_fd);
    if (_new_socket > 0) {
        shutdown(this->_new_socket, SHUT_RDWR);
        close(this->_new_socket);
    }
    for (int i = 0; i < max_clients; i++) {
        shutdown(client_socket[i], SHUT_RDWR);
        close(client_socket[i]);
    }
}

void Socket::string_clear() {
    this->root.clear();
    this->_server_name.clear();
}

/*
 * Setters
 */
void Socket::alloc_clients(int n) {
    this->client_socket = new int[n];
}

void Socket::set_max_body_size(int size) {
    this->max_body_size = size;
}

void Socket::set_tmp_port(int port) {
    this->tmp_port = port;
}

static u_int32_t string_to_byte_order(const char *ip_addr) {
    int octets[4];
    size_t i = 0;
    int j = 0;
    int h = 0;
    char tmp[3];

    if (ip_addr == NULL)
        return 0;

    while (i < std::strlen(ip_addr) + 1) {
        if (ip_addr[i] == '.' || i == std::strlen(ip_addr)) {
            tmp[j] = 0;
            octets[h] = std::atoi(tmp);
            h++;
            j = 0;
            i++;
            continue;
        }
        tmp[j] = ip_addr[i];
        i++;
        j++;
    }
    for (i = 0; i < 4; i++) {
        if (octets[i] < 0 || octets[i] > 255)
            return 0;
    }
    return ((u_int32_t)octets[0] << 24) | ((u_int32_t)octets[1] << 16) | ((u_int32_t)octets[2] << 8) | ((u_int32_t)octets[3]);
}

static bool check_if_ipv4(std::string str) {
    int dots = 0;

    if (str.empty())
        return false;

    for (size_t i = 0; i < str.size(); i++) {
        if (str.at(i) == '.')
            dots++;
    }

    if (dots != 3)
        return false;

    int tmp, n_nums = 0;
    const char *c_str = str.c_str();
    std::string tmp_num;
    std::stringstream ss;
    size_t i = 0;

    while (i < std::strlen(c_str) + 1) {
        if (c_str[i] == '.' || i == std::strlen(c_str)) {
            tmp_num.append(ss.str());
            ss.str("");
            tmp = std::atoi(tmp_num.c_str());
            tmp_num.clear();
            n_nums++;
            i++;
            if (tmp < 0 || tmp > 255)
                return false;
            continue;
        }
        ss << c_str[i];
        i++;
    }
    tmp_num.clear();
    if (n_nums != 4)
        return false;
    return true;
}

void Socket::set_tmp_host(std::string host) {
    if (host.empty())
        throw InvalidIpAddress();
    if (!check_if_ipv4(host))
        throw NotIpv4Address();
    for (std::string::iterator it = host.begin(); it != host.end(); it++) {
        if (*it == '.')
            continue;
        if (std::isdigit(*it) == 0)
            throw InvalidIpAddress();
    }
    this->tmp_addr = string_to_byte_order(host.c_str());
}

void Socket::set_server_name(std::string name) {
    std::stringstream ss;

    ss << this->tmp_port;
    this->_server_name = name + ":" + ss.str();
    ss.clear();
}

void Socket::set_root(std::string root_path) {
    this->root = root_path;
}

void Socket::set_location(Location *new_location) {
    this->location = new_location;
}

void Socket::set_methods(std::string methods) {
    size_t pos;
    std::string tmp;

    pos = methods.find(',');
    tmp = methods.substr(0, pos);
    methods.erase(0, pos + 1);
    this->allowed_methods.push_back(tmp);
    tmp.clear();
    while (pos != std::string::npos) {
        pos = methods.find(',');
        tmp = methods.substr(0, pos);
        methods.erase(0, pos + 1);
        this->allowed_methods.push_back(tmp);
        tmp.clear();
    }
}

void Socket::set_index(std::string index) {
    this->index = index;
}

/*
 * Getters
 */
Location *Socket::get_location() const {
    return this->location;
}

int Socket::get_max_body_size() const {
    return this->max_body_size;
}

struct sockaddr_in Socket::get_addr_struct() const {
    return this->_address;
}

std::string Socket::get_root() const {
    return this->root;
}

int Socket::get_fd() const {
    return this->_socket_fd;
}

int Socket::get_new_fd() const {
    return this->_new_socket;
}

int *Socket::get_client_socket() const {
    return this->client_socket;
}

in_port_t Socket::get_tmp_port() const {
    return this->tmp_port;
}

in_addr_t Socket::get_tmp_host() const {
    return this->tmp_addr;
}

std::string Socket::get_index() const {
    return this->index;
}

std::string Socket::get_server_name() const {
    return this->_server_name;
}