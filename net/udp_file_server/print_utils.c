#include "print_utils.h"

// Function to find the number of digits in an integer
int num_digits(int num) {
    if (num == 0) return 1;
    int count = 0;
    while (num != 0) {
        count++;
        num /= 10;
    }
    return count;
}

// Function to print a row separator line
void print_separator(int total_cols, int column_widths[]) {
    for (int i = 0; i < total_cols; i++) {
        printf("+");
        for (int j = 0; j < column_widths[i]; j++) {
            printf("-");
        }
    }
    printf("+\n");
}

// Function to calculate column widths automatically
void calculate_column_widths(int string_cols, int numeric_cols, int rows, char** string_data, int* integer_data, char** headers, int column_widths[]) {
    int total_cols = 1 + string_cols + numeric_cols; // +1 for the ID column

    // Set the width for the ID column
    column_widths[0] = num_digits(rows - 1) + 2; // ID column should be wide enough to hold the largest index

    // Initialize column widths based on header lengths
    for (int col = 1; col <= string_cols; col++) {
        column_widths[col] = strlen(headers[col - 1]) + 2;  // Base width is the length of the string column headers + padding
    }
    for (int col = string_cols + 1; col < total_cols; col++) {
        column_widths[col] = strlen(headers[col - 1]) + 2;  // Base width is the length of the numeric column headers + padding
    }

    // Adjust column widths based on the data
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < string_cols; col++) {
            // String column
            int str_width = strlen(string_data[row * string_cols + col]) + 2;  // Add padding
            if (str_width > column_widths[col + 1]) {
                column_widths[col + 1] = str_width;
            }
        }

        for (int col = 0; col < numeric_cols; col++) {
            // Numeric column
            int num_width = num_digits(integer_data[row * numeric_cols + col]) + 2;  // Add padding
            if (num_width > column_widths[string_cols + col + 1]) {
                column_widths[string_cols + col + 1] = num_width;
            }
        }
    }
}

// Function to print the table
void print_table(char** string_data, int* integer_data, int rows, int string_cols, int numeric_cols, char** headers) {
    int total_cols = 1 + string_cols + numeric_cols; // +1 for the ID column
    int column_widths[total_cols];

    // Calculate column widths automatically
    calculate_column_widths(string_cols, numeric_cols, rows, string_data, integer_data, headers, column_widths);

    // Print the header row (add "ID" column)
    print_separator(total_cols, column_widths);
    printf("| %-*s ", column_widths[0] - 1, "ID"); // ID column
    for (int i = 1; i <= string_cols; i++) {
        printf("| %-*s ", column_widths[i] - 1, headers[i - 1]);
    }
    for (int i = string_cols + 1; i < total_cols; i++) {
        printf("| %-*s ", column_widths[i] - 1, headers[i - 1]);
    }
    printf("|\n");
    print_separator(total_cols, column_widths);

    // Print the data rows (add the auto-incrementing ID)
    for (int row = 0; row < rows; row++) {
        printf("| %*d ", column_widths[0] - 1, row); // Print the ID
        for (int col = 0; col < string_cols; col++) {
            // Print string columns first
            printf("| %-*s ", column_widths[col + 1] - 1, string_data[row * string_cols + col]); // Left-aligned string
        }
        for (int col = 0; col < numeric_cols; col++) {
            // Print numeric columns after string columns
            printf("| %*d ", column_widths[string_cols + col + 1] - 1, integer_data[row * numeric_cols + col]); // Right-aligned integer
        }
        printf("|\n");
    }

    // Print footer separator
    print_separator(total_cols, column_widths);
}

void print_files(dir_files files){
  char *headers[] = {"Name", "MD5", "Size"};

  int rows = files.filecounts;
  int numeric_cols = 1; 
  int string_cols = 2;  
  int* filesize = malloc(files.filecounts * sizeof(int));

  char** string_data = (char**) malloc(files.filecounts * 2 * sizeof(char*));
  for(int i = 0; i<files.filecounts; i++){
	  fileinfo file = files.files[i];
	  string_data[2 * i] = file.name;
	  string_data[2 * i + 1 ] = file.hash;

	  filesize[i] = file.size;
  }
	  

  print_table(string_data, filesize, rows, string_cols, numeric_cols,
              headers);

  free(string_data);
  free(filesize);


}
char* printable = "!@#$%^&*()+= `<>[]{}/\\-_'\":";
void try_print(unsigned char* buffer, int size){
	for(int i = 0; i<size; i++){
		char c = '?';
		if (buffer[i] >= 'a' && buffer[i] <= 'z'){
			c = buffer[i];
		} else if (buffer[i] >= 'A' && buffer[i] <= 'Z'){
			c = buffer[i];
		} else if (buffer[i] >= '0' && buffer[i] <= '9'){
			c = buffer[i];
		} else if (strchr(printable, buffer[i])){
			c = buffer[i];
		}

		printf("%c", c);
	}
	printf("\n");
}

