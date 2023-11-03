/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandle.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchiewli <rchiewli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/18 18:12:46 by psuanpro          #+#    #+#             */
/*   Updated: 2023/11/03 23:22:01 by rchiewli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestHandle.hpp"


HttpRequestHandle::HttpRequestHandle()
{
}

HttpRequestHandle::~HttpRequestHandle()
{
}

HttpRequestHandle &HttpRequestHandle::operator=(const HttpRequestHandle &rhs)
{
	if (this != &rhs)
	{

	}
	return *this;
}

bool	HttpRequestHandle::validateUrlAllow(const std::string& url, const t_serverConf& server, const std::string& vmethod) {
	for (size_t i = 0; i < server.location.size(); i++) {
		if (server.location[i].path == url) {
			if (this->validateMethodAllow(server.location[i].method, vmethod))
				return true;
		}
	}
	return false;
}

bool	HttpRequestHandle::validateMethodAllow(std::vector<std::string> method, const std::string& vmethod) {
	for (size_t i = 0; i < method.size(); i++) {
		if (method[i] == vmethod)
			return true;
	}
	return false;
}

bool	HttpRequestHandle::validateCgi(const std::string& url, const t_serverConf& server) {
	std::string	extention = CgiHandler().getExtensionByUrl(url, server);
	if (extention.empty())
		return false;
	if (url.find(extention) != std::string::npos) {
		return true;
	}
	return false;
}

std::string	HttpRequestHandle::defaultRoute(const std::string& url, const t_serverConf& server) {

	std::string	path = CgiHandler().getRootByUrlFromServer(url, server);
	path += "/";
	path += CgiHandler().getDefaultFileByUrl(url, server);
	std::string file = File(path).getContent();
	if (file.empty()) {
		return Response(404, "<h1>Not Found</h1>").HttpResponse();
	}
	return Response(200, file).HttpResponse();
}

std::string	HttpRequestHandle::callCgiGet(const std::string& url, const Request& req, const t_serverConf& server)
{
	CgiHandler cgi;
	std::string	path_file;

	path_file += cgi.getRootByUrlFromServer(url, server);
	path_file += url;
	cgi.initArgv(cgi.getExecuteByUrl(url, server), path_file);
	StringMatrix	argv(cgi.getArgv());
	StringMatrix	env(req.getQueryUrl());
	return cgi.executeCgi(argv, env);
}

std::string	HttpRequestHandle::allRoute(const std::string& url, const Request& req, const t_serverConf& server) {

	t_detail	loc = CgiHandler().getAllLocation(url, server);
	std::string	path;

	if (loc.status && !this->validateMethodAllow(loc.location.method, "GET")) {
		return Response(405).HttpResponse();
	}

	path = CgiHandler().getRootByUrlFromServer(url, server);
	if (path.empty()) {
		path = CgiHandler().getRootDefaultByUrlFromServer(server);
	}
	if (!loc.location.redirection.empty())
		path += loc.location.redirection;
	else
		path += req.getUrl();
	std::string	file = File(path).getContent();
	if (file.empty()) {
		return Response(404).HttpResponse();
	}
	return Response(200, file).HttpResponse();
}

std::string	HttpRequestHandle::generateHtml(const std::string& title, const std::string& content) {
	std::stringstream	html;
	html << "<!DOCTYPE html>\n";
	html << "\t<html lang=\"en\">\n";
	html << "\t\t<head>\n";
	html << "\t\t\t<meta charset=\"UTF-8\">\n";
	html << "\t\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	html << "\t\t\t<title>" << title << "</title>\n";
	html << "\t\t</head>\n";
	html << "<body>\n";
	html << content << '\n';
	html << "</body>\n";
	html << "</html>\n";
	return html.str();
}

std::string	HttpRequestHandle::openDir(const std::string& url) {
	DIR *dir;
	std::stringstream ss;

	struct dirent *ent;
	dir = opendir(url.c_str());
	if (dir != nullptr) {
		 while ((ent = readdir(dir)) != nullptr) {
			ss << "<h3>" << ent->d_name  << "</h3>" << std::endl;
		}
		closedir(dir);
		std::string	content = this->generateHtml("Directory listing", ss.str());
		return Response(200, content).HttpResponse();
	}
	return Response(404).HttpResponse();
}

std::string	HttpRequestHandle::dirListing(const std::string& url, const t_serverConf& server) {
	std::string	dir_path;

	if (!server.dirListing)
		return Response(403).HttpResponse();
	if (url == "//")
		dir_path = server.serverroot;
	else {
		dir_path = server.serverroot;
		dir_path += url;
	}
	return this->openDir(dir_path);
}


bool	HttpRequestHandle::isDirlist(const std::string& url) {
	if (url == "//")
		return true;
	if (url != "/" && url[url.size() - 1] == '/')
		return true;
	return false;
}

std::string	HttpRequestHandle::getMethodRoute(const std::string& url, const Request& req, const t_serverConf& server) {
	if (this->validateCgi(url, server)) {
		return this->callCgiGet(url, req, server);
	}else {
		if (url == "/" || url == "/index.html") {
			return this->defaultRoute(url, server);
		}
		else if (this->isDirlist(url)) {
			return this->dirListing(url, server);
		}
		else {
			return this->allRoute(url, req, server);
		}
	}
	return	Response(404, "<h1> Get not found </h1>").HttpResponse();
}

t_detail	HttpRequestHandle::validateHostRequestAndGetServer(Request& req, const t_con& config) {

	std::string host = req.getHeadersByValue("Host");
	if (host.find("localhost") != std::string::npos) {
		std::string	port = host.substr(host.find_last_of(':') + 1);
		for (size_t i = 0; i < config.server.size(); i++) {
			if (config.server[i].port == atoi(port.c_str()))
				return Detail(true, port, config.server[i]).getDetailStruct();
		}
		return Detail(false).getDetailStruct();
	}
	else {
		for (size_t i = 0; i < config.server.size(); i++) {
			if (config.server[i].serverName == host)
				return Detail(true, config.server[i]).getDetailStruct();
		}
	}
	return Detail(false).getDetailStruct();
}

std::string	HttpRequestHandle::getMethod(const Request& req, const t_serverConf& config) {
	return this->getMethodRoute(req.getUrl(), req, config);
}

bool	HttpRequestHandle::validateAllowDelete(const t_serverConf& server) {
	if (!server.allow_delete)
		return false;
	return true;
}

std::string	HttpRequestHandle::deleteMethod(const std::string& url, const t_serverConf& server) {
	if (this->validateAllowDelete(server))
		return Response(405, "<h1>Method not allow</h1>").HttpResponse();
	std::string	path = CgiHandler().getRootDefaultByUrlFromServer(server);
	path += url;
	int status = File().deleteFile(path);
	if (status == -1)
		return Response(404).HttpResponse();
	else if (status == -2)
		return Response(500).HttpResponse();
	return Response(200).HttpResponse();
}

std::string	HttpRequestHandle::postMethod(const Request& req, const t_serverConf& server) {


	t_detail	loc = CgiHandler().getAllLocation(req.getUrl(), server);
	CgiHandler cgi;
	std::string	path;

	if (loc.status && !this->validateMethodAllow(loc.location.method, "POST")) {
		return Response(405).HttpResponse();
	}
	path = CgiHandler().getRootByUrlFromServer(req.getUrl(), server);
	path += req.getUrl();
	std::string save_path = loc.location.upload.savePath;
	cgi.initArgv(cgi.getExecuteByUrl(req.getUrl(), server), path);
	StringMatrix	argv(cgi.getArgv());
	StringMatrix	env(req.getHeaderC(), save_path);
	std::string		content;
	if (!req.getBoundary().empty())
		content = req.getContent();
	else
		content = req.getBody();


	return cgi.executeCgi(argv, env, content);
}

std::string	HttpRequestHandle::validateMethod(const Request& req,const t_con& config) {

	t_detail tmp = this->validateHostRequestAndGetServer(const_cast<Request&>(req), config);
	if (tmp.status) {
		if (req.getMethod() == "GET") {
			return this->getMethod(req, tmp.server);
		}
		else if (req.getMethod() == "POST") {
			return this->postMethod(req, tmp.server);
		}
		else if (req.getMethod() == "DELETE") {
			return this->deleteMethod(req.getUrl(), tmp.server);
		}
	}
	return Response(404, req.getMethod()).HttpResponse();
}


int HttpRequestHandle::getPortFromRequest(Request req)
{
	std::string hostHeader = req.getHeadersByValue("Host");
	std::string::size_type colonPos = hostHeader.find_last_of(':');
	if (colonPos != std::string::npos)
	{
		std::string portStr = hostHeader.substr(colonPos + 1);
		std::istringstream iss(portStr);
		int port;
		iss >> port;
		if (!iss.fail())
		{
			return port;
		}
	}
	// Return -1 or any other value to indicate that the port wasn't found
	return -1;
}


