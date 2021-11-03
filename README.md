# Piper

An academic exercise.
The purpose is to simulate bash piping, e.g `ls -l /tmp | sort | head -n 2` by running `./piper "ls -l /tmp" "sort" "head -n 2"`.
The implementation is not perfect, specifically:
- It won't work when passing big buffers between processes, because the producers are executed and waited on before reading from the pipe.
- When parsing every quote-enclosed command line, we allow up to 128 arguments, additional arguments won't be handled correctly.

