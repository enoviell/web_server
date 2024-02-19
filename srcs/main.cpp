#include <iostream>
#include "../include/WebServer.hpp"
#include "../include/ConfigParsing.hpp"
#include "../include/AstGeneration.hpp"
#include "../include/httpResponder.hpp"
#include "../include/getResponder.hpp"
#include "../include/postResponder.hpp"
#include "../include/deleteResponder.hpp"

WebServer *server_ref_holder;

static void free_ast(ASection *ast) {
    if (ast) {
        if (Section *s = dynamic_cast<Section *>(ast)) {
            for (std::multimap<std::string, ASection *>::iterator it = s->block.begin(); it != s->block.end(); ++it) {
                free_ast(it->second);
            }
            delete s;
        } else if (Directives *d = dynamic_cast<Directives *>(ast)) {
            delete d;
        }
    }
}

static void free_soc_vec(std::vector<Socket *>& sock_vec) {
    std::cout << '\n' << "\033[33m" <<  "closing client/server sockets..." << "\033[0m" << '\n';
    std::cout << '\n' << "\033[33m" <<  "clearing socket location..." << "\033[0m" << '\n';
    std::cout << '\n' << "\033[33m" <<  "clearing client socket array..." << "\033[0m" << '\n';

    for (std::vector<Socket *>::iterator it = sock_vec.begin(); it != sock_vec.end(); it++) {
        (*it)->close_socket(server_ref_holder->get_max_clients());
        delete (*it)->get_location();
        delete[] (*it)->get_client_socket();
        (*it)->string_clear();
        delete *it;
    }
    sock_vec.clear();
}

static void close_server(const int sig) {
    if (sig == SIGINT) {
        std::cout << '\n' << "\033[33m" << "Shutting down the server..." << "\033[0m" << '\n';

        std::cout << '\n' << "\033[33m" << "clearing client sockets..." << "\033[0m" << '\n';
        std::vector<Socket *> &sock_vec = server_ref_holder->get_sock_vec();
        free_soc_vec(sock_vec);

        std::cout << '\n' << "\033[33m" << "clearing fd_set..." << "\033[0m" << '\n';
        fd_set tmp_set = server_ref_holder->get_server_fd_set();
        FD_ZERO(&tmp_set);

        std::cout << '\n' << "\033[33m" << "clearing server_ref_holder..." << "\033[0m" << '\n';
        delete server_ref_holder;

        std::cout << '\n' << "\033[33m" << "closing server..." << "\033[0m" << '\n';
        exit(EXIT_SUCCESS);
    }
}

static httpResponder	*settingResponder(int id, Location *location, std::string pwd)
{
    location->set_pwd(pwd);
    switch (id)
    {
        case 0:
            return new getResponder();
        case 1:
            return new postResponder(location);
        case 2:
            return new deleteResponder();
        default:
            return new getResponder();
    }
}

static void start_sockets(std::vector<Socket *> &sockets) {
    for (std::vector<Socket *>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
        (*it)->initialize_socket();
        (*it)->bind_socket((*it)->get_tmp_port(), (*it)->get_tmp_host());
        (*it)->listening();
    }
}

static void configure_server(std::string config_path, WebServer *server) {
    ConfigParsing parser;
    ASection *ast = new Section();
    try {
        parser.conf_parser_wrapper(config_path);
        tokens_to_ast(parser.get_tokens(), 0, parser.get_tokens_len() - 1, ast);
        parser.clear_vector();
        ast_to_settings(ast, server, NULL);
        server->alloc_clients_socket();
        std::vector<Socket*> &sock_vec = server->get_sock_vec();
        start_sockets(sock_vec);
    } catch (const std::exception &e) {
        std::cerr << "\033[96m ↑ \033[0m Caught an exception while setting the server: " << e.what() << '\n';
        free_ast(ast);
        if (!server->get_sock_vec().empty())
            free_soc_vec(server->get_sock_vec());
        exit(EXIT_FAILURE);
    }
    free_ast(ast);
}

static int select_reposonse_type(std::string method) {
    if (method == "GET")
        return 0;
    if (method == "POST")
        return 1;
    if (method == "DELETE")
        return 2;
    return -1;
}

