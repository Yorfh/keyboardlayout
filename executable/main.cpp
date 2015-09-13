#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <regex>
#include "optionparser.h"
#include "Optimizer.hpp"
#include "BMAOptimizer.hpp"
#include "TravelingSalesman.hpp"
#include "mQAP.hpp"
#include "QAP.hpp"


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

enum  optionIndex {
	UNKNOWN, HELP, MAXT, MINT, NUMSTEPS, FAST_MAXT, FAST_MINT, FAST_NUMSTEPS, PARETO_MAXT,
	PARETO_MINT, PARETO_EQUALMULT, NUMEVALUATIONS, POPULATION, TEST, OUTPUT, SEED,
	SHORT_IMPROVEMENT, LONG_IMPROVEMENT, STAGNATION_ITERATIONS, STAGNATION_MIN, STAGNATION_MAX,
	TENURE_MIN, TENURE_MAX, JUMP_MAGNITUDE, DIRECTED_PERTUBATION, SMAC, INSTANCE_INFO,
	CUTOFF_TIME, CUTOFF_LENGTH, TOUR_POOLSIZE, TOUR_MUT_FREQ, TOUR_MUT_STR, TOUR_MUT_GRO,
	ALGO_TYPE, CROSSOVER_TYPE, PERTURB_TYPE, ANYTIME,
};

const option::Descriptor usage[] =
{
	{ UNKNOWN,		0, "", "",				option::Arg::None,	"USAGE: keyboardlayout [options]\n\nOptions:" },
	{ HELP,			0, "", "help",			option::Arg::None,	"  --help  \tPrint usage and exit." },
	{ MAXT,			0, "", "max_t",			floatingPoint,		"  --max_t  \tThe maximum temperature" },
	{ MINT,			0, "", "min_t",			floatingPoint,		"  --min_t  \tThe minimum temperature" },
	{ NUMSTEPS,		0, "", "steps",			unsignedInteger,	"  --steps  \tThe number of temperature steps" },
	{ FAST_MAXT,    0, "", "fast_max_t",	floatingPoint,		"  --fast_max_t  \tThe fast cooling maximum temperature" },
	{ FAST_MINT,    0, "", "fast_min_t",	floatingPoint,		"  --fast_min_t  \tThe fast cooling minimum temperature" },
	{ FAST_NUMSTEPS,0, "", "fast_steps",	unsignedInteger,	"  --fast_steps  \tThe number of fast cooling temperature steps" },
	{ PARETO_MAXT,  0, "", "pareto_max_t",	floatingPoint,		"  --pareto_max_t  \tThe pareto cooling maximum temperature" },
	{ PARETO_MINT,  0, "", "pareto_min_t",	floatingPoint,		"  --pareto_min_t  \tThe pareto cooling minimum temperature" },
	{ PARETO_EQUALMULT,0, "", "pareto_equal_multiplier", floatingPoint, "  --pareto_equal_multiplier  \tThe pareto cooling energy multiplier for two equally good solutions" },
	{ NUMEVALUATIONS,0, "", "evaluations",	unsignedInteger,	"  --evaluations  \tThe maximum number of evaluations" },
	{ POPULATION,	0, "", "population",	unsignedInteger,	"  --population  \tThe population size"},
	{ TEST,			0, "", "test",			required,			"  --test  \tThe name of the test to execute" },
	{ OUTPUT,		0, "", "output",		required,			"  --output  \tThe name of the output file" },
	{ SEED,			0, "", "seed",			unsignedInteger,	"  --seed  \tThe random seed" },
	{ SHORT_IMPROVEMENT, 0, "", "short_improvement", unsignedInteger,	"  --short_improvement  \tShort improvement iterations for BMA" },
	{ LONG_IMPROVEMENT,	0, "", "long_improvement", unsignedInteger,	"  --long_improvement  \tLong improvement iterations for BMA" },
	{ STAGNATION_ITERATIONS,	0, "", "stagnation_iterations", unsignedInteger,	"  --stagnation_iterations  \tThe number of iterations before stagnation for for BMA" },
	{ STAGNATION_MIN,	0, "", "stagnation_min", floatingPoint,	"  --stagnation_min  \tThe minimum stagnation magnitude for BMA" },
	{ STAGNATION_MAX,	0, "", "stagnation_max", floatingPoint,	"  --stagnation_max  \tThe maximum stagnation magnitude for BMA" },
	{ TENURE_MIN,	0, "", "tenure_min", floatingPoint,	"  --tenure_min  \tThe minimum tenure for BMA" },
	{ TENURE_MAX,	0, "", "tenure_max", floatingPoint,	"  --tenure_max  \tThe maximum tenure for BMA" },
	{ JUMP_MAGNITUDE,	0, "", "jump_magnitude", floatingPoint,	"  --jump_magnitude  \tThe jump magnitude for BMA" },
	{ DIRECTED_PERTUBATION,	0, "", "min_directed_pertubation", floatingPoint,	"  --min_directed_pertubation  \tThe minimum percentage of directed pertubation for BMA" },
	{ TOUR_POOLSIZE,	0, "", "tournament_pool_size", unsignedInteger,	"  --tournament_pool_size  \tThe tournament pool size for BMA" },
	{ TOUR_MUT_FREQ,	0, "", "tournament_mutation_frequency", unsignedInteger,	"  --tournament_mutation_frequency  \tThe tournament mutation frequency for BMA" },
	{ TOUR_MUT_STR,	0, "", "tournament_min_mutation_strength", floatingPoint,	"  --tournament_min_mutation_strength  \tThe tournament mutation strength for BMA" },
	{ TOUR_MUT_GRO,	0, "", "tournament_mutation_growth", unsignedInteger,	"  --tournament_mutation_growth  \tThe tournament mutation growth for BMA" },
	{ CROSSOVER_TYPE,	0, "", "crossover_type", required,	"  --crossover_type uniform|partially_matched \tThe crossover type for BMA" },
	{ PERTURB_TYPE,	0, "", "perturb_type", required,	"  --perturb_type normal|annealed|disabled \tThe perturb type for BMA" },
	{ SMAC,	0, "", "smac", option::Arg::None,	"  --smac  \tThe output should be in SMAC format" },
	{ INSTANCE_INFO,	0, "", "instance_info", required,	"  --instance_info  \tThe smac instance information" },
	{ CUTOFF_TIME,	0, "", "cutoff_time", unsignedInteger,	"  --cutoff_time  \tThe smac instance cutoff time" },
	{ CUTOFF_LENGTH,	0, "", "cutoff_length", unsignedInteger,	"  --cutoff_length  \tThe smac instance cutoff length" },
	{ ALGO_TYPE,	0, "", "algo_type", required,	"  --algotype annealing|bma \tThe algorithm type" },
	{ ANYTIME,	0, "", "anytime", unsignedInteger,	"  --anytime snapshot_delay \tTake snapshots regularly to optimize for any time" },
	{ 0,0,0,0,0,0 }
};

