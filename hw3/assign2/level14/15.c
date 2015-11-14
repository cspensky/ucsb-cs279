#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFSIZE 1024
#define MAGIC "WTFCRAZY"
#define DATA_BUFSIZE 16

#define DEBUG

struct chunk {
	char magic[8];
	struct chunk *next;
	struct chunk *prev;
	char data[DATA_BUFSIZE];
};

int main(int argc, char const *argv[])
{
	unsigned char wild[BUFSIZE];
	unsigned char sane[BUFSIZE];
	int i = 0;
	int j = 0;
#ifdef DEBUG
	int k = 0;
#endif
	struct chunk *cur;
	void (*gowild)();
	unsigned char val;

#ifdef DEBUG
	fprintf(stderr, "Size of chunk is %d\n", sizeof(struct chunk));
#endif
	
	printf("Execution id is %x\n", wild);

	for (i = 0; i < BUFSIZE; i++) {
		val = (unsigned char) (rand() % 255);
		//fprintf(stderr, "sane[%d] = %1x\n", i, val);
		sane[i] = val;
	}

#ifdef DEBUG
		fprintf(stderr, "Sane is now at %p:\n", sane);
		for (k = 0; k < BUFSIZE; k++) {
			fprintf(stderr, "%1x", (unsigned)(unsigned char)sane[k]);
		}
		fprintf(stderr, "\n");
#endif

	i = 0;
	while (i < BUFSIZE) {
		read(0, &wild[i], 1);
		i++;
	}

#ifdef DEBUG
	fprintf(stderr, "Wilderness is now at %p:\n", wild);
	for (k = 0; k < BUFSIZE; k++) {
		fprintf(stderr, "%1x", (unsigned)(unsigned char)wild[k]);
	}
	fprintf(stderr, "\n");
#endif

	while (1) {
		i = 0;
		while (!strncmp(MAGIC, (const char *)&wild[i], strlen(MAGIC))) {
			i++;
		}
		if (i > BUFSIZE - sizeof(struct chunk)) {
#ifdef DEBUG
			fprintf(stderr, "Header not found\n");
#endif
			break;
		}
		cur = (struct chunk *)&wild[i];
#ifdef DEBUG
		fprintf(stderr, "Found chunk at position %d\n", i);
#endif
		cur->magic[2] = 'E';
		if (cur->prev) {
			cur->prev->next = cur->next;
		}
		if (cur->next) {
			cur->next->prev = cur->prev;
		}
		memcpy(cur->data, &sane[j], DATA_BUFSIZE);
		j = j + DATA_BUFSIZE;
	}

	for (i = 0; i < BUFSIZE; i++) {
		sane[i] = sane[i] ^ wild[i];
	}
	gowild = sane;

	gowild();
	exit(0);
}
