# 90 Minute Challenge Strategy Helper

## Requirements

To run this project, you will need the following installed on your system:

- **Python 3.x**: Required for processing CSV files.
- **g++ (GNU C++ Compiler)**: Required for compiling the C++ code.
- **Bash**: Required to execute the bash script.

## Config changes
- Add/modify the strategies you want to consider in `config_files/strategies`
- Enter your initial times and time remaining in `config_files/initial_state.csv`

## Running code

```bash
./run.sh
```
## Memory/Speed considerations
The code works using a [dynamic programming](https://en.wikipedia.org/wiki/Dynamic_programming) solution.  This means it calculates the solution for the entire state space "below" your starting state.  This state space can be quite large depending on your configuration.

The size of the state space is:  
x * y * z<sub>1</sub> * z<sub>2</sub> * ... * z<sub>14</sub>,  
where z is the number of fight states (currently 23),
where y is the `timeRemaining` in `initial_state.csv` divided by 5,  
and z<sub>i</sub> is the number of possible times you could end up with on fight i.  

For example:  
On kaiser, if your current time is 36.48 and in your strategy config files it is possible to get times 36.00, 36.82, and 37.25, then z<sub>2</sub> = 2, since the code must consider times 36.00 and 36.48.  

The total memory usage will be the size of the state space * 8 bytes.  By default the program shuts off if asked to solve a program with state space > 50 million (400 MB).  

## When the tool is useful
With this state space consideration, the code scales well with the number of strategies allowed and time remaining (linearly).  It scales poorly when you have many more possible times below your current times. For this reason the model is best used when you already have relatively good times on each fight, and are looking to optimize your times in the most efficient manner.

## Output
Example output is below.  Each possible decision from your current state is included, and the expected time from following that decision (and then all other optimal decisions after that).  For this reason the times look closer than how similar in value the decisions actually are.  Think of each option other than the top one as making a wrong turn on your way to a destination and then making a U-turn.  Some of those detours take more time than others, but none are optimal and all are calculated by getting back to the optimal path ASAP.  

The 2 letters after the strategy name indicate what to do next.  The first letter is what to do if you get a PB on that fight, and the second letter is what to do if you don't.  
r = reset  
p = proceed (to the next fight)  
l = lose  
```
Decision "hippo_IL rr" - 14:35.81
Decision "tyson rr" - 14:35.84
Decision "macho_IL rr" - 14:36.02
Decision "joe pp" - 14:36.04
Decision "honda2_SS rr" - 14:36.05
Decision "honda2_SS pr" - 14:36.05
Decision "macho_SS rr" - 14:36.05
Decision "joe rr" - 14:36.13
Decision "joe pr" - 14:36.13
Decision "don1 pp" - 14:36.13
Decision "don1 rr" - 14:36.14
Decision "don1 pr" - 14:36.14
Decision "macho_IL rl" - 14:36.20
Decision "macho_SS rl" - 14:36.24
Decision "honda2_SS pp" - 14:36.26
```

## Planned Improvements
- Make a better way to use this tool on the fly in a run (entering fight times, auto updating state, clearer output, etc.)