template<size_t KeyboardSize, typename Objective>
int oneDimensionalAnnealing(Objective& objective, float minT, float maxT, int numSteps, float fast_minT, float fast_maxT, int fast_numSteps, int numEvaluations, unsigned int seed)
{
	Optimizer<KeyboardSize, 1> o(seed);
	o.populationSize(1);
	o.initialTemperature(maxT, minT, numSteps);
	o.fastCoolingTemperature(fast_maxT, fast_minT, fast_numSteps);
	auto objectives = { objective };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), numEvaluations);
	auto result = solutions.getResult()[0].m_keyboard;
	int resultValue = static_cast<int>(-std::round(objective.evaluate(result)));
	return resultValue;

}

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
	TravelingSalesman<14> salesman(latitudes, longitudes);
	return oneDimensionalAnnealing<13>(salesman, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, numEvaluations, seed);
}

template<size_t NumLocations, size_t NumObjectives>
int mqap_helper(const std::string filename, float minT, float maxT, int numSteps, float fast_minT, float fast_maxT, int fast_numSteps, float pareto_minT, float pareto_maxT, float pareto_equalMultiplier,
	unsigned int numEvaluations, unsigned int population, unsigned int seed, const std::string outputFile)
{
	std::vector<mQAP<NumLocations>> objectives;
	for (size_t i = 0; i < NumObjectives; i++)
	{
		mQAP<NumLocations> objective(filename, i);
		objectives.push_back(objective);
	}
	Optimizer<NumLocations, NumObjectives, 32> o(seed);
	o.populationSize(population);
	o.initialTemperature(maxT, minT, numSteps);
	o.fastCoolingTemperature(fast_maxT, fast_minT, fast_numSteps);
	o.paretoTemperature(pareto_maxT, pareto_minT, pareto_equalMultiplier);
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

int mqap(const std::string filename, float minT, float maxT, int numSteps, float fast_minT, float fast_maxT, int fast_numSteps, float pareto_minT, float pareto_maxT, float pareto_equalMultiplier,
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
			return mqap_helper<10, 2>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, pareto_equalMultiplier, numEvaluations, population, seed, outputFile);
		}
		else if (numObjectives == 3)
		{
			return mqap_helper<10, 3>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, pareto_equalMultiplier, numEvaluations, population, seed, outputFile);
		}
	}
	else if (numLocations == 20)
	{
		if (numObjectives == 2)
		{
			return mqap_helper<20, 2>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, pareto_equalMultiplier, numEvaluations, population, seed, outputFile);
		}
		else if (numObjectives == 3)
		{
			return mqap_helper<20, 3>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, pareto_equalMultiplier, numEvaluations, population, seed, outputFile);
		}
	}
	else if (numLocations == 30)
	{
		if (numObjectives == 2)
		{
			return mqap_helper<30, 2>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, pareto_equalMultiplier, numEvaluations, population, seed, outputFile);
		}
		else if (numObjectives == 3)
		{
			return mqap_helper<30, 3>(filename, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, pareto_minT, pareto_maxT, pareto_equalMultiplier, numEvaluations, population, seed, outputFile);
		}
	}
	return 0;
}

