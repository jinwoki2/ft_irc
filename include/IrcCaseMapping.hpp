#ifndef IRCCASEMAPPING_HPP
# define IRCCASEMAPPING_HPP

# include <string>

class IrcCaseMapping
{
private:

public:
	static std::string	normalize(const std::string& str);
	static bool			equals(const std::string& a, const std::string& b);
	static bool			isValidNickname(const std::string& nick);
	static bool			isValidChannelName(const std::string& channelName);
};


#endif /* IRCCASEMAPPING_HPP */