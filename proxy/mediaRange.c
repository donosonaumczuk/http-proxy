#include <mediaRange.h>
#include <management.h>
#include <utilities.h>

#define isOWS(a) (a == ' ' || a == '\t')

enum matchResultOption { CAN, CANT };

typedef struct MediaRanges {
	char **listMediaTypes;
	int *canBeMatch;
	int length;
} MediaRange_t;

MediaRangePtr_t
createMediaRangeFromListOfMediaType(MediaRangePtr_t mediaRange) {
	char **listOfMediaTypes = mediaRange->listMediaTypes;
	int length				= mediaRange->length;
	MediaRangePtr_t mrp		= calloc(1, sizeof(MediaRange_t));

	int i, j = 0;
	while (j < length) {
		i = -1;
		if (j % BLOCK == 0) {
			mrp->listMediaTypes =
				realloc(mrp->listMediaTypes, j + BLOCK * sizeof(char *));
			memset(mrp->listMediaTypes + j * sizeof(char *), 0,
				   BLOCK * sizeof(char *));
		}
		do {
			i++;
			if (i % BLOCK == 0) {
				mrp->listMediaTypes[j] =
					realloc(mrp->listMediaTypes[j], i + BLOCK * sizeof(char));
			}
			mrp->listMediaTypes[j][i] = listOfMediaTypes[j][i];
		} while (listOfMediaTypes[j][i] != '\0');
		j++;
	}

	mrp->length		= j;
	mrp->canBeMatch = calloc(mrp->length, sizeof(int));
	return mrp;
}

MediaRangePtr_t createMediaRange(char const *string) {
	MediaRangePtr_t mrp = calloc(1, sizeof(MediaRange_t));
	mrp->listMediaTypes = calloc(1, sizeof(char *));

	addMediaRange(mrp, string);

	mrp->canBeMatch = calloc(mrp->length, sizeof(int));
	return mrp;
}

void addMediaRange(MediaRangePtr_t mrp, char const *string) {
	if (((mrp->length) % BLOCK) == 0) {
		mrp->listMediaTypes = realloc(mrp->listMediaTypes,
									  ((mrp->length) + BLOCK) * sizeof(char *));
	}
	int i = 0, j = 0;
	while (string[i] != '\0') {
		if (string[i] == ',') {
			if (((mrp->length) % BLOCK) == 0) {
				mrp->listMediaTypes =
					realloc(mrp->listMediaTypes,
							((mrp->length) + BLOCK) * sizeof(char *));
			}
			mrp->listMediaTypes[mrp->length][j] = '\0';
			(mrp->length)++;
			j = 0;

			// ignore OWS after ','
			i++;
			while (isOWS(string[i]) && string[i] != '\0') {
				i++;
			}
			i--;
			// ignore OWS after ','
		}
		else {
			if ((j % BLOCK) == 0) {
				if (j == 0) {
					mrp->listMediaTypes[mrp->length] = NULL;
				}
				mrp->listMediaTypes[mrp->length] =
					realloc(mrp->listMediaTypes[mrp->length],
							(j + BLOCK) * sizeof(char));
			}
			mrp->listMediaTypes[mrp->length][j] = string[i];
			j++;
		}
		i++;
	}

	mrp->listMediaTypes[mrp->length][j] = '\0';
	(mrp->length)++;

	generateAndUpdateTimeTag(MIME_ID);
}

enum matchResult doesMatchAt(int n, char mediaTypeCharAtN,
							 MediaRangePtr_t mediaRange) {
	enum matchResult ans = NO;
	for (int i = 0; i < mediaRange->length; i++) {
		if (mediaRange->canBeMatch[i] == CAN) {
			if (mediaRange->listMediaTypes[i][n] == mediaTypeCharAtN) {
				ans = YES;
			}
			else if (mediaRange->listMediaTypes[i][n] == '*') {
				ans = ALL;
				return ans;
			}
			else {
				mediaRange->canBeMatch[i] = CANT;
			}
		}
	}
	return ans;
}

void resetMediaRange(MediaRangePtr_t mediaRange) {
	int i = 0;
	while (i < mediaRange->length) {
		mediaRange->canBeMatch[i] = CAN;
		i++;
	}

	return;
}

void freeMediaRange(MediaRangePtr_t mrp) {
	int j = 0;
	while (j < mrp->length) {
		free(mrp->listMediaTypes[j]);
		j++;
	}
	free(mrp->listMediaTypes);
	free(mrp->canBeMatch);
	free(mrp);
}

char *getMediaRangeAsString(MediaRangePtr_t mrp) {
	char *ans			 = NULL;
	unsigned int sizeAns = 0;
	int i = 0, j = 0;
	while (j < mrp->length) {
		if (sizeAns != 0) {
			ans = addCharToString(ans, &sizeAns, ',');
			ans = addCharToString(ans, &sizeAns, ' ');
		}
		while ((mrp->listMediaTypes)[j][i] != '\0') {
			if (sizeAns == 0 && strcmp((mrp->listMediaTypes)[j], ";") == 0) {
				break;
			}
			ans = addCharToString(ans, &sizeAns, (mrp->listMediaTypes)[j][i]);
			i++;
		}
		i = 0;
		j++;
	}

	ans = addCharToString(ans, &sizeAns, '\0');

	return ans;
}
