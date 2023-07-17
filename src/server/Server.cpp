/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psuanpro <Marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/16 20:56:29 by psuanpro          #+#    #+#             */
/*   Updated: 2023/07/17 21:05:50 by psuanpro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{

}

Server::Server(Server const & src)
{
    *this = src;
}

Server::~Server()
{

}

Server &	Server::operator=(Server const & rhs)
{
    if (this != &rhs)
    {
        //do something
    }
    return (*this);
}
