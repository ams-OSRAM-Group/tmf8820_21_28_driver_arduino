# These are python files for the talk to arduino via python

Python version 3.6 or higher is required.

## Virtual environment

Recommendation is to set-up a virtual environment. Open you favorite Windows PowerShell, VisualStudio Code or Spyder.
To install a virtual environement named env, and use it:
python -m venv env     
./env/Scripts/Activate.ps1    

## Requirements

pip install -r ./requirements.txt


## Scripts 

Please check on which COM port the Arduino is connected to your PC. Correct the com port in the python script to match
your PC.

To run a simple python script do this:

python talk_to_3_arduino_1MBaud.py



