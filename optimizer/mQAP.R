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
num_evaluations <- 150000

dir.create("output", showWarnings = FALSE)

instances <- floor(runif(200, min=0, max=4294967295 + 1))

hook.run <- function(instance, candidate, extra.params = NULL, config = list())
{
  args <- sprintf("--max_t %f --min_t %f --steps %i --fast_max_t %f --fast_min_t %f --fast_steps %i --pareto_max_t %f --pareto_min_t %f --population %i --evaluations %i --seed %s --test %s --output output/%i.txt", candidate$values[["max_t"]], candidate$values[["min_t"]], candidate$values[["steps"]], candidate$values[["fast_max_t"]], candidate$values[["fast_min_t"]], candidate$values[["fast_steps"]], candidate$values[["pareto_max_t"]], candidate$values[["pareto_min_t"]], candidate$values[["population"]], num_evaluations, as.character(instance), test_file, candidate$index) 
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
steps "" i (1, 1000)
fast_max_t "" r (0, 1000)
fast_min_t "" r (0, 1000)
fast_steps "" i (1, 1000)
pareto_max_t "" r (0, 1000)
pareto_min_t "" r (0, 1000)
population "" i (1, 1000)
'


parameters <- readParameters(text = parameters.table)

result <- irace(tunerConfig = list(
                  hookRun = hook.run,
		          hookEvaluate = hook.evaluate,
				  forbiddenFile = "parameters.forbidden",
                  instances = instances[1:100],
                  maxExperiments = 1000,
				  nbIterations = 8,
                  logFile = ""),
                parameters = parameters)

candidates.print(result)
