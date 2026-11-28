#ifndef SPYSENDER_HPP
# define SPYSENDER_HPP

#include <string>
#include <vector>

#include "ISender.hpp"

struct SentPacket
{
	int				fd;
	std::string		data;

	SentPacket(int fd, const std::string& payload)
		: fd(fd), data(payload)
	{
	}
};

class SpySender : public ISender
{
public:
	std::vector<SentPacket>	sent;
	std::vector<int>		closed;

	virtual ~SpySender() {}

	virtual void	send(int fd, const std::string& data)
	{
		sent.push_back(SentPacket(fd, data));
	}

	virtual void	close(int fd)
	{
		closed.push_back(fd);
	}
};

#endif /* SPYSENDER_HPP */