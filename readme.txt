DNSD
This is a psudo DNS Server, for it does not maintain the resourse records or a zone file itself. It just receives request from client, queries the known NS Server and then return a response to client. You may call it a soft-router which only handles DNS messages:)

INSTALL
To install, change dir to dnsd where lies the makefile, and type:
	make
You will have the excutable in dnsd/bin/main.
To run the program, just type:
	./bin/main
Make sure that the black.list is in dnsd/ when you run the program.

FILE STRUCTURE

|-- black.list			// the black list for the domains to block
|-- inc					// includes
|   |-- black_list.h	// a hash table for storing black list entries
|   |-- common.h		// common definitions 
|   |-- dns_consts.h	// consts relating to DNS protocol
|   |-- hhrt.h			// the half handled request table(see the documentation for details)
|   |-- protocol.h		// DNS protocol handling
|   |-- req_queue.h		// the request queue
|   `-- util.h			// some auxialary routines
|-- makefile			// makefile
|-- readme.txt			// this file
`-- src					// source file, corresponding to the inc/
    |-- black_list.c
    |-- hhrt.c
    |-- main.c
    |-- protocol.c
    |-- req_queue.c
    `-- util.c
