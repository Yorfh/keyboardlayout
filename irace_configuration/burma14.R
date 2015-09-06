# ========================================================================= #
# Optimizes the parameters for the burma14 traveling salesman problem       #
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

instances <- floor(runif(200, min=0, max=4294967295 + 1))

hook.run <- function(instance, candidate, extra.params = NULL, config = list())
{
  args <- sprintf("--max_t %f --min_t %f --steps %i --fast_max_t %f --fast_min_t %f --fast_steps %i --evaluations 20000 --seed %s --test burma14", candidate$values[["max_t"]], candidate$values[["min_t"]], candidate$values[["steps"]], candidate$values[["fast_max_t"]], candidate$values[["fast_min_t"]], candidate$values[["fast_steps"]], as.character(instance)) 
  output <- system2(EXE, args=args, stdout=TRUE, stderr=TRUE)
  return(as.numeric(output[1]))
}

parameters.table <- '
max_t "" r (0, 1000)
min_t "" r (0, 1000)
steps "" i (1, 1000)
fast_max_t "" r (0, 1000)
fast_min_t "" r (0, 1000)
fast_steps "" i (1, 1000)
'

parameters <- readParameters(text = parameters.table)

result <- irace(tunerConfig = list(
                  hookRun = hook.run,
                  instances = instances[1:100],
                  maxExperiments = 1000,
				  forbiddenFile = "parameters.forbidden",
                  logFile = ""),
                parameters = parameters)

candidates.print(result)
