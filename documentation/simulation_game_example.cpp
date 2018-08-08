#include<vector>
#include<random>
#include<cmath>


std::vector<int> player;

int resources;

int turn_count;

int event_strength()
{
	return turn_count;
}


void wave(unsigned int strength)
{
	for (auto& it : player)
	{
		it -= strength;
	}
}

void thunder(unsigned int strength)
{
	if (player.size() <= strength)
	{
		player.clear();
	}
	else
	{
		for (int i = 0; i != strength; ++i)
		{
			player.pop_back();
		}
	}
}

void wind(unsigned int strength)
{
	for (int i = 0; i != strength; ++i)
	{
		if (player[i] == 1)
		{
			player[i] = 0;
			player.resize(i);
			return;
		}
		else
		{
			player.push_back(player[i] - 1);
			player[i] = 1;
		}
	}
}



void grow(unsigned int strength)
{
	if (strength > resources)
	{
		strength = resources;
	}
	resources -= strength;

	for (int i = 0; i < player.size() && i < strength; ++i)
	{
		player[i] += strength - i;
	}
}

void stretch(unsigned int strength)
{
	if (strength > resources)
	{
		strength = resources;
	}
	resources -= strength;

	for (int i = 0; i != strength; ++i)
	{
		player.push_back(1);
	}
}

void shrink(unsigned int strength)
{
	if (strength > resources)
	{
		strength = resources;
	}
	if (strength > player.size())
	{
		strength = player.size();
	}
	strength = 2 * (strength / 2); //must be even
	resources -= strength;

	for (int i = 0; i != strength; ++++i)
	{
		player[i / 2] = player[i] + 2 * player[i + 1];
	}
	for (int i = 0; i != strength / 2; ++i)
	{
		player.pop_back();
	}
}

//runs after every event or play, returns true when the player loses
bool clean()
{
	for (int i = 0; i != player.size(); ++i)
	{
		if (player[i] <= 0)
		{
			player.resize(i);
			break;
		}
	}
	if (player.size() == 0)
	{
		return true;
	}

	++turn_count;

	int newresources = 0;

	for (auto it : player)
	{
		newresources += it;
	}
	resources += newresources / player.size();

	return false;
}

void set_up(std::vector<int>&& a, int b)
{
	player = std::move(a);
	resources = b;
	turn_count = 1;
}

std::string random_event()
{
	static std::uniform_int_distribution<> d(1, 3);
	static std::random_device r;
	static std::mt19937 g(r());
	switch (d(g))
	{
	case(1):
		return "wave";
	case(2):
		return "thunder";
	case(3):
		return "wind";
	}

	return "";//this should never happen
}

#include<iostream>
#include<string>
#include<functional>


#include"../evaluator.h"
using namespace expr;


int main()
{
	//this environment is for doing the various operations on the player's state
	environment options;
	options.functions
		<< "event.wave" << sfn(wave) << "event.thunder" << sfn(thunder) << "event.wind" << sfn(wind)
		<< "player.grow" << sfn(grow) << "player.stretch" << sfn(stretch) << "player.shrink" << sfn(shrink);

	//and this environment is for letting the user view/interact with the rest of the program and the first environment
	interpreter evaluator;

	//pass the final result of all expressions to _view
	evaluator.settings.auto_call("_view");

	//print errors
	evaluator.settings.error_print(true);

	//reprint the input as parsed
	evaluator.settings.input_reprint(false);

	//binding various aspects of the program
	evaluator.functions
		<< "res" << refto(resources)
		<< "state" << refto(player)
		<< "turn" << refto(turn_count)
		<< "set_up" << sfn(set_up)

		//making a function for interacting with the first environment and letting a turn go by
		<< "play" << sfn(std::function<void()>([options = &options]
		() -> void
	{
		if (player.size() == 0)
		{
			std::cout << "cannot play with empty board\n" << std::flush;
			return;
		}
		std::string todo;
		std::getline(std::cin, todo);
		//evaluate the concatenation of "player." and the new input.
		if (options->evaluate("player." + todo, std::cout).is_nullval())
		{
			//if the evaluation returned empty
			std::cout << "bad input\n" << std::flush;
			return;
		}
		std::string event = random_event();
		std::cout << event << ' ' << std::to_string(event_strength()) << std::endl;

		options->evaluate("event." + event + "(" + std::to_string(event_strength()) + ")", std::cout);
		if (clean())
		{
			std::cout << "game over\n" << std::flush;
		}
	}
	));

	evaluator.go();
}
/*
a sample game looks like this:
/|\
\\\
set_up([3,3,3],10)
///
|||
\\\
state
///
[3,3,3]
|||
\\\
res
///
10
|||
\\\
play
///
grow(10)
wave 1
|||
\\\
state
///
[12,11,10]
|||
\\\
res
///
11
|||
\\\
play
///
stretch(4)
thunder 2
|||
\\\
state
///
[12,11,10,1,1]
|||
\\\
res
///
14
|||
\\\
play
///
grow(14)
wave 3
|||
\\\
state
///
[23,21,19,9,8]
|||
\\\
res
///
16
|||
\\\
play
///
stretch(7)
thunder 4
|||
\\\
state
///
[23,21,19,9,8,1,1,1]
|||
\\\
res
///
19
|||
\\\
turn
///
5
|||
\\\
grow(19)
///
|||
function name "grow" not found
call to function "view" returned null
\\\
play
///
grow(19)
wave 5
|||
\\\
state
///
[37,34,31,20,18,10,9,8]
|||
\\\
res
///
20
|||
\\\
play
///
stretch(10)
wave 6
|||
\\\
state
///
[31,28,25,14,12,4,3,2]
|||
\\\
res
///
24
|||
\\\
play
///
stretch(12)
wind 7
|||
\\\
state
///
[1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,30,27,24,13,11,3,2]
|||
\\\
res
///
16
|||
\\\
play
///
grow(16)
thunder 8
|||
\\\
state
///
[17,16,15,14,13,12,11,11,9,8,7,6,5,4,3,2,1,1,1]
|||
\\\
res
///
8
|||
\\\
play
///
grow(8)
wave 9
|||
\\\
state
///
[16,14,12,10,8,6,4,3]
|||
\\\
res
///
9
|||
\\\
play
///
grow(9)
thunder 10
game over
|||
\\\
turn
///
10
|||
\\\
*/
