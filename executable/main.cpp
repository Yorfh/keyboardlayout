#include <iostream>
#include <vector>
#include <string>
#include <array>
#include "optionparser.h"
#include "Optimizer.hpp"
#include "TravelingSalesman.hpp"

enum  optionIndex { UNKNOWN, HELP, MAXT, MINT, NUMSTEPS };

void printError(const char* msg1, const option::Option& opt, const char* msg2)
{
	fprintf(stderr, "ERROR: %s", msg1);
	fwrite(opt.name, opt.namelen, 1, stderr);
	fprintf(stderr, "%s", msg2);
}

option::ArgStatus floatingPoint(const option::Option& option, bool msg)
{
	bool invalid = true;
	try
	{
		if (option.arg)
		{
			std::stof(option.arg);
		}

		invalid = false;
	}
	catch (std::invalid_argument e)
	{
		
	}
	if (!invalid)
	{
		return option::ARG_OK;
	}

	if (msg)
	{
		printError("Option '", option, "' requires a numeric argument\n");
	}
	return option::ARG_ILLEGAL;
}

const char* optionNames[] =
{
	"",
	"help",
	"max_t",
	"min_t",
	"steps"
};

const option::Descriptor usage[] =
{
	{ UNKNOWN, 0, "" , optionNames[UNKNOWN],  option::Arg::None, "USAGE: keyboardlayout [options]\n\nOptions:" },
	{ HELP,    0, "" , optionNames[HELP],     option::Arg::None, "  --help  \tPrint usage and exit." },
	{ MAXT,    0, "" , optionNames[MAXT],     floatingPoint,           "  --max_t  \tThe maximum temperature" },
	{ MINT,    0, "" , optionNames[MINT],     floatingPoint, "  --min_t  \tThe minimum temperature" },
	{ NUMSTEPS,0, "" , optionNames[NUMSTEPS], floatingPoint, "  --steps  \tThe number of steps" },
	{ 0,0,0,0,0,0 }
};

int burma14(float minT, float maxT, int numSteps)
{
	std::array<double, 14> latitudes = {
		16.47,
		16.47,
		20.09,
		22.39,
		25.23,
		22.00,
		20.47,
		17.20,
		16.30,
		14.05,
		16.53,
		21.52,
		19.41,
		20.09
	};
	std::array<double, 14> longitudes = {
		96.10,
		94.44,
		92.54,
		93.37,
		97.24,
		96.05,
		97.02,
		96.29,
		97.38,
		98.12,
		97.38,
		95.59,
		97.13,
		94.55
	};
	Optimizer<13> o;
	o.populationSize(1);
	o.localSearchDept(1);
	o.numIterations(20);
	o.temperature(50.0, 5.0, 10000);
	TravelingSalesman<14> salesman(latitudes, longitudes);
	auto objectives = { salesman };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives));
	auto result = solutions.getResult()[0].first;
	int resultValue = static_cast<int>(-std::round(salesman.evaluate(result)));
	return resultValue;
}

int main(int argc, char* argv[])
{
	argc -= (argc > 0); argv += (argc > 0); // skip program name argv[0] if present
	option::Stats stats(usage, argc, argv);

	std::vector<option::Option> options(stats.options_max);
	std::vector<option::Option> buffer(stats.buffer_max);
	option::Parser parse(usage, argc, argv, options.data(), buffer.data());

	if (parse.error())
	{
		option::printUsage(std::cout, usage);
		return 0;
	}

	if (options[HELP] || argc == 0)
	{
		option::printUsage(std::cout, usage);
		return 0;
	}

	for (size_t i = MAXT; i <= NUMSTEPS; ++i)
	{
		if (!options[i])
		{
			std::cout << "Option '" << optionNames[i] << "' is required" << std::endl;
			option::printUsage(std::cout, usage);
			return 0;
		}
	}
	float minT = std::stof(options[MINT].arg);
	float maxT = std::stof(options[MAXT].arg);
	int steps = std::stol(options[NUMSTEPS].arg);
	auto res = burma14(minT, maxT, steps);
	std::cout << res << std::endl;
}