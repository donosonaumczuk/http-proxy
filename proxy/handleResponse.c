#include <handleResponse.h>
#include <http.h>
#include <httpProxyADT.h>
#include <headersParser.h>
#include <stdio.h>
#include <selector.h>
#include <transformBody.h>
#include <configuration.h>
#include <utilities.h>
#include <logger.h>

/*
 * Returns buffer to read from to write on clientFd
 */
static buffer *getCurrentResponseBuffer(httpADT_t state);

void responseInit(const unsigned state, struct selector_key *key) {
	struct handleResponse *handleResponse =
		getHandleResponseState(GET_DATA(key));
	headersParserInit(&(handleResponse->parseHeaders), key, FALSE);
	buffer_init(&(handleResponse->requestDataBuffer), BUFFER_SIZE,
				handleResponse->requestData);
	handleResponse->responseFinished = FALSE;
	logAccess(GET_DATA(key), REQ);
}

void responseDestroy(const unsigned state, struct selector_key *key) {
	struct handleResponse *handleResponse =
		getHandleResponseState(GET_DATA(key));
	int aux = getTransformContentParser(&(handleResponse->parseHeaders));
	if (getTransformEncode(&handleResponse->parseHeaders)) {
		setTransformContent(GET_DATA(key), aux);
	}
	else {
		setTransformContent(GET_DATA(key), FALSE);
	}
	setIsChunked(GET_DATA(key), handleResponse->parseHeaders.isChunked);
	logAccess(GET_DATA(key), RESP);
}

unsigned responseRead(struct selector_key *key) {
	buffer *writeBuffer = getWriteBuffer(GET_DATA(key));
	struct handleResponse *handleResponse =
		getHandleResponseState(GET_DATA(key));
	unsigned ret = HANDLE_RESPONSE;
	uint8_t *pointer;
	size_t count;
	ssize_t bytesRead;

	if (key->fd == getClientFd(GET_DATA(key))) {
		return readFromClient(key);
	}

	if (handleResponse->parseHeaders.state == BODY_START &&
		getIsTransformationOn(getConfiguration())) {
		return TRANSFORM_BODY;
	}
	// if there is no space to read should write what i already read
	if (!buffer_can_write(writeBuffer)) {
		// set interest no op on fd an write on origin fd
		return setResponseFdInterests(key);
	}

	pointer   = buffer_write_ptr(writeBuffer, &count);
	bytesRead = recv(key->fd, pointer, count, 0);

	if (bytesRead > 0) {
		int begining = pointer - writeBuffer->data;
		buffer_write_adv(writeBuffer, bytesRead);
		if (handleResponse->parseHeaders.state != BODY_START) {
			parseHeaders(&handleResponse->parseHeaders, writeBuffer, begining,
						 begining + bytesRead);
		}
		ret = setResponseFdInterests(key);
	}
	else if (bytesRead == 0) {
		handleResponse->responseFinished = TRUE;
		if (!buffer_can_read(writeBuffer)) {
			setErrorDoneFd(key);
			ret = DONE;
		}
		else {
			ret = setResponseFdInterests(key);
		}
	}
	else {
		ret = ERROR;
	}

	return ret;
}

unsigned readFromClient(struct selector_key *key) {
	struct handleResponse *handleResponse =
		getHandleResponseState(GET_DATA(key));
	buffer *writeBuffer = &handleResponse->requestDataBuffer;
	unsigned ret		= HANDLE_RESPONSE;
	uint8_t *pointer;
	size_t count;
	ssize_t bytesRead;

	// if there is no space to read should write what i already read
	if (!buffer_can_write(writeBuffer)) {
		// set interest no op on fd an write on origin fd
		return setResponseFdInterests(key);
	}

	pointer   = buffer_write_ptr(writeBuffer, &count);
	bytesRead = recv(key->fd, pointer, count, 0);

	if (bytesRead > 0) {
		buffer_write_adv(writeBuffer, bytesRead);
		ret = setResponseFdInterests(key);
	}
	else if (bytesRead == 0) {
		setErrorDoneFd(key);
		ret = DONE;
	}
	else {
		setErrorDoneFd(key);
		ret = ERROR;
	}

	return ret;
}

