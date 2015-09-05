# ========================================================================= #
# Optimizes the parameters for the quadratic assignment                     # 
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
test_file <- paste("../tests/QAPData/", commandArgs(trailingOnly = TRUE)[1], sep="")

num_evaluations <- 70000
experiments <- 1000
# This can be set to true to test only the fast cooling phase
# In that case you should probably change the default parameters as well
fast_cooling_only <- FALSE

dir.create("output", showWarnings = FALSE)

instances <- floor(runif(200, min=0, max=4294967295 + 1))

hook.run <- function(instance, candidate, extra.params = NULL, config = list())
{
  evaluations = num_evaluations
  cmdLine <- buildCommandLine(candidate$values, candidate$labels)
  args <- sprintf("%s --evaluations %i --seed %s --test %s", cmdLine, num_evaluations, as.character(instance), test_file)
  output <- system2(EXE, args=args, stdout=TRUE, stderr=TRUE)
  if ("status" %in% attributes(output))
  {
    print(output)
    return(0)
  }
  return(as.numeric(output[1]))
}

parameters.table <- '
population "--population " i (2, 10)
short_improvement "--short_improvement " i (1, 5000)
long_impovement "--long_improvement " i (1, 5000)
stagnation_iterations "--stagnation_iterations " i (1, 1000)
stagnation_min "--stagnation_min " r (0.0, 1.0)
stagnation_max "--stagnation_max " r (0.0, 1.0)
tenure_min "--tenure_min " r (0.5, 1.5)
tenure_max "--tenure_max " r (0.5, 1.5)
jump_magnitude "--jump_magnitude " r (0.0, 1.0)
min_directed_pertubation "--min_directed_pertubation " r (0.0, 1.0)
'

parameters <- readParameters(text = parameters.table)


result <- irace(tunerConfig = list(
	  hookRun = hook.run,
	  instances = instances[1:100],
	  maxExperiments = experiments,
	  logFile = ""),
	  parameters = parameters)

candidates.print(result)