int qap_bma(const std::string filename, size_t population, size_t shortDepth, size_t longDepth, size_t stagnationIters,
	float stagnationMinMag, float stagnationMaxMag, float jumpMagnitude, float minDirectedPertubation, 
	size_t tournamentPoolSize, size_t mutationFreuency, float minMutationStrength, size_t mutationStrengthGrowth, 
	CrossoverType crossoverType, PerturbType perturbType, size_t evaluations, size_t anytime, unsigned int seed)
{
	QAP<12> objective(filename);
	Keyboard<12> keyboard;
	BMAOptimizer<12, 1> o(seed);
	o.populationSize(population);
	o.improvementDepth(shortDepth, longDepth);
	o.stagnation(stagnationIters, stagnationMinMag, stagnationMaxMag);
	o.jumpMagnitude(jumpMagnitude);
	o.minDirectedPertubation(minDirectedPertubation);
	o.tournamentPool(tournamentPoolSize);
	o.mutation(mutationFreuency, minMutationStrength, mutationStrengthGrowth);
	o.crossover(crossoverType);
	o.perturbType(perturbType);
	if (anytime != 0)
	{
		o.snapshots(anytime);
	}
	auto objectives = { objective };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), evaluations);
	int resultValue = 0;
	if (anytime == 0)
	{
		auto result = solutions.getResult()[0].m_keyboard;
		resultValue = static_cast<int>(-std::round(objective.evaluate(result)));
	}
	else
	{
		auto& snapshots = o.getSnapshots();
		float result = solutions.getResult()[0].m_solution[0];
		float multiplierSum = 1.0f;
		for (auto&& s: snapshots)
		{ 
			float multiplier = static_cast<float>(s.second) / static_cast<float>(evaluations);
			result += multiplier * s.first[0].m_solution[0];
			multiplierSum += multiplier;
		}
		resultValue = static_cast<int>(-std::round(result / multiplierSum));

	}
	return resultValue;
}

int qap_annealing(const std::string& filename, float minT, float maxT, int numSteps, float fast_minT, float fast_maxT, 
	int fast_numSteps, int numEvaluations, unsigned int seed)
{
	QAP<12> objective(filename);
	return oneDimensionalAnnealing<12>(objective, minT, maxT, numSteps, fast_minT, fast_maxT, fast_numSteps, numEvaluations, seed);
}

template<typename T, bool IsSigned = std::is_signed<T>::value, size_t NumBytes = sizeof(T)>
struct GetArgumentHelper
{
};

