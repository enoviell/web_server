/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   getResponder.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: riccardobordin <riccardobordin@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/11 17:02:50 by riccardobor       #+#    #+#             */
/*   Updated: 2024/01/10 14:58:12 by riccardobor      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETRESPONDER_HPP
# define GETRESPONDER_HPP

#include "httpResponder.hpp"
#include "Location.hpp"
#include <dirent.h>
#include <fstream>
#include <iostream>
class	getResponder : public httpResponder
{
	public:
		getResponder();
		~getResponder();
		void	answer(HTTPParser pars, Location *location, WebServer *server, std::string server_name, std::string index);
class UnableToOpenDir : public std::exception {
public:
    virtual const char *what() const throw() {
        return "UnableToOpenDir";
    }
};
};

#endif
