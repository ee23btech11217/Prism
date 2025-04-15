python3 src/utils/createBank.py files AudioBank audiobank.pab 

g++ -std=c++17 src/utils/pab_parser.cpp -o src/utils/pab_parser -I/opt/homebrew/include -L/opt/homebrew/lib -lportaudio
./src/utils/pab_parser AudioBank/audiobank.pab $1