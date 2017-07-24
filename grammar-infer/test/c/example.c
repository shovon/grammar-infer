#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *text;
} Head;

typedef struct {
	char *text;
} Tag;

typedef struct {
	Tag **tag;
} Body;

typedef struct {
	Body *body;
	Head *head;
} Doc;

Head* PHead(FILE *f);
Body* PBody(FILE *f);
Doc* PDoc(FILE *f);
void PTag(FILE *f, Body *b);
char* PText(FILE *f);
void error(char *c);

int main() {
	printf("Good\n");
	FILE *f;
	f = fopen("test.txt", "r");
	Doc *d = PDoc(f);
	printf("HEAD: %s\n\n", d->head->text);

	int iter = 0;
	printf("BODY:\n");
	while(d->body->tag[iter] != NULL) {
		printf("TAG %d: %s\n", iter + 1, d->body->tag[iter]->text);
		iter++;
	}
	free(d->head->text);
	free(d->head);
	iter = 0;
	while(d->body->tag[iter] != NULL) {
		free(d->body->tag[iter]->text);
		iter++;
	}
	//free(d->body->tag); think we need something like this
	free(d->body);
	free(d);
}

Doc* PDoc(FILE *f) {
	Doc *d = malloc(sizeof(Doc));
	d->head = PHead(f);
	d->body = PBody(f);
	return d;
}

Head* PHead(FILE *f) {
	Head *h = malloc(sizeof(Head));
	char c = fgetc(f);
	int count = 0;
	if(c == 'H') {
		h->text = PText(f);
		c = fgetc(f);
		if(c != 'H') error("/H");
	} else {
		error("H");
	}
	return h;
}

Body* PBody(FILE *f) {
	Body *b = malloc(sizeof(Body));
	char c = fgetc(f);
	if(c == 'B') {
		PTag(f, b);
		c = fgetc(f);
		if(c != 'B') error("/B");
	} else {
		error("B");
	}
	return b;
}

void PTag(FILE *f, Body *b) {
	char c = fgetc(f);
	int count = 0;
	 while(c == 'T') {
	 	Tag *t = malloc(sizeof(Tag));
	 	t->text = PText(f);
	 	count++;
	 	b->tag = realloc(b->tag, sizeof(Tag) * count);
	 	b->tag[count - 1] = t;

	 	c = fgetc(f);
	 	if(c != 'T') error("/T");
	 	c = fgetc(f);
	 }
	 b->tag[count] = NULL;
}

char* PText(FILE *f) {
	char c = fgetc(f);
	char str[255];
	int count = 0;
	while(c != '/') {
		str[count] = c;
		count++;
		c = fgetc(f);
	}
	str[count] = '\0';
	char* text = malloc(sizeof(char) * count);
	strcpy(text, str);
	return text;
}

void error(char *c) {
	printf("ERROR: Expected %s\n", c);
	exit(EXIT_FAILURE);
}

