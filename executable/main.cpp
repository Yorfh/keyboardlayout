#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <regex>
#include "optionparser.h"
#include "Optimizer.hpp"
#include "TravelingSalesman.hpp"
#include "mQAP.hpp"


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

option::ArgStatus unsignedInteger(const option::Option& option, bool msg)
{
	bool invalid = true;
	try
	{
		if (option.arg)
		{
			std::stoul(option.arg);
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

option::ArgStatus required(const option::Option& option, bool msg)
{
	if (option.arg)
	{
		return option::ARG_OK;
	}
	else if (msg)
	{
		printError("Option '", option, "' requires a string argument\n");
	}
	return option::ARG_ILLEGAL;
}

enum  optionIndex { UNKNOWN, HELP, MAXT, MINT, NUMSTEPS, FAST_MAXT, FAST_MINT, FAST_NUMSTEPS, PARETO_MAXT, PARETO_MINT, NUMEVALUATIONS, POPULATION, TEST, OUTPUT, SEED };
const char* optionNames[] =
{
	"",
	"help",
	"max_t",
	"min_t",
	"steps",
	"fast_max_t",
	"fast_min_t",
	"fast_steps",
	"pareto_max_t",
	"pareto_min_t",
	"evaluations",
	"population",
	"test",
	"output",
	"seed"
};

const option::Descriptor usage[] =
{
	{ UNKNOWN, 0, "" , optionNames[UNKNOWN],  option::Arg::None, "USAGE: keyboardlayout [options]\n\nOptions:" },
	{ HELP,    0, "" , optionNames[HELP],     option::Arg::None, "  --help  \tPrint usage and exit." },
	{ MAXT,    0, "" , optionNames[MAXT],     floatingPoint,     "  --max_t  \tThe maximum temperature" },
	{ MINT,    0, "" , optionNames[MINT],     floatingPoint,     "  --min_t  \tThe minimum temperature" },
	{ NUMSTEPS,0, "" , optionNames[NUMSTEPS], unsignedInteger,     "  --steps  \tThe number of temperature steps" },
	{ FAST_MAXT,    0, "" , optionNames[FAST_MAXT],     floatingPoint,     "  --fast_max_t  \tThe fast cooling maximum temperature" },
	{ FAST_MINT,    0, "" , optionNames[FAST_MINT],     floatingPoint,     "  --fast_min_t  \tThe fast cooling minimum temperature" },
	{ FAST_NUMSTEPS,0, "" , optionNames[FAST_NUMSTEPS], unsignedInteger,     "  --fast_steps  \tThe number of fast cooling temperature steps" },
	{ PARETO_MAXT,    0, "" , optionNames[PARETO_MAXT],     floatingPoint,     "  --pareto_max_t  \tThe pareto cooling maximum temperature" },
	{ PARETO_MINT,    0, "" , optionNames[PARETO_MINT],     floatingPoint,     "  --pareto_min_t  \tThe pareto cooling minimum temperature" },
	{ NUMEVALUATIONS,0, "" , optionNames[NUMEVALUATIONS], unsignedInteger,     "  --evaluations  \tThe maximum number of evaluations" },
	{ POPULATION,0, "" , optionNames[POPULATION], unsignedInteger,     "  --population  \tThe population size"},
	{ TEST,    0, "" , optionNames[TEST],     required,          "  --test  \tThe name of the test to execute" },
	{ OUTPUT,    0, "" , optionNames[OUTPUT], required,			 "  --output  \tThe name of the output file" },
	{ SEED,    0, "" , optionNames[SEED],     unsignedInteger,   "  --seed  \tThe random seed" },
	{ 0,0,0,0,0,0 }
};

int burma14(float minT, float maxT, int numSteps, float fast_minT, float fast_maxT, int fast_numSteps, int numEvaluations, unsigned int seed)
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
	Optimizer<13, 1> o(seed);
	o.populationSize(1);
	o.initialTemperature(maxT, minT, numSteps);
	o.fastCoolingTemperature(fast_maxT, fast_minT, fast_numSteps);
	TravelingSalesman<14> salesman(latitudes, longitudes);
	auto objectives = { salesman };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), numEvaluations);
	auto result = solutions.getResult()[0].m_keyboard;
	int resultValue = static_cast<int>(-std::round(salesman.evaluate(result)));
	return resultValue;
}

