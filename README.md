# ICFPC2023 Team Manarimo

## Team Members
* Kenkou Nakamura (@kenkoooo)
* mkut
* Osamu Koga (@osa_k)
* Shunsuke Ohashi (@pepsin_amylase)
* Yu Fujikake (@yuusti)
* Yuki Kawata (@kawatea03)

## Strategy
### Algorithm
The first impression we got as soon as we read the problem was "Simulated Annealing (SA) must do extremely good job for this kind of problems 🤔." So we go straight ahead with this direction.

For the negibour states of SA, we used two transitions:

* Choose two musicians and swap their positions.
* Choose a musician and slightly move them to random direction for a random distance.

A challenge was how to calculate score quickly. We implemented incremental algorithm to update scores during SA. Also for quick full score calculation, we implemented an event-driven algorithm to determine if each attendee is visible from a musician.

## Solver Codes
* `kawatea/no_block.cpp`: Initial solver which doesn't consider blocking effect.
* `kawatea/block.cpp`: SA-based solver for lightning-round problems.
* `kawatea/block_pillar.cpp`: SA-based solver for full-contest problems.