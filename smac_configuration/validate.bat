set seed=%RANDOM%
call ../smac/smac-validate.bat --num-run %seed% --output-dir output\validate --use-scenario-outdir true --scenario-file %*
c:\python27\python analyze_validation_results.py --seed %seed% --output-dir output\validate --scenario %*
