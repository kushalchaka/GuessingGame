# Word Guessing Game
A simple client-server implementation of a word guessing game using FIFOs for inter-process communication. The server selects a random word from a dictionary, and clients attempt to guess it letter by letter. Tracks wrong guesses and reveals the word when the game ends.

To run, compile both programs and run gserver. Using the output from gserver, use that as the input when running gclient