template<typename T>
struct GetArgumentHelper<T, true, 4>
{
	T operator()(const char* arg) const
	{
		return std::stol(arg);
	}
};

template<typename T>
struct GetArgumentHelper<T, false, 4>
{
	T operator()(const char* arg) const
	{
		return std::stoul(arg);
	}
};

template<typename T>
struct GetArgumentHelper<T, false, 8>
{
	T operator()(const char* arg) const
	{
		return std::stoull(arg);
	}
};

template<typename T>
struct GetArgumentHelper<T, true, 8>
{
	T operator()(const char* arg) const
	{
		return std::stoll(arg);
	}
};

template<>
struct GetArgumentHelper<float, true, 4>
{
	float operator()(const char* arg) const
	{
		return std::stof(arg);
	}
};

template<>
struct GetArgumentHelper<double, true, 8>
{
	double operator()(const char* arg) const
	{
		return std::stod(arg);
	}
};

template<typename T>
T getArgument(const std::vector<option::Option>& options, size_t index)
{
	if (!options[index])
	{
		throw std::invalid_argument(usage[index].longopt);
	}
	return GetArgumentHelper<T>()(options[index].arg);
}

template<>
std::string getArgument<std::string>(const std::vector<option::Option>& options, size_t index)
{
	if (!options[index])
	{
		throw std::invalid_argument(usage[index].longopt);
	}
	return options[index].arg;
}

template<typename T>
void outputResult(T result, size_t seed, bool smac, bool minimize)
{
	if (smac)
	{
		std::cout << "Result of this algorithm run: ";
		std::cout << "SAT, ";
		size_t runTime = 1;
		size_t runLenght = 1;
		std::cout << runTime << ", " << runLenght << ", ";
		std::cout << (minimize ? result : -result) << ", ";
		std::cout << seed << ", 0" << std::endl;
	}
	else
	{
		std::cout << (minimize ? -result : result) << std::endl;
	}
}

