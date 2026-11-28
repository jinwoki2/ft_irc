#ifndef ICLOCK_HPP
# define ICLOCK_HPP

# include <ctime>
# include <string>

class IClock
{
public:
	virtual	~IClock() {}
	virtual std::time_t	now() const = 0;
	virtual std::string	nowFormatted() const = 0;
};

#endif /* ICLOCK_HPP */