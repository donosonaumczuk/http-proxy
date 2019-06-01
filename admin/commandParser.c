#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <admin.h>
#include <commandParser.h>

int parseCommand(uint8_t *operation, uint8_t *id, void **data,
				 size_t *dataLength) {
	enum returnCode_t returnCode = IGNORE;
	enum state_t currentState	= NOTHING;
	char currentChar			 = getchar();
	do {
		switch (currentState) {
			case NOTHING:
				switch (currentChar) {
					case 'g':
						currentState = G;
						break;
					case 's':
						currentState = S;
						break;
					case 'b':
						currentState = B;
						break;
					case '.':
						currentState = DOT;
						break;
					case '\n':
					case '\t':
					case ' ': /* space */
						/* Keeps currentState = NOTHING */
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case G:
				EXPECTS('e', GE);
				break;
			case S:
				EXPECTS('e', SE);
				break;
			case B:
				EXPECTS('y', BY);
				break;
			case GE:
				EXPECTS('t', GET);
				break;
			case SE:
				EXPECTS('t', SET);
				break;
			case BY:
				EXPECTS('e', BYE);
				break;
			case GET:
				EXPECTS_SPACE(GET_);
				break;
			case SET:
				EXPECTS_SPACE(SET_);
				break;
			case BYE:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case '\n':
						/* TODO: set command information to *bye* */
						returnCode = NEW;
					default:
						returnCode = INVALID;
				}
				break;
			case DOT:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case '\n':
						returnCode = SEND;
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case GET_:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case 'm':
						currentState = GET_M;
						break;
					case 'b':
						currentState = GET_B;
						break;
					case 't':
						currentState = GET_T;
						break;
					case 'c':
						currentState = GET_C;
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case SET_:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case 'm':
						currentState = SET_M;
						break;
					case 'b':
						currentState = SET_B;
						break;
					case 't':
						currentState = SET_T;
						break;
					case 'c':
						currentState = SET_C;
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case GET_M:
				EXPECTS('t', GET_MT);
				break;
			case GET_MT:
				EXPECTS('r', GET_MTR);
				break;
			case GET_MTR:
				EXPECTS_SPACE(GET_MTR_);
				break;
			case GET_MTR_:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case 'c':
						currentState = GET_MTR_C;
						break;
					case 'h':
						currentState = GET_MTR_H;
						break;
					case 'b':
						currentState = GET_MTR_B;
						break;
					case '\n':
						/* TODO: set command information to *get mtr* */
						returnCode = NEW;
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case GET_MTR_C:
				EXPECTS('n', GET_MTR_CN);
				break;
			case GET_MTR_H:
				EXPECTS('s', GET_MTR_HS);
				break;
			case GET_MTR_B:
				EXPECTS('t', GET_MTR_BT);
				break;
			case GET_MTR_CN:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case '\n':
						/* TODO: set command information to *get mtr cn* */
						returnCode = NEW;
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case GET_MTR_HS:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case '\n':
						/* TODO: set command information to *get mtr hs* */
						returnCode = NEW;
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case GET_MTR_BT:
				switch (currentChar) {
					case '\t':
					case ' ': /* space */
						/* Keeps current state */
						break;
					case '\n':
						/* TODO: set command information to *get mtr bt* */
						returnCode = NEW;
						break;
					default:
						returnCode = INVALID;
				}
				break;
			case GET_B:
				EXPECTS('f', GET_BF);
				break;
		}
	} while (returnCode == IGNORE);

	if (returnCode == INVALID && currentChar != '\n') {
		while (currentChar != '\n') {
			currentChar = getchar();
		}
	}

	return returnCode;
}