template<size_t NumLocations, size_t NumObjectives>
int mqap_helper(const std::string filename, float minT, float maxT, int numSteps, float fast_minT, float fast_maxT, int fast_numSteps, float pareto_minT, float pareto_maxT,
	unsigned int numEvaluations, unsigned int population, unsigned int seed, const std::string outputFile)
{
	std::vector<mQAP<NumLocations>> objectives;
	for (size_t i = 0; i < NumObjectives; i++)
	{
		mQAP<NumLocations> objective(filename, i);
		objectives.push_back(objective);
	}
	Optimizer<NumLocations, NumObjectives, 32> o;
	o.populationSize(population);
	o.initialTemperature(maxT, minT, numSteps);
	o.fastCoolingTemperature(fast_maxT, fast_minT, fast_numSteps);
	o.paretoTemperature(pareto_maxT, pareto_minT);
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), numEvaluations);
	auto result = solutions.getResult();
	std::ofstream f(outputFile, std::ios::out | std::ios::trunc);
	f << "#" << std::endl;
	for (auto&& r : result)
	{
		for (auto&& o : r.m_solution)
		{
			f << std::setprecision(16) << -o << " ";
		}
		f << std::endl;
	}
	f << "#" << std::endl;
	return 0;
}

int mqap(const std::string filename, float minT, float maxT, int numSteps, float fast_minT, float fast_maxT, int fast_numSteps, float pareto_minT, float pareto_maxT,
	unsigned int numEvaluations, unsigned int population, unsigned int seed, const std::string outputFile)
{
	auto regex = std::regex("KC(.*)-(.)fl");
	std::smatch match;
	std::regex_search(filename, match, regex);
	int numLocations = std::stoi(match.str(1));
	int numObjectives = std::stoi(match.str(2));
	if (numLocations == 10)
	{
		if (numObjectives == 2)
		{
			return mqap_helper<10, 2>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, numEvaluations, population, seed, outputFile);
		}
		else if (numObjectives == 3)
		{
			return mqap_helper<10, 3>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, numEvaluations, population, seed, outputFile);
		}
	}
	else if (numLocations == 20)
	{
		if (numObjectives == 2)
		{
			return mqap_helper<20, 2>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, numEvaluations, population, seed, outputFile);
		}
		else if (numObjectives == 3)
		{
			return mqap_helper<20, 3>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, numEvaluations, population, seed, outputFile);
		}
	}
	else if (numLocations == 30)
	{
		if (numObjectives == 2)
		{
			return mqap_helper<30, 2>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, numEvaluations, population, seed, outputFile);
		}
		else if (numObjectives == 3)
		{
			return mqap_helper<30, 3>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, numEvaluations, population, seed, outputFile);
		}
	}
	return 0;
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
		return 1;
	}

	if (options[HELP] || argc == 0)
	{
		option::printUsage(std::cout, usage);
		return 1;
	}

	for (size_t i = MAXT; i <= NUMSTEPS; ++i)
	{
		if (!options[i])
		{
			std::cout << "Option '" << optionNames[i] << "' is required" << std::endl;
			option::printUsage(std::cout, usage);
			return 1;
		}
	}
	float minT = std::stof(options[MINT].arg);
	float maxT = std::stof(options[MAXT].arg);
	int steps = std::stol(options[NUMSTEPS].arg);
	float fast_minT = std::stof(options[FAST_MINT].arg);
	float fast_maxT = std::stof(options[FAST_MAXT].arg);
	int fast_steps = std::stol(options[FAST_NUMSTEPS].arg);
	float pareto_minT = std::stof(options[PARETO_MINT].arg);
	float pareto_maxT = std::stof(options[PARETO_MAXT].arg);
	unsigned int evaluations = std::stol(options[NUMEVALUATIONS].arg);
	unsigned int seed = std::stoul(options[SEED].arg);
	if (options[TEST])
	{
		if (strcmp(options[TEST].arg, "burma14") == 0)
		{
			auto res = burma14(minT, maxT, steps, fast_minT, fast_maxT, fast_steps, evaluations, seed);
			std::cout << res << std::endl;
		}
		else
		{
			if (!options[OUTPUT])
			{
				std::cout << "An output file is required" << std::endl;
				return 1;
			}
			unsigned int population = std::stol(options[POPULATION].arg);
			auto res = mqap(options[TEST].arg, minT, maxT, steps, fast_minT, fast_maxT, fast_steps, pareto_minT, pareto_maxT, evaluations, population, seed, options[OUTPUT].arg);
			std::cout << res << std::endl;
		}
	}
}