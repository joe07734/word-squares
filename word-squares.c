#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "m_apm.h"


#define TRUE 1
#define FALSE 0



typedef struct {
	int length;
	int word_length;
	char *words;
} word_list_t;


void words_of_length(word_list_t *new_list, int len) {

	FILE *word_file = NULL;
	int list_max;
	size_t line_len;
	char *word;

	assert(new_list != NULL);
	if (len < 1) goto error;

	new_list->word_length = len;
	list_max = 1000;
	new_list->length = 0;
	new_list->words = malloc(list_max * len);  if (!new_list->words) goto error;

	word_file = fopen("words.txt", "r");  if (!word_file) goto error;
	while (word = fgetln(word_file, &line_len)) {

		if (line_len - 1 == len) {

			if (new_list->length == list_max) {
				list_max += 1000;
				new_list->words = realloc(new_list->words, list_max * len);  if (!new_list->words) goto error;
			}
			memcpy(&new_list->words[new_list->length * len], word, len);
			new_list->length += 1;

		}

	}
	fclose(word_file);
	
	new_list->words = realloc(new_list->words, new_list->length * len);  if (!new_list->words) goto error;
	return;

error:
	if (word_file) fclose(word_file);
	if (new_list->words) {
		free(new_list->words);
		new_list->words = NULL;
	}
	new_list->length = 0;

}


void stems_of_words_of_length(word_list_t *new_list, word_list_t *words, int len) {

	int list_max;
	int i;
	char *last_stem = NULL;

	assert(new_list != NULL);
	assert(words != NULL);
	if (len < 1 || len > words->word_length) goto error;

	new_list->word_length = len;
	list_max = 1000;
	new_list->length = 0;
	new_list->words = malloc(list_max * len);  if (!new_list->words) goto error;
	for (i = 0; i < words->length; ++i) {

		char *word = &words->words[i * words->word_length];

		if (last_stem == NULL || memcmp(last_stem, word, len) != 0) {

			if (new_list->length == list_max) {
				list_max += 1000;
				new_list->words = realloc(new_list->words, list_max * len);  if (!new_list->words) goto error;
			}
			memcpy(&new_list->words[new_list->length * len], word, len);
			new_list->length += 1;

			last_stem = word;

		}

	}

	return;

error:
	if (new_list->words) {
		free(new_list->words);
		new_list->words = NULL;
	}
	new_list->length = 0;

}


void commafy(char *string) {

// doesn't handle negative numbers

	char *s, *d;
	int i, nc;

	i = strlen(string);
	nc = (i - 1) / 3;
	if (nc > 0) {
		s = &string[i];
		d = &string[i + nc];
		*d-- = *s--;  // copy the null terminator
		while (nc-- > 0) {
			*d-- = *s--;
			*d-- = *s--;
			*d-- = *s--;
			*d-- = ',';
		}
	}

}


void mprint_commafied(M_APM n) {

	int len, nc;
	char *string;

	len = m_apm_significant_digits(n);
	nc = (len - 1) / 3;
	len += nc + 10;

	string = malloc(len);  if (!string) goto error;

	m_apm_to_integer_string(string, n);
	commafy(string);
	fprintf(stderr, "%s", string);

	free(string);

error:;

}


void print_commafied(long n) {

	char string[100];

	snprintf(string, 70, "%ld", n);
	commafy(string);
	fprintf(stderr, "%s", string);

}


void mprint_fixed(M_APM n, int places) {

	int len;
	char *string;

	len = m_apm_significant_digits(n);
	len += 10;

	string = malloc(len);  if (!string) goto error;

	m_apm_to_fixpt_string(string, places, n);
	fprintf(stderr, "%s", string);

	free(string);

error:;

}


mprint_duration(M_APM seconds) {

	static char *duration_names[] = {"year", "day", "hour", "minute", "second"};
	static unsigned long duration_in_seconds[] = {31556926, 86400, 3600, 60, 1};  // cf. Rent, "Seasons of Love"
	static M_APM mduration_in_seconds[5];
	static M_APM dur;
	int i;

	if (mduration_in_seconds[0] == NULL) {
		for (i = 0; i < 5; ++i) {
			mduration_in_seconds[i] = m_apm_init();
			m_apm_set_long(mduration_in_seconds[i], duration_in_seconds[i]);
		}
		dur = m_apm_init();
	}

	for (i = 0; i < 5; ++i) {

		int comp;

		m_apm_integer_divide(dur, seconds, mduration_in_seconds[i]);
		comp = m_apm_compare(dur, MM_One);
		if (comp > 0) {  // that is, dur > 1
			mprint_commafied(dur);
			fprintf(stderr, " %s%s", duration_names[i], comp == 0 ? "" : "s");
			break;
		}

	}

}


print_word(char *word, int len) {

	int i;

	for (i = 0; i < len; ++i)
		putchar(word[i]);

}


