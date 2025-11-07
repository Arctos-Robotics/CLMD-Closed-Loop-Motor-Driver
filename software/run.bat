@echo off
echo Starting Closed Loop Motor Configuration Tool...
echo.

if not exist venv (
    echo Creating virtual environment...
    python -m venv venv
    echo.
)

echo Activating virtual environment...
call venv\Scripts\activate.bat

echo Installing dependencies...
pip install -q -r requirements.txt

echo.
echo Starting application...
python main.py

pause
