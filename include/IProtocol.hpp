#ifndef IPROTOCOL_HPP
# define IPROTOCOL_HPP

# include <string>

class ISender;

class IProtocol
{
public:
	virtual ~IProtocol() {}
	virtual void	setSender(ISender* sender) = 0;
	virtual void	onConnect(int fd, const std::string& host) = 0;
	virtual	void	onMessage(int fd, const std::string& line) = 0;
	virtual	void	onDisconnect(int fd) = 0;
};

#endif /* IPROTOCOL_HPP */