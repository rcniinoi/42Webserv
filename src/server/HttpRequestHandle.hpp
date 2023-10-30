/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandle.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: psuanpro <psuanpro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/18 18:12:48 by psuanpro          #+#    #+#             */
/*   Updated: 2023/10/30 10:33:03 by psuanpro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTHANDLE_HPP
# define HTTPREQUESTHANDLE_HPP

# include "../stdlib.hpp"
# include "../struct.hpp"
# include "Response.hpp"
# include "Request.hpp"
# include "Store.hpp"
# include "../parsing/Conf.hpp"
# include "../cgihandler/CgiHandler.hpp"


struct ServerConfig
{
	int port;
	std::string name;
	std::string landingPagePath;
};

class HttpRequestHandle
{
	private:
		Request									_req;
		std::string								_cgi_path;
		std::string								_method;
		std::vector<ServerConfig>				_serverConfigs;
	public:
		HttpRequestHandle();
		~HttpRequestHandle();
		HttpRequestHandle &operator=(const HttpRequestHandle & rhs);

		std::string					validateMethod(const Request& req, const t_con& config);
		bool						validateMethodAllow(std::vector<std::string> method, const std::string& vmethod);
		bool						validateUrlAllow(const std::string& url, const t_serverConf& server, const std::string& vmethod);
		bool						validateCgi(const std::string& url, const t_serverConf& server);
		t_detail					validateHostRequestAndGetServer(Request& req, const t_con& server);


		std::string					callCgiGet(const std::string& url, const Request& req, const t_serverConf& server);
		std::string					defaultRoute(const std::string& url, const t_serverConf& server);
		std::string					allRoute(const std::string& url, const Request& req,const t_serverConf& server);


		std::string					getMethodRoute(const std::string& url, const Request& req, const t_serverConf& config);
		std::string					getMethod(const Request& req, const t_serverConf& config);
		int							getPortFromRequest(Request req);
		const std::string&			postMethod(const Request& req, Store *st);
		const std::string&			deleteMethod(const Request& req);
		std::string					readFile(std::stringstream& path);
};

#endif
