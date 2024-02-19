/* ************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: riccardobordin <riccardobordin@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/11 16:21:12 by riccardobor       #+#    #+#             */
/*   Updated: 2024/01/10 14:58:29 by riccardobor      ###   ########.fr       */
/*                                                                            */
/* ************************** */

#ifndef HTTPRESPONDER_HPP
# define HTTPRESPONDER_HPP

# include "include_wrapper.hpp"
# include "HTTPParser.hpp"
#include "Location.hpp"
#include "WebServer.hpp"

struct responderInfo
{
    std::string	contentType;
    int			contentLenght;
};

class HTTPParser;

class httpResponder
{
private:
    std::map<std::string, std::string>	header;
    std::string							body;
    responderInfo						info;
    int									status;
    int									socket;
    std::string                         full_answer;

public:
    httpResponder();
    virtual 							~httpResponder();
    void								setStatus(int x);
    int									getStatus();
    int									getSocket();
    std::string                         getFullAnswer() const;
    std::string                         generateFullAnswer(std::string file_path, std::string _body, std::string to_find, std::string server_name);
    void								setBody(std::string x);
    std::map<std::string, std::string>	getHeader();
    std::string                         getBody();
    std::string                         getHeaderString();
    void								setHeader(std::string key, std::string value);
	void								setFullAnswer(std::string _ans);
	virtual void						answer(HTTPParser pars, Location *location, WebServer *server, std::string server_name, std::string index) = 0;
    virtual void                        generate_error(std::string error_code, Location *location, WebServer *server);
    void                                clear_full_answer();

};

class UnableToOpenDir : public std::exception {
public:
    virtual const char *what() const throw() {
        return "UnableToOpenDir";
    }
};

#endif
