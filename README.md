# Keylogger

Client: Records every keystroke typed and sends the data over using TCP connections.

Server: Uses Poll to accept incoming connections (MAX = 15) and writes to a file.
  - Takes TAB, SPACE, ENTER, and BACKSPACE into consideration


Possible Future Changes:
  - Clean up file after client connection ends
  - Search for frequencies of words

