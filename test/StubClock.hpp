#ifndef STUBCLOCK_HPP
# define STUBCLOCK_HPP

#include <ctime>
#include <string>
#include "IClock.hpp"

class StubClock : public IClock
{
private:
	std::time_t	_fixedTime;
	std::string	_fixedText;

public:
	StubClock(std::time_t fixedTime, const std::string& fixedText)
		: _fixedTime(fixedTime), _fixedText(fixedText)
	{
	}

	virtual	~StubClock() {}

	virtual std::time_t	now() const
	{
		return (this->_fixedTime);
	}

	virtual std::string	nowFormatted() const
	{
		return (this->_fixedText);
	}
	
};

#endif /* STUBCLOCK_HPP */