#include "IrcCaseMapping.hpp"

/* ------------------------------------------------------------------------- */
//	normalize, equals	  
/* ------------------------------------------------------------------------- */

static bool	isUpper(char c)
{
	return ('A' <= c && c <= 'Z');
}

static bool	isScandinavianUpper(char c)
{
	return (('[' <= c && c <= ']') || (c == '~'));
}

static void	convertUpperToLower(char& c)
{
	if (c == '~')
		c = c - ('a' - 'A');
	else
		c = c + ('a' - 'A');
}

static std::string	toLower(const std::string& str)
{
	std::string	res(str);

	for (std::string::iterator it = res.begin(); it != res.end(); ++it) {
		if (isUpper(*it) || isScandinavianUpper(*it))
			convertUpperToLower(*it);
	}

	return (res);
}

std::string	IrcCaseMapping::normalize(const std::string& str)
{
	return (toLower(str));
}

bool	IrcCaseMapping::equals(const std::string& a, const std::string& b)
{
	return (normalize(a) == normalize(b));
}

/* ------------------------------------------------------------------------- */
//	isValidNickname
/* ------------------------------------------------------------------------- */

static bool	isValidLength(const std::string& nick)
{
	return (!nick.empty() && !(nick.size() > 9));
}

static bool	isLetter(char c)
{
	return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));	
}

static bool	isDigit(char c)
{
	return ('0' <= c && c <= '9');
}

static bool	isSpecial(char c)
{
	return (('[' <= c && c <= '`') || ('{' <= c && c <= '}'));
}

static bool	isHyphen(char c)
{
	return (c == '-');
}

static bool	isValidFirstChar(const std::string& nick)
{
	return (isLetter(nick[0]) || isSpecial(nick[0]));
}

static bool	isValidSetOfAscii(const std::string& nick)
{
	for (std::string::const_iterator it = nick.begin(); it != nick.end(); ++it) {
		if (!(isLetter(*it) || isDigit(*it) || isSpecial(*it) || isHyphen(*it)))
			return (false);
	}
	return (true);
}

bool	IrcCaseMapping::isValidNickname(const std::string& nick)
{
	return (isValidLength(nick) &&
			isValidFirstChar(nick) &&
			isValidSetOfAscii(nick));
}

/* ------------------------------------------------------------------------- */
//	isValidChannelName  
/* ------------------------------------------------------------------------- */

static bool	isValidChannelNameLength(const std::string& channelName)
{
	return (2 <= channelName.size() && channelName.size() <= 50);
}

static bool	isValidChannelNamePrefix(const char& c)
{
	return (c == '#' || c == '&');
}

static bool	isValidSetOfChannelNameChar(const std::string& channelName)
{
	std::string setOfInvalidChar("\0\x07\n\r ,:", 7);

	for (std::string::const_iterator it = channelName.begin(); it != channelName.end(); ++it){
		if (setOfInvalidChar.find(*it) != std::string::npos)
			return (false);
	}
	return (true);
}

bool	IrcCaseMapping::isValidChannelName(const std::string& channelName)
{
	return (isValidChannelNameLength(channelName) &&
			isValidChannelNamePrefix(channelName[0]) &&
			isValidSetOfChannelNameChar(channelName));
}
