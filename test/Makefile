NAME		= run_tests

CPPUTEST_HOME ?= $(HOME)/goinfre/cpputest

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -g -MMD -MP
CPPFLAGS	= -I../include -isystem $(CPPUTEST_HOME)/include

LDFLAGS		= -L$(CPPUTEST_HOME)/lib
LDLIBS		= -lCppUTestExt -lCppUTest

PROD_SRCDIR		= ../src
OBJDIR		= objs

TEST_SRCS	= AllTest.cpp \
			  SmokeTest.cpp \
			  TestDoublesTest.cpp \
			  IrcCaseMappingTest.cpp

PROD_SRCS	= $(PROD_SRCDIR)/IrcCaseMapping.cpp

TEST_OBJS	= $(TEST_SRCS:%.cpp=$(OBJDIR)/%.o)
PROD_OBJS	= $(PROD_SRCS:../src/%.cpp=$(OBJDIR)/src_%.o)
OBJS		= $(TEST_OBJS) $(PROD_OBJS)
DEPS		= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME) $(LDFLAGS) $(LDLIBS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/src_%.o: ../src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

run: $(NAME)
	./$(NAME) -v

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all run clean fclean re