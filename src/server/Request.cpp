/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psuanpro <psuanpro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/23 00:23:37 by psuanpro          #+#    #+#             */
/*   Updated: 2023/08/23 01:39:02 by psuanpro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() {

}

Request::Request(const std::string& raw_request) {
	std::stringstream	requestStream(raw_request);
	requestStream >> this->_method >> this->_path >> this->_protocol;
	std::string line;
	while (std::getline(requestStream, line) && line != "\r\n") {
		size_t colon_p = line.find(':');
		if (colon_p != std::string::npos) {
			std::string key = line.substr(0, colon_p);
			std::string value = line.substr(colon_p + 2, line.size() - colon_p - 3);
			this->_headers[key] = value;
		}
	}
}

Request::~Request() {

}

const std::string& Request::getMethod() const {
	return this->_method;
}

const std::string& Request::getPath() const {
	return this->_path;
}

const std::string& Request::getProtocol() const {
	return this->_protocol;
}

const std::string&	Request::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = this->_headers.find(key);
	if (it == this->_headers.end())
		throw HeaderNotFound(key);
	return (it->second);
}