int main(int argc, char* argv[])
{
	argc -= (argc > 0); argv += (argc > 0); // skip program name argv[0] if present
	option::Stats stats(usage, argc, argv, 0, true);

	std::vector<option::Option> options(stats.options_max);
	std::vector<option::Option> buffer(stats.buffer_max);
	option::Parser parse(usage, argc, argv, options.data(), buffer.data(), 0, true);

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
	std::cout << "Raw options" << std::endl;
	for (size_t i = 0;i < argc;i++)
	{
		std::cout << argv[i] << " ";
	}
	std::cout << std::endl;

	std::cout << "Options:" << std::endl;
	for (size_t i = 0;i < stats.options_max;i++)
	{
		if (options[i] && options[i].arg)
		{
			std::cout << options[i].name << " " << options[i].arg << std::endl;
		}
		else if (options[i])
		{
			std::cout << options[i].name << std::endl;
		}
	}
	std::cout << "end of options" << std::endl;

	try
	{
		unsigned int evaluations = getArgument<int>(options, NUMEVALUATIONS);
		unsigned int seed = getArgument<unsigned int>(options, SEED);
		if (options[TEST])
		{
			std::string test = options[TEST].arg;
			std::transform(test.begin(), test.end(), test.begin(), tolower);
			bool isBurma = test == "burma14";
			bool ismqap = test.find("mqap") != -1;
			bool isqap = !ismqap && test.find("qap") != -1;
			if (isBurma || ismqap)
			{
				float minT = getArgument<float>(options, MINT);
				float maxT = getArgument<float>(options, MAXT);
				int steps = getArgument<int>(options, NUMSTEPS);
				float fast_minT = getArgument<float>(options, FAST_MINT);
				float fast_maxT = getArgument<float>(options, FAST_MAXT);
				int fast_steps = getArgument<int>(options, FAST_NUMSTEPS);
				if (isBurma)
				{
					auto res = burma14(minT, maxT, steps, fast_minT, fast_maxT, fast_steps, evaluations, seed);
					outputResult(res, seed, options[SMAC] != nullptr, true);
				}
				else
				{
					float pareto_minT = getArgument<float>(options, PARETO_MINT);
					float pareto_maxT = getArgument<float>(options, PARETO_MAXT);
					float pareto_equalMultiplier = getArgument<float>(options, PARETO_EQUALMULT);
					if (!options[OUTPUT])
					{
						std::cout << "An output file is required" << std::endl;
						return 1;
					}
					unsigned int population = getArgument<unsigned int>(options, POPULATION);
					auto res = mqap(test, minT, maxT, steps, fast_minT, fast_maxT, fast_steps, pareto_minT, pareto_maxT, pareto_equalMultiplier, evaluations, population, seed, options[OUTPUT].arg);
					outputResult(res, seed, options[SMAC] != nullptr, true);
				}
			}
			else if (test.find("qap") != -1)
			{
				std::string algoType = getArgument<std::string>(options, ALGO_TYPE);
				bool useAnnealing = algoType == "annealing";
				if (useAnnealing)
				{
					float minT = getArgument<float>(options, MINT);
					float maxT = getArgument<float>(options, MAXT);
					int steps = getArgument<int>(options, NUMSTEPS);
					float fast_minT = getArgument<float>(options, FAST_MINT);
					float fast_maxT = getArgument<float>(options, FAST_MAXT);
					int fast_steps = getArgument<int>(options, FAST_NUMSTEPS);
					auto res = qap_annealing(test, minT, maxT, steps, fast_minT, fast_maxT, fast_steps, evaluations, seed);
					outputResult(res, seed, options[SMAC] != nullptr, true);

				}
				else
				{
					size_t population = getArgument<size_t>(options, POPULATION);
					size_t shortDepth = getArgument<size_t>(options, SHORT_IMPROVEMENT);
					size_t longDepth = getArgument<size_t>(options, LONG_IMPROVEMENT);
					size_t stagnationIters = getArgument<size_t>(options, STAGNATION_ITERATIONS);
					float stagnationMin = getArgument<float>(options, STAGNATION_MIN);
					float stagnationMax = getArgument<float>(options, STAGNATION_MAX);
					float jumpMagnitude = getArgument<float>(options, JUMP_MAGNITUDE);
					float directedPertubation = getArgument<float>(options, DIRECTED_PERTUBATION);
					size_t tournamentPoolSize = getArgument<size_t>(options, TOUR_POOLSIZE);
					size_t tournamentMutationFrequency = getArgument<size_t>(options, TOUR_MUT_FREQ);
					float tournamentMutationStrength = getArgument<float>(options, TOUR_MUT_STR);
					size_t tournamentMutGrowth = getArgument<size_t>(options, TOUR_MUT_GRO);
					std::string crossoverType = getArgument<std::string>(options, CROSSOVER_TYPE);
					size_t anytime = 0;
					if (options[ANYTIME])
					{
						anytime = getArgument<size_t>(options, ANYTIME);
					}
					CrossoverType ct;
					if (crossoverType == "uniform")
					{
						ct = CrossoverType::Uniform;
					}
					else if (crossoverType == "partially_matched")
					{
						ct = CrossoverType::PartiallyMatched;
					}
					else
					{
						std::cout << "Invalid crossover type " << crossoverType;
						return 1;
					}
					std::string perturbTypeStr = getArgument<std::string>(options, PERTURB_TYPE);
					PerturbType perturbType = PerturbType::Normal;
					if (perturbTypeStr == "annealed")
					{
						perturbType = PerturbType::Annealed;

					}
					else if (perturbTypeStr == "normal")
					{
						perturbType = PerturbType::Normal;
					}
					else if (perturbTypeStr == "disabled")
					{
						perturbType = PerturbType::Disabled;
					}
					else
					{
						std::cout << "Invalid perturb type " << perturbTypeStr;
						return 1;
					}

					auto res = qap_bma(test, population, shortDepth, longDepth, stagnationIters, stagnationMin, stagnationMax, jumpMagnitude, 
						directedPertubation, tournamentPoolSize, tournamentMutationFrequency, tournamentMutationStrength, tournamentMutGrowth, 
						ct, perturbType, evaluations, anytime, seed);
					outputResult(res, seed, options[SMAC] != nullptr, true);
				}
			}
		}
	}
	catch (std::invalid_argument e)
	{
			std::cout << "Option '" << e.what() << "' is required" << std::endl;
			option::printUsage(std::cout, usage);
			return 1;
	}
}