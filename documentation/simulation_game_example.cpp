#include<vector>
#include<random>


std::vector<int> player;

int resources;

int turn_count;

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
	resources -= strength;

	for (int i = 0; i != 2*strength; ++++i)
	{
		if (i + 1 >= player.size())
		{
			shrink((strength - i) / 2);
			return;
		}
		player[i/2] = player[i] + player[i + 1];
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

	for (auto it : player)
	{
		resources += it;
	}
	resources /= player.size();

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


#include"../details.h"
using namespace expr;


int main()
{
	environment options;
	options.functions
		<< "event.wave" << sfn(wave) << "event.thunder" << sfn(thunder) << "event.wind" << sfn(wind)
		<< "player.grow" << sfn(grow) << "player.stretch" << sfn(stretch) << "player.shrink" << sfn(shrink);

	interpreter evaluator;
	evaluator.settings.auto_call("_view");
	evaluator.settings.error_print(true);
	evaluator.settings.input_reprint(false);
	evaluator.functions
		<< "res" << val(&resources)
		<< "state" << val(&player)
		<< "turn" << val(&turn_count)
		<< "set_up" << sfn(set_up)
		<< "play" << sfn(std::function<void()>([options = &options]
		() -> void
		{
			if (player.size() == 0)
			{
				std::cout << "cannot play with empty board\n" << std::flush;
			}
			std::string todo;
			std::getline(std::cin,todo);
			if (options->evaluate(elem::make("player." + todo), std::cout).is_nullval())
			{
				std::cout << "bad input\n" << std::flush;
				return;
			}
			std::string event = random_event();
			std::cout << event << std::endl;
			
			options->evaluate(elem::make("event." + event + "(" + std::to_string(turn_count) + ")"), std::cout);
			if (clean())
			{
				std::cout << "game over\n" << std::flush;
			}
		}
	));

	evaluator.go();
}