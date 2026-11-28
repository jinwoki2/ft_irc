#ifndef ISENDER_HPP
# define ISENDER_HPP

#include <string>

class ISender
{
public:
	virtual ~ISender() {}
	virtual void	send(int fd, const std::string& data) = 0;
	virtual void	close(int fd) = 0;  
};

#endif /* ISENDER_HPP */