print_spaced_word(char *word, int len) {

	int i;

	putchar(word[0]);
	for (i = 1; i < len; ++i) {
		putchar(' ');
		putchar(word[i]);
	}

}


inspect_word_list(word_list_t *list) {

	int i;
	int len;

	assert(list != NULL);
	len = list->word_length;

	printf("Word length: %d, number of words: %d\n", len, list->length);
	for (i = 0; i < list->length; ++i) {
		printf("%d: ", i);
		print_word(&list->words[i * len], len);
		printf("\n");
	}

}


unsigned long long time_now(void) {

	struct timeval tp;

	gettimeofday(&tp, NULL);
	return ((unsigned long long) tp.tv_sec * 1000) + tp.tv_usec / 1000;

}


#define MAX_LETTERS		15

typedef int (*compare_word_t) (const void *, const void *);

int compare_words1 (const void *key, const void *elem) { return memcmp(key, elem,  1); }
int compare_words2 (const void *key, const void *elem) { return memcmp(key, elem,  2); }
int compare_words3 (const void *key, const void *elem) { return memcmp(key, elem,  3); }
int compare_words4 (const void *key, const void *elem) { return memcmp(key, elem,  4); }
int compare_words5 (const void *key, const void *elem) { return memcmp(key, elem,  5); }
int compare_words6 (const void *key, const void *elem) { return memcmp(key, elem,  6); }
int compare_words7 (const void *key, const void *elem) { return memcmp(key, elem,  7); }
int compare_words8 (const void *key, const void *elem) { return memcmp(key, elem,  8); }
int compare_words9 (const void *key, const void *elem) { return memcmp(key, elem,  9); }
int compare_words10(const void *key, const void *elem) { return memcmp(key, elem, 10); }
int compare_words11(const void *key, const void *elem) { return memcmp(key, elem, 11); }
int compare_words12(const void *key, const void *elem) { return memcmp(key, elem, 12); }
int compare_words13(const void *key, const void *elem) { return memcmp(key, elem, 13); }
int compare_words14(const void *key, const void *elem) { return memcmp(key, elem, 14); }
int compare_words15(const void *key, const void *elem) { return memcmp(key, elem, 15); }

compare_word_t compare_words[MAX_LETTERS+1] = {
	NULL,
	compare_words1, compare_words2, compare_words3, compare_words4, compare_words5,
	compare_words6, compare_words7, compare_words8, compare_words9, compare_words10,
	compare_words11, compare_words12, compare_words13, compare_words14, compare_words15
};


