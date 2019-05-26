#ifndef TARGET_PARSER_H
#define TARGET_PARSER_H

#include <stdint.h>
#include <buffer.h>
#include <stdlib.h>
#include <utilities.h>

enum targetState { // TODO: check ports
	START_T,
	A_AUTH_OR_A_SCHEMA,
	BAR_A_SCHEMA,
	D_BAR_A_SCHEMA,
	A_USERINFO,
	PORT_T,
	A_AUTH,
	END_T
};

struct targetParser {
	enum targetState state;
	int charactersRead;
	char *host;
	unsigned int sizeHost;
	char *target;
	unsigned int sizeTarget;
	int port;
};

#define PORT_DEFAULT 80

/*
 * Initialize parser
 */
void parseTargetInit(struct targetParser *parser);

int parseTargetChar(struct targetParser *parser, char l);

/*
 * Returns the current state of the target state machine
 */
unsigned getTargetState(struct targetParser *parser);

/*
 * Returns the host found by the state machine or null if not found
 */
char *getHost(struct targetParser *parser);

/*
 * Returns the target found by the state machine
 */
char *getTarget(struct targetParser *parser);

/*
 * Returns the port found by the parser
 */
int getPortTarget(struct targetParser *parser);

#endif
