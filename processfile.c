#include <stdio.h>
#include "processfile.h"
#include "processline.h"
#include "string.h"

/* Globals because we want to access them only across this file (for get_curr_line/curr_filename, mostly for printing errors) */
static char *curr_filename;
static int curr_line;

/* Fully processes the assembly file, and writing all the associated files. Returns whether succeeded. */
void process_file(char *filename) {
	int ic, dc, icf, dcf, is_error;
	char *input_filename;
	char temp_line[MAX_LINE_LENGTH+1]; /* temporary string for storing line, read from file */
	FILE *file_des; /* Current assembly file descriptor to process */
	machine_data *data_img[CODE_ARR_IMG_LENGTH]; /* Contains an image of the machine code */
	machine_word *codeword [CODE_ARR_IMG_LENGTH];
	/* We'll use multiple symbol tables, for each symbol type: .data, .code, .ext, .ent */
	table data_table, code_table, ext_table, ent_table;

	/* We get just the name, without the extension, so we have to add it (.as+'\0'): */
	input_filename = malloc_with_check(strlen(filename)+4);
	strcpy(input_filename, filename);
	strcat(input_filename, ".as"); /* add extension */

	file_des = fopen(input_filename, "r"); /* TODO: The filename, passed from command line args, shouldn't contain extension! it should be added here! */
	if (file_des == NULL) {
		/* if file couldn't be opened, write to stderr. */
		printf("Error: file \"%s\" is inaccessible for reading.", filename);
		return;
	}
	/* Put in static variables for error messaging */
	curr_filename = input_filename;
	/* Initialize  */
	data_table = code_table = ext_table = NULL;
	is_error = FALSE;
	ic = 100;
	dc = 0;
	/* File opened successfully, start first pass: */
	/* Read line - stop if read failed (when NULL returned) - usually when EOF. increase line counter for error printing. */
	for (curr_line = 1; fgets(temp_line, MAX_LINE_LENGTH, file_des) != NULL; curr_line++) {
		is_error = is_error || process_line_fpass(temp_line, &data_table, &code_table, &ext_table, &ic, &dc, codeword, data_img);
	}
	if (is_error) {
		printf("Stopped assembling the file %s. See the above output for more information.\n", filename);
		return;
	}
	/* Save ICF & DCF (1.18) */
	icf = ic;
	dcf = dc;
	/* Now let's add IC to each DC in data symbol table (step 1.19) */
	add_to_each_value(data_table, icf);

	/* First pass done right. start second pass: */
	rewind(file_des); /* Reread the file from the beginning */
	for (curr_line = 1; !feof(file_des); curr_line++)  {
		fgets(temp_line, MAX_LINE_LENGTH, file_des); /* Get line */ /* TODO: Implement */
		/*is_error = is_error || process_line_spass(temp_line, &ent_table, &code_table, &ic, ext_table, data_table);*/
	}
	if (is_error) {
		printf("Stopped assembling the file %s. See the above output for more information.\n", filename);
		return;
	}
	/* Everything was done. Write to *filename.ob/.ext/.ent */
}

int get_curr_line() {
	return curr_line;
}

char *get_curr_filename() {
	return curr_filename;
}