static void server_loop(WebServer *server, std::string &_pwd) {
    char read_buffer[8192];
    int max_sd, byte_read, sd, max_clients;
    std::vector<Socket *> &server_sockets = server->get_sock_vec();
    std::string porcoddio, pwd, answer, temp;
    HTTPParser req_parser;
    httpResponder *responder;
    pwd = _pwd;
    _pwd.clear();
    max_clients = server->get_max_clients();

    for (std::vector<Socket *>::iterator it = server_sockets.begin(); it != server_sockets.end(); ++it) {
        for (int i = 0; i < max_clients; i++) {
            (*it)->client_socket[i] = 0;
        }
    }

    while (true) {
        req_parser.clear_parser();
        memset(read_buffer, 0, sizeof(read_buffer));
        FD_ZERO(&server->server_fd_set);
        signal(SIGINT, close_server);
        byte_read = 0;
        max_sd = 0;


        // fill fd_set with original sockets
        for (std::vector<Socket *>::iterator it = server_sockets.begin(); it != server_sockets.end(); it++) {
            sd = (*it)->get_fd();
            FD_SET(sd, &server->server_fd_set);

            if (sd > max_sd)
                max_sd = sd;
        }

        // fill fd_set with client sockets
        for (std::vector<Socket *>::iterator it = server_sockets.begin(); it != server_sockets.end(); it++) {
            for (int i = 0; i < max_clients; i++) {
                sd = (*it)->client_socket[i];
                if (sd > 0)
                    FD_SET(sd, &server->server_fd_set);

                if (sd > max_sd)
                    max_sd = sd;
            }
        }

        // select active fd
        try {
            sd = select(max_sd + 1, &server->server_fd_set, NULL, NULL, NULL);
            if (sd < 0 && errno != EINTR)
                throw FailedToSelectSocketFd();
        } catch (const std::exception &exception) {
            std::cerr << "\033[96m ↑ \033[0m Caught an exception while selecting the active socket fd: " << exception.what() << ": ";
            std::cerr << strerror(errno) << '\n';
        }

        // accept connections
        for (std::vector<Socket *>::iterator it = server_sockets.begin(); it != server_sockets.end(); it++) {
            if (FD_ISSET((*it)->get_fd(), &server->server_fd_set)) {
                (*it)->accept_connection();

                for (int i = 0; i < max_clients; i++) {
                    if ((*it)->client_socket[i] == 0) {
                        (*it)->client_socket[i] = (*it)->get_new_fd();
                        break;
                    }
                }
            }
        }

        // read/write on client sockets
        for (std::vector<Socket *>::iterator it = server_sockets.begin(); it != server_sockets.end(); it++) {
            for (int i = 0; i < max_clients; i++) {
                sd = (*it)->client_socket[i];

                req_parser.set_max_body_size((*it)->get_max_body_size());
                if (FD_ISSET(sd, &server->server_fd_set) && sd != (*it)->get_fd()) {
                    porcoddio.clear();
                    int total_read = 0;
                    while (true) {
                        byte_read = 0;
                        memset(read_buffer, 0, sizeof(read_buffer));
                        byte_read = recv(sd, read_buffer, sizeof(read_buffer), MSG_DONTWAIT); //MSG_WAITALL

                        //check if the connection is interrupted
                        if (byte_read <= 0) {
                            break;
                        }
                        total_read += byte_read;
                        porcoddio.append(read_buffer, byte_read);
                        usleep(150);
                    }

                    bool err_400 = false;
                    if (!porcoddio.empty()) {
                        try {
                            req_parser.tokenizer_wrapper(porcoddio);
                            porcoddio.clear();
                            if (std::atoi(req_parser.get_content_by_key("Content-Length").c_str()) != req_parser.get_req_body().size())
                                throw ContentLengthMismatch();
                        } catch (const std::exception &e) {
                            std::cout << "\033[96m ↑ \033[0m Caught an exception while performing tokenization checks: "<< e.what() << '\n';
                            err_400 = true;
                        }
                        if (!err_400) {
                            int type = select_reposonse_type(req_parser.get_content_by_key("METHOD"));
                            responder = settingResponder(type, (*it)->get_location(), pwd);
                            if (type != -1) {
                                if ((*it)->check_method_validity(req_parser.get_content_by_key("METHOD"))) {
                                    if (req_parser.get_req_body().size() >= (*it)->get_max_body_size())
                                        responder->generate_error("413", (*it)->get_location(), server);
                                    else
                                        responder->answer(req_parser, (*it)->get_location(), server, (*it)->get_server_name(), (*it)->get_index());
                                } else {
                                    responder->generate_error("406", (*it)->get_location(), server);
                                }
                            } else {
                                responder->generate_error("501", (*it)->get_location(), server);
                            }
                        } else {
                            responder = settingResponder(-1, (*it)->get_location(), pwd);
                            if (err_400)
                                std::cout << "dafq?\n";
                            responder->generate_error("400", (*it)->get_location(), server);
                        }
                        answer = responder->getFullAnswer();
                        req_parser.clear_parser();
                        responder->clear_full_answer();
                        if (httpResponder *get = dynamic_cast<getResponder *>(responder)) {
                            delete get;
                        } else if (httpResponder *post = dynamic_cast<postResponder *>(responder)) {
                            delete post;
                        } else if (httpResponder *del = dynamic_cast<deleteResponder *>(responder)) {
                            delete del;
                        }

                        long int dataSent;
                        do
                        {
                            temp = answer.substr(0, 32000);
                            dataSent = send(sd, temp.c_str(), temp.length(), 0);
                            if (dataSent < 0) {
                                break;
                            }
                            temp.clear();
                            answer = answer.substr(dataSent);
                        } while (!answer.empty());
                        answer.clear();
                        temp.clear();
                        shutdown(sd, SHUT_RDWR);
                        close(sd);
                        (*it)->client_socket[i] = 0;
                        FD_CLR(sd, &server->server_fd_set);
                    }
                }
            }
        }
    }
}

static std::string get_project_root(char **envp) {
    int i = 0;
    std::string res = "";

    while (envp[i])
    {
        if (envp[i][0] == 'P' && envp[i][1] == 'W' && envp[i][2] == 'D' && envp[i][3] == '=')
        {
            res.append(envp[i]);
            break;
        }
        i++;
    }
    return res;
}

int main(int argc, char **argv, char **envp) {
    std::string config_path;
    WebServer *server = new WebServer;

    std::string pwd = get_project_root(envp);
    pwd = pwd.substr(4);
    server_ref_holder = server;
    if (argc == 2)
        config_path = argv[1];
    else
        config_path = "./config/config.conf";

    configure_server(config_path, server);
    config_path.clear();
    server_loop(server, pwd);
}
