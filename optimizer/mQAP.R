# ========================================================================= #
# Optimizes the parameters for the multiobjective quadratic assignment      # 
# problem                                                                   #
# ------------------------------------------------------------------------- #
# Copyright (C) 2015 Fred Sundvik                                           #
# ========================================================================= #
# This program is free software; you can redistribute it and/or modify it   #
# under the terms of the GNU General Public License as published by the     #
# Free Software Foundation; either version 2 of the License, or (at your    #
# option) any later version.                                                #
#                                                                           #
# This program is distributed in the hope that it will be useful, but       #
# WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         #
# General Public License for more details.                                  #
#                                                                           #
# You should have received a copy of the GNU General Public License along   #
# with this program; if not, write to the Free Software Foundation, Inc.,   #
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.                  #
# ========================================================================= #

library("irace")

EXE <- "../x64/Release/keyboardlayout.exe"
HYPERVOLUME <- "../x64/Release/wfg.exe"
test_file <- paste("../tests/mQAPData/", commandArgs(trailingOnly = TRUE)[1], sep="")
num_evaluations <- 300000

dir.create("output", showWarnings = FALSE)

instances <- floor(runif(200, min=0, max=4294967295 + 1))

type <- "initial"

max_t <- 0.0
min_t <- 0.0
steps <- 0
fast_max_t <- 0
fast_min_t <- 0
fast_steps <- 0
pareto_min_t <- 0
pareto_max_t <- 0
pareto_equal_multiplier <- 0.0
population <- 0

hook.run <- function(instance, candidate, extra.params = NULL, config = list())
{
  if (type == "initial")
  {
    max_t <- candidate$values[["max_t"]]
    min_t <- candidate$values[["min_t"]]
    steps <- as.integer(num_evaluations * 0.1 / candidate$values[["population"]])
    fast_max_t <- 0.01
    fast_min_t <- 0.01
    fast_steps <- 1
    pareto_min_t <- 0.01
    pareto_max_t <- 0.0
    population <- candidate$values[["population"]]
    evaluations <- population * steps
  }
  else
  {
    fast_max_t <- candidate$values[["fast_max_t"]]
    fast_min_t <- candidate$values[["fast_min_t"]]
    fast_steps <- candidate$values[["fast_steps"]]
    pareto_max_t <- candidate$values[["pareto_max_t"]]
    pareto_min_t <- candidate$values[["pareto_min_t"]]
    pareto_equal_multiplier <- candidate$values[["pareto_equal_multiplier"]]
    evaluations = num_evaluations
  }
  args <- sprintf("--max_t %f --min_t %f --steps %i --fast_max_t %f --fast_min_t %f --fast_steps %i --pareto_max_t %f --pareto_min_t %f --pareto_equal_multiplier %f --population %i --evaluations %i --seed %s --test %s --output output/%i.txt", max_t, min_t, steps, fast_max_t, fast_min_t, fast_steps, pareto_max_t, pareto_min_t, pareto_equal_multiplier, population, evaluations, as.character(instance), test_file, candidate$index) 
  output <- system2(EXE, args=args, stdout=TRUE, stderr=TRUE)
  return(as.numeric(output[1]))
}

hook.evaluate <- function(instance, candidate, num.candidates, extra.params, config, hook.run.call)
{
  table <- NULL
  for (i in 1:num.candidates)
  {
    outputFile <- sprintf("output/%i.txt", i)
    table <- rbind(table, read.table(outputFile))
  }
  nadir <- apply(table, 2, max)
  nadir <- nadir + 1
  names(nadir) <- NULL
  args <- sprintf("output/%i.txt %s", candidate$index, paste(nadir, collapse=" "))
  output <- system2(HYPERVOLUME, args=args, stdout=TRUE, stderr=TRUE)
  hv <- strsplit(output[1], "=")[[1]]
  return(-as.numeric(hv[2]))
}

parameters.table <- '
max_t "" r (0, 1000)
min_t "" r (0, 1000)
population "" i (1, 1000)
'


parameters <- readParameters(text = parameters.table)

experiments <- 1000

if (0)
{
result <- irace(tunerConfig = list(
                hookRun = hook.run,
		hookEvaluate = hook.evaluate,
		forbiddenFile = "parameters_initial.forbidden",
                instances = instances[1:100],
                maxExperiments = experiments,
                logFile = ""),
                parameters = parameters)

candidates.print(result)
initial_result <- result

max_t <- as.numeric(result[1, c("max_t")])
min_t <- as.numeric(result[1, c("min_t")])
population <- as.integer(result[1, c("population")])
}

max_t <- 524.3690 
min_t <- 216.8189
population <- 151

print(max_t)
print(min_t)
print(population)

type = "cooling"

parameters.table <- '
fast_max_t "" r (0, 1000)
fast_min_t "" r (0, 1000)
fast_steps "" i (1, 1000)
pareto_max_t "" r (0.0, 1.0)
pareto_min_t "" r (0.0, 0.1)
pareto_equal_multiplier "" r (0.0, 1.0)
'

parameters <- readParameters(text = parameters.table)
result <- irace(tunerConfig = list(
                hookRun = hook.run,
		hookEvaluate = hook.evaluate,
		forbiddenFile = "parameters_cooling.forbidden",
                instances = instances[1:100],
                maxExperiments = experiments,
                logFile = ""),
                parameters = parameters)


candidates.print(initial_result)
candidates.print(result)
