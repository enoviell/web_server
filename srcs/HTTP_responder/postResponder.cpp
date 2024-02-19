/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   postResponder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: riccardobordin <riccardobordin@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/11 17:10:40 by riccardobor       #+#    #+#             */
/*   Updated: 2024/01/10 15:09:48 by riccardobor      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/postResponder.hpp"
/*
 * Constructors
 */
postResponder::postResponder( Location *_cgi): httpResponder() {
    this->cgi = _cgi;
}

postResponder::~postResponder() {}

/*
 * Response construction functions
 */
static std::string get_response_body(std::string *cgi_response) {
    if (cgi_response->empty() || cgi_response->size() <= 0)
        return "";
    const size_t start = cgi_response->find("\r\n\r\n");

    if (start == std::string::npos)
        return "";
    std::string res = cgi_response->substr(start + 4, std::string::npos);
    cgi_response->erase(start + 2, std::string::npos);
    return res;
}

static std::vector<std::string>response_header_parse(std::string cgi_response) {
    std::vector<std::string> headers_vec;
    int pos;
    std::string header;
    bool type = false, length = false;

    for (size_t i = 0; i < cgi_response.size();) {
        pos = cgi_response.find("\r\n");
        header = cgi_response.substr(0, pos + 2);
        cgi_response.erase(0, pos + 2);
        headers_vec.push_back(header);
    }

    for (std::vector<std::string>::iterator it = headers_vec.begin(); it != headers_vec.end(); ++it) {
        if (it->find("Content-Type: ") != std::string::npos)
            type = true;
        if (it->find("Content-Length: ") != std::string::npos)
            length = true;
    }
    if ((type && length))
        return headers_vec;
    headers_vec.clear();
    return headers_vec;
}

/*
 * Implementation parent virtual function
 */
void postResponder::answer(HTTPParser pars, Location *location, WebServer *server, std::string server_name, std::string index)
{
    std::string final_answer;
    std::string cgi_response = "";
    std::string response_body = "";
    std::vector<std::string> response_headers;
    std::string upload_script = location->autoindex(cgi->get_cgi_pass_val(), pars.get_content_by_key("PATH"));

    if (upload_script.empty())
        generate_error("404", location, server);

    if(cgi->cgi_handler(upload_script, pars, &cgi_response) == 0) {
        response_body = get_response_body(&cgi_response);
        response_headers = response_header_parse(cgi_response);
        if (response_headers.empty() || response_body.empty()) {
            generate_error("500", location, server);
            return ;
        }
        final_answer.append("HTTP/1.1 200 OK\r\n");
        for (std::vector<std::string>::iterator it = response_headers.begin(); it != response_headers.end(); ++it) {
            final_answer.append(*it);
        }
        final_answer.append("\r\n\r\n");
        final_answer.append(response_body);
        setFullAnswer(final_answer);
    } else {
        std::cerr << "cgi failed\n";
        generate_error("500", location, server);
    }
}
