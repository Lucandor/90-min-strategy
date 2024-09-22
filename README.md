# 90 Minute Challenge Strategy Helper

## Overview

The **90 Minute Challenge Strategy Helper** is a tool designed to optimize your performance in the game by calculating the best strategies to minimize total fight times.

## Requirements

To run this project, ensure you have the following installed:

- **Python 3.x**: For processing CSV files.
- **g++ (GNU C++ Compiler)**: For compiling the C++ code.
- **Bash**: To execute the provided bash script.

## Configuration

1. **Strategies**: Add or modify strategies in `config_files/strategies`.
2. **Initial State**: Enter your initial times, time remaining, and fight state id in `config_files/initial_state.csv`.


## Running the Code

Execute the following command to run the tool:

```bash
./run.sh
```

## Memory and Speed Considerations

This project employs a [dynamic programming](https://en.wikipedia.org/wiki/Dynamic_programming) approach, calculating solutions for the entire state space "below" your starting state. The size of the state space is determined as follows:

\[ \text{Size of state space} = x \times y \times z_1 \times z_2 \times ... \times z_{14} \]

Where:
- **x** = Number of fight states (currently 23)
- **y** = `timeRemaining` in `initial_state.csv` divided by 5
- **zᵢ** = Number of possible times for fight i

For instance, if your current time on Kaiser is 36.48 and the strategy configuration allows times of 36.00, 36.82, and 37.25, then \( z_2 = 2 \) (considering 36.00 and 36.48).

### Memory Usage

The total memory usage will be the size of the state space multiplied by 8 bytes. By default, the program will terminate if the state space exceeds 50 million (approximately 400 MB).

## When to Use This Tool

This tool is most effective when you have relatively good times on each fight and seek to optimize further. It scales well with the number of strategies and time remaining, but may struggle with many potential times below your current records.

## Output

The output presents each possible decision from your current state, along with the expected time if that decision is followed (and subsequent optimal decisions thereafter). The times may appear similar due to the nature of optimal path calculations, akin to taking detours that delay your progress.

The two letters following the strategy name indicate next steps:
- **First letter**: Action if you achieve a personal best (PB) on that fight.
- **Second letter**: Action if you do not.

Options:
- `r`: reset
- `p`: proceed to the next fight
- `l`: lose

### Example Output

```
Decision "hippo_IL rr" - 14:35.81
Decision "tyson rr" - 14:35.84
Decision "macho_IL rr" - 14:36.02
...
```

## Planned Improvements

- Enhance user interaction for real-time updates during runs (e.g., entering fight times, auto-updating state, clearer output).

---

Feel free to modify any sections further!