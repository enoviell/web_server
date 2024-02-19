/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   postResponder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: riccardobordin <riccardobordin@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/11 17:08:12 by riccardobor       #+#    #+#             */
/*   Updated: 2024/01/10 14:58:12 by riccardobor      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTRESPONDER_HPP
# define POSTRESPONDER_HPP

#include "httpResponder.hpp"
#include "Location.hpp"

class	postResponder : public httpResponder
{
	public:
		postResponder(Location *_cgi);
		~postResponder();
		void	answer(HTTPParser pars, Location *location, WebServer *server, std::string server_name, std::string index);
private :
    Location *cgi;
};

#endif
