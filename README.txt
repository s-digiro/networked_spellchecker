Sean DiGirolamo

Networked Spell Checker

Background:
	This program starts a server which users can connect to via commandline
	programs such as telnet. It uses concurrent threads to retrieve strings
	from the client, check if it is in the dictionary, and serve whether or
	not the word is misspelled back to the client. In addition, a thread is
	used to log the history of the server to a log file. The integrity of
	the program is preserved and race bugs are avoided with condition
	variables.

Usage:
	./spell_check-server [-d path/to/dictionary/file] [-p port_number]

	Where the dictionary file is a list of words seperated by return chars
	and port_number is the port that the program will listen for connections
	on.
