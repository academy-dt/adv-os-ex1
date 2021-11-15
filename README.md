# Piper

## Part A

The purpose is to simulate bash piping, e.g `ls -l /tmp | sort | head -n 2` by running `./piper "ls -l /tmp" "sort" "head -n 2"`.
The implementation is not perfect, specifically:
- When parsing every quote-enclosed command line, we allow up to 128 arguments, additional arguments won't be handled correctly.

## Part B

Consider the command `ps aux | grep pizza-margarita`.
Assuming there are no processes with the expression `pizza-margarita`, when we execute the command above in a terminal, we create
a new shell process. That process will fork and exec both children (`ps aux` and `grep pizza-margarita`).
So basically, grep-ing an expression creates a process that matches the same expression, and we will see that process as the output.
