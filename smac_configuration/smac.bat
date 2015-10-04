set seed=%RANDOM%
call ../smac/smac.bat --seed %seed% --output-dir output --scenario-file %*
c:\python27\python analyze_validation_results.py --seed %seed% --output-dir output --scenario-file %*
