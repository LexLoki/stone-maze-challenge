# Stone Automata Maze Challenge

## Intro
This repository represents my winning submission :trophy: for the 2023 Stone Automata Maze Challenge competition, hosted by SigmaGeek.

You can check the [results](https://sigmageek.com/stone_results/stone-automata-maze-challenge) and the [complete call](https://sigmageek.com/challenge/stone-automata-maze-challenge).

## Files

Each file reprents the code that generates my submitted solutions. Some of them, specially c5, should not work right up from the batch, because it requires automata states to be first stored in the HD. Since these files are being shared more for reference, I have not prepared them to be run "plug and play" (I might someday). Feel free to figure things out by yourself.


 - [bool](stone-automata%20-%20bool%20version.cpp): the first/most simple not optimized solution, for the first phase of the challenge
 - [c1](stone-automata%20-%20c1.cpp): Challenge 1 of the 2nd phase
 - [c2](stone-automata%20-%20c2.cpp): Challenge 2 of the 2nd phase
 - [c3](stone-automata%20-%20c3.cpp): Challenge 3 of the 2nd phase
 - [c4](stone-automata%20-%20c4.cpp): Challenge 4 of the 2nd phase
 - [c5-5stack_sides_push](stone-automata%20-%20c5-5stack_sides_push.cpp): Challenge 5 of the 2nd phase

 #### Additionally
 - [Notes](stone-sigma-challenge-notes.txt): development notes
 - [Puzzle solver](puzzle.cpp): challenge 4 requires solving first a 10x10 puzzle. This is the algorithm that solves PART of it - I have solved manually the majority of it and feed the rest to the algorithm to find.

 ## Solutions values found
 For quick reference:
 - Challenge 1: 6176 length path (optimal) => 1000 points (max: 1000)
 - Challenge 2: 6016 length path (optimal) => 1500 points (max: 1500)
 - Challenge 3: 6200 length path (suboptimal) => 1470.968 points (max: 1500)
 - Challenge 4: 2299 length path (optimal) => 2500 points (max: 2500)
 - Challenge 5: 10145 length path => 3500 points (max: 3500)

 ## Execution information

 I have compiled all of them with `g++ O3`

 The executables do not require parameters when run, since they are all defined in DEFINE derivatives or along the code, like every ~~bad~~ competition code

 Some information on reasoning, values found and execution times can by found in [notes](stone-sigma-challenge-notes.txt)

 ## Specs used
 - Processor: AMD Ryzen 7 5800H with Radeon Graphics 3.20 GHz
 - GPU: GeForce RTX 3070
 - Installed RAM: 16.0 GB
 - Storage: SSD
 - OS: Windows 11