int main(int argc, char **argv) {

	word_list_t words, stems[MAX_LETTERS];
	int letters, i;
	int *shuffle;
	char *row_words[MAX_LETTERS+1];
	int index[MAX_LETTERS+1];
	int row;

	unsigned long long start_time, last_time, next_time;
	int throttle = 0;

	M_APM progress_increment[MAX_LETTERS+1];
	M_APM found, progress, last_progress;
	M_APM possibilities, mtmp, words_length, mletters;
	M_APM remaining_seconds, per_second, percent;
	M_APM m100, m1000, mprog, mtime;

	srand(time(0));

	found = m_apm_init();
	progress = m_apm_init();
	last_progress = m_apm_init();
	possibilities = m_apm_init();
	mtmp = m_apm_init();
	words_length = m_apm_init();
	mletters = m_apm_init();

	remaining_seconds = m_apm_init();
	per_second = m_apm_init();
	percent = m_apm_init();
	m100 = m_apm_init();
	m1000 = m_apm_init();
	mprog = m_apm_init();
	mtime = m_apm_init();

	m_apm_set_long(m100, 100);
	m_apm_set_long(m1000, 1000);

	m_apm_set_long(found, 0);
	m_apm_set_long(progress, 0);
	m_apm_set_long(last_progress, 0);

	if (argc == 2) {
		letters = atoi(argv[1]);
	} else
		letters = 3;
	assert(letters <= MAX_LETTERS);

	words_of_length(&words, letters);
	fprintf(stderr, "Dictionary contains ");
	print_commafied(words.length);
	fprintf(stderr, " %d-letter words.\n", letters);

	m_apm_set_long(mletters, letters);

	m_apm_set_long(possibilities, 1);
	m_apm_set_long(words_length, words.length);
	for (i = 0; i < letters; ++i) {
		m_apm_multiply(mtmp, possibilities, words_length);
		m_apm_copy(possibilities, mtmp);
	}
	fprintf(stderr, "There are ");
	mprint_commafied(possibilities);
	fprintf(stderr, " possible word squares.\n");

	for (i = 1; i < letters; ++i) {
		stems_of_words_of_length(&stems[i], &words, i);
	}

	shuffle = malloc(sizeof(int) * words.length);  if (!shuffle) goto error;
	for (i = 0; i < words.length; ++i) {
		shuffle[i] = i;
	}
#if 1
	for (i = 0; i < words.length; ++i) {
		int t, j = rand() % words.length;
		t = shuffle[i]; shuffle[i] = shuffle[j]; shuffle[j] = t;
	}
#endif

	progress_increment[letters] = m_apm_init();
	m_apm_set_long(progress_increment[letters], 1);
	for (i = letters - 1; i > 0; --i) {
		progress_increment[i] = m_apm_init();
		m_apm_multiply(progress_increment[i], progress_increment[i+1], words_length);
	}

	start_time = last_time = time_now();
	next_time = last_time + 1000;

	row = 1;
	index[row] = -1;

	while (1) {

		char *row_word;

		if (throttle-- <= 0) {

			throttle = 10000;
			unsigned long long now = time_now();

			if (now >= next_time) {

				m_apm_subtract(mprog, progress, last_progress);
				m_apm_set_long(mtime, now - last_time);
				m_apm_divide(mtmp, 20, mprog, mtime);
				m_apm_multiply(per_second, mtmp, m1000);

//				per_ms = (progress - last_progress) / (now - last_time);

				m_apm_divide(mprog, 20, progress, possibilities);
				m_apm_multiply(percent, mprog, m100);

//				percent = progress / (possibilities / (100 * 1000));
//				percent /= 1000;

				m_apm_divide(mprog, 20, possibilities, progress);
				m_apm_set_long(mtime, now - start_time);
				m_apm_divide(mtmp, 20, mtime, m1000);
				m_apm_multiply(remaining_seconds, mtmp, mprog);

				mprint_commafied(progress);
				fprintf(stderr, "   ");
				mprint_fixed(percent, 5);
				fprintf(stderr, "%%   ");
				mprint_commafied(found);
				fprintf(stderr, "   ");
				mprint_commafied(per_second);
				fprintf(stderr, " /sec   ");
				mprint_duration(remaining_seconds);
				fprintf(stderr, " remaining\n");
	
				m_apm_copy(last_progress, progress);
				last_time = next_time;
				next_time = now + 1000;

			}
		}

		index[row]++;
		if (index[row] == words.length) {
			if (--row == 0) break;
			continue;
		}

		row_word = &words.words[letters * shuffle[index[row]]];

		if (row > 1) {
			int include = FALSE;
			for (i = 1; i < row; ++i) {
				if (row_word == row_words[i]) {
					include = TRUE;
					break;
				}
			}
			if (include) {
				m_apm_add(mtmp, progress, progress_increment[row]);  // skipping this one and all the possibilities beneath it
				m_apm_copy(progress, mtmp);
				continue;
			}
		}

		row_words[row] = row_word;

		if (row < letters) {

			int r, c, ok;
			char col_stem[MAX_LETTERS];
			int col_stem_length;
			void *match;

			ok = TRUE;
			for (c = 0; c < letters; ++c) {

				col_stem_length = 0;
				for (r = 1; r <= row; ++r) {
					col_stem[col_stem_length++] = row_words[r][c];
				}
				
				match = bsearch(col_stem, stems[row].words, stems[row].length, stems[row].word_length, compare_words[row]);
				if (!match) {
					ok = FALSE;
					break;
				}

			}

			if (!ok) {
				m_apm_add(mtmp, progress, progress_increment[row]);  // skipping this one and all the possibilities beneath it
				m_apm_copy(progress, mtmp);
				continue;
			}

			row++;  // moving down to next row
			index[row] = -1;

		} else {

			int r, c, ok;
			char col_word[MAX_LETTERS];
			int col_word_length;
			char *col_words[MAX_LETTERS];
			int col_words_length;
			void *match;

			m_apm_add(mtmp, progress, MM_One);
			m_apm_copy(progress, mtmp);

			ok = TRUE;
			col_words_length = 0;
			for (c = 0; c < letters; ++c) {

				col_word_length = 0;
				for (r = 1; r <= letters; ++r) {
					col_word[col_word_length++] = row_words[r][c];
				}

				match = bsearch(col_word, words.words, words.length, letters, compare_words[letters]);
				if (!match) {
					ok = FALSE;
					break;
				}

				for (i = 1; i <= row; ++i) {
					if (match == row_words[i]) {
						ok = FALSE;
						break;
					}
				}
				if (!ok) break;

				for (i = 0; i < col_words_length; ++i) {
					if (match == col_words[i]) {
						ok = FALSE;
						break;
					}
				}
				if (!ok) break;

				col_words[col_words_length++] = match;

			}

			if (!ok) continue;

			m_apm_add(mtmp, found, MM_One);
			m_apm_copy(found, mtmp);
#if 1
			printf("\n");
			for (i = 1; i <= letters; ++i) {
				print_spaced_word(row_words[i], letters);
				printf("   %d\n", index[i]);
			}
			printf("\n");
			fflush(stdout);
#endif

		}

	}

	m_apm_free_all_mem();

	return 0;


error:
	fprintf(stderr, "error\n");
	return 1;

}
