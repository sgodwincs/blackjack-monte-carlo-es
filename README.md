# Blackjack Monte Carlo Exploring Starts

----
## What is it?

A program written in both Python and C++ for finding the optimal policy for a game of blackjack. Specifically, the rules defined for this variation of blackjack are as follows:
 - Infinite number of decks are used, thus counting cards does not have any impact.
 - Dealer stick for a value of 17 or higher.
 - Player can only hit or stick.
 - All values of 21 are equal.

This was written to obtain a better understanding of the Monte Carlo approach for finding optimal policies in reinforcement learning as defined by the [Sutton and Barto Reinforcement Learning book](https://webdocs.cs.ualberta.ca/~sutton/book/the-book.html) second draft. This specific approach uses exploring starts with the state consisting of the following variables:
 - The player's current sum. This can be between 11 and 21.
 - The value of the card the dealer is showing. This can be between 1 and 10.
 - Whether or not the player currently has a usable ace.

 By generating these variables and an action from the generated state with equal probability and alternating between policy evaluation and improvement, eventually an optimal policy will be generated.

----
## How to run?

There is the Python version and there is the C++ version. The python version exists in the `python` directory and can by run by simply running `python blackjack.py`. By default, it runs for 2 million iterations and takes about a minute. The C++ version exists in the `c++` folder, uses premake5, and can be configured for your build tools via a command such as `premake5 vs2015`. By default, it runs for 1 billion iterations and takes around 30 minutes to complete, although this can be changed via the constant variable.

----
## Result

I initially had performance issues with the Python version before I made some optimizations (such as incremental averaging). As a result, I decided to work on a C++ version which I then made multithreaded. Since states with a specified dealer card have no possible actions leading to a state with a different dealer card, all subsets of states with different dealer cards can be treated independently which allows for easy multithreading.

Here is a possible result from the Python version (running for 2 million iterations):

![Python With Usable Ace](https://github.com/sgodwincs/blackjack-monte-carlo-es/blob/master/py_with_usable_ace.png) ![Python Without Usable Ace](https://github.com/sgodwincs/blackjack-monte-carlo-es/blob/master/py_without_usable_ace.png)

And the C++ version (running for 1 billion iterations):

![C++ With Usable Ace](https://github.com/sgodwincs/blackjack-monte-carlo-es/blob/master/c++_with_usable_ace.png) ![C++ Without Usable Ace](https://github.com/sgodwincs/blackjack-monte-carlo-es/blob/master/c++_without_usable_ace.png)

Forgot to label the graphs, but the X-axis is the dealer's showing card, the Y-axis is the player's current sum, red points are when you should hit, and finally blue points are when you should stick. The C++ version obviously generated the optimal policy because of more iterations, but the Python version is near optimal. Specifically, in some runs the Python version would find the optimal policy without usable aces, but it had a hard time finding the optimal policy with usable aces.