unsigned responseWrite(struct selector_key *key) {
	buffer *writeBuffer = getCurrentResponseBuffer(GET_DATA(key));
	struct handleResponse *handleResponse =
		getHandleResponseState(GET_DATA(key));
	buffer *parsedBuffer = &(handleResponse->parseHeaders.valueBuffer);

	unsigned ret = HANDLE_RESPONSE;
	uint8_t *pointer;
	size_t count;
	ssize_t bytesRead;
	if (key->fd == getOriginFd(GET_DATA(key))) {
		return writeToOrigin(key);
	}

	if (handleResponse->parseHeaders.state == BODY_START &&
		getIsTransformationOn(getConfiguration()) &&
		!buffer_can_read(parsedBuffer)) {
		return TRANSFORM_BODY;
	}

	// if everything is read on buffer
	if (!buffer_can_read(writeBuffer)) {
		// set interest no op on fd an read on client fd
		return setResponseFdInterests(key);
	}

	pointer   = buffer_read_ptr(writeBuffer, &count);
	bytesRead = send(key->fd, pointer, count, 0);

	if (bytesRead > 0) {
		buffer_read_adv(writeBuffer, bytesRead);
		increaseTransferBytes(bytesRead);
		ret = setResponseFdInterests(key);
	}
	else if (handleResponse->responseFinished == TRUE &&
			 !buffer_can_read(writeBuffer)) {
		setErrorDoneFd(key);
		return DONE;
	}
	else {
		setErrorDoneFd(key);
		ret = ERROR;
	}

	return ret;
}

unsigned writeToOrigin(struct selector_key *key) {
	struct handleResponse *handleResponse =
		getHandleResponseState(GET_DATA(key));
	buffer *writeBuffer = &handleResponse->requestDataBuffer;
	unsigned ret		= HANDLE_RESPONSE;
	uint8_t *pointer;
	size_t count;
	ssize_t bytesRead;

	// if everything is read on buffer
	if (!buffer_can_read(writeBuffer)) {
		// set interest no op on fd an read on client fd
		return setResponseFdInterests(key);
	}

	pointer   = buffer_read_ptr(writeBuffer, &count);
	bytesRead = send(key->fd, pointer, count, 0);

	if (bytesRead > 0) {
		buffer_read_adv(writeBuffer, bytesRead);
		increaseTransferBytes(bytesRead);
		ret = setResponseFdInterests(key);
	}
	else {
		setErrorDoneFd(key);
		ret = ERROR;
	}

	return ret;
}

unsigned setResponseFdInterests(struct selector_key *key) {
	httpADT_t state						  = GET_DATA(key);
	struct handleResponse *handleResponse = getHandleResponseState(state);
	buffer *writeBuffer					  = getWriteBuffer(GET_DATA(key));
	buffer *parsedBuffer  = &(handleResponse->parseHeaders.valueBuffer);
	buffer *requestBuffer = &(handleResponse->requestDataBuffer);
	;
	unsigned ret	   = HANDLE_RESPONSE;
	int clientInterest = OP_NOOP;
	int originInterest = OP_NOOP;

	if (buffer_can_read(parsedBuffer) ||
		(handleResponse->parseHeaders.state == BODY_START &&
		 buffer_can_read(writeBuffer))) {
		clientInterest |= OP_WRITE;
	}

	if (buffer_can_read(requestBuffer)) {
		originInterest |= OP_WRITE;
	}

	if (buffer_can_write(writeBuffer) && !buffer_can_read(parsedBuffer)) {
		originInterest |= OP_READ;
	}

	if (buffer_can_write(requestBuffer) && !buffer_can_read(requestBuffer)) {
		clientInterest |= OP_READ;
	}

	if (SELECTOR_SUCCESS !=
			selector_set_interest(key->s, getClientFd(state), clientInterest) ||
		SELECTOR_SUCCESS !=
			selector_set_interest(key->s, getOriginFd(state), originInterest)) {
		return ERROR;
	}

	return ret;
}

static buffer *getCurrentResponseBuffer(httpADT_t state) {
	struct handleResponse *handleResponse = getHandleResponseState(state);
	if (handleResponse->parseHeaders.state != BODY_START ||
		buffer_can_read(&handleResponse->parseHeaders.valueBuffer)) {
		return &handleResponse->parseHeaders.valueBuffer;
	}
	else {
		return getWriteBuffer(state);
	}
}
