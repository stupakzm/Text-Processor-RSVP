//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <conio.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>


const int lineRSVP = 3;
const int lineSentence = 5;
const int lineStateInfo = 1;
int wordsPerFrame = 1;
int frame = 200000;// devide by 1000 for ms
int frameStep = 20000;
int automaticState = 0;
int paused = 0;
int previousSentenceBookmark = 0;

void clear_screen() {
    // \033[H: Moves the cursor to the home position first.
    // \033[2J: This escape sequence clears the entire console screen. It erases all the displayed characters.
    // \033[J: Clears the part of the screen from the cursor's current position to the end of the screen. Given the cursor has just been moved to the home position, this effectively clears the entire screen.
    //printf("\033[H\033[J");
    //printf("\033[2J\033[H");
    #ifdef _WIN32
    system("cls"); // For Windows
    #else
    system("clear"); // For Unix-like systems
    #endif

}

void set_cursor(int line, int column) {
    printf("\033[%d;%dH", line, column);
}

void print_states(){
    set_cursor(lineStateInfo,1);
    int frameVisual = frame / 1000;
    printf("Automatic [%d], Paused [%d], Words/Frame [%d], Frame [%d ms]", automaticState, paused, wordsPerFrame, frameVisual);
}

void print_error_and_exit(const char *message, FILE *file, void *buffer) {
    printf("%s\n", message);
    if (file) fclose(file);
    if (buffer) free(buffer);
    exit(EXIT_FAILURE);
}

int read_current_index(FILE *file) {
    char *line = NULL;
    size_t line_size = 256; // Initial size for the line buffer
    line = malloc(line_size);
    if (line == NULL) {
        printf("Memory allocation failed.");
        return -1; // Memory allocation failed, return -1
    }

    if (fgets(line, line_size, file) == NULL) {
        free(line);
        printf("Error reading file or file is empty");
        return -1; // Error reading file or file is empty, return -1
    }

    int index = -1;
    if (strncmp(line, "sentence_index_", 16) != 0) {
        index = atoi(line + 15); // Extract index
    }
    free(line);
    return index;
}


void write_new_index(FILE *file, int index) {
    if (!file) {
        printf("File pointer is NULL.\n");
        return; // Exit if file pointer is invalid
    }

    //rewind(file); // Go back to the start of the file

    if (fseek(file, 0L, SEEK_SET) != 0) {
    // Handle error, fseek returns non-zero on error
    perror("Failed to rewind the file");
    // Optionally, handle the error by closing the file, freeing resources, etc.
    }

    if (fprintf(file, "sentence_index_%d\n", index) < 0) {
        printf("Error writing to file.\n");
        return; // Check for writing errors
    }

    if (fflush(file) != 0) {
        printf("Error flushing file.\n");
        return; // Check for errors during flush
    }
}


void get_and_update_sentence_index(const char *filepath, int new_index) {
    FILE *file = fopen(filepath, "r+");
    if (!file) {
        print_error_and_exit("Error opening file!", NULL, NULL);
    }

    int current_index = read_current_index(file);
    if (current_index == -1) {
        print_error_and_exit("Error reading current index or file is empty", file, NULL);
    }

    if (new_index) {
        write_new_index(file, new_index);
    }


    fclose(file);
}

int get_sentence_index(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        // Handle error opening the file (e.g., print an error message)
        return;
    }

    return read_current_index(file);

    // Allocate memory for the first line with a reasonable initial size
    size_t line_size = 256;
    char *line = malloc(line_size);
    if (line == NULL) {
        fclose(file);
        printf("Error memory allocation!\n");
        // Handle memory allocation error (e.g., print an error message)
        return;
    }

    // Read the first line using fgets, considering potential buffer overflow
    if (fgets(line, line_size, file) == NULL) {
        free(line);
        fclose(file);
        printf("Error reading the line!\n");
        // Handle error reading the line (e.g., print an error message)
        return;
    }

    // Check if the line is longer than the allocated buffer
    size_t actual_length = strlen(line);
    if (actual_length == line_size - 1 && line[actual_length - 1] != '\n') {
        // Reallocate memory to fit the entire line, including newline
        line_size = actual_length + 2; // +1 for newline, +1 for null terminator
        char *temp = realloc(line, line_size);
        if (temp == NULL) {
            free(line);
            fclose(file);printf("Error memory reallocation !\n");
            // Handle memory reallocation error (e.g., print an error message)
            return;
        }
        line = temp;

        // Read the remaining characters until newline
        if (fgets(line + actual_length, line_size - actual_length, file) == NULL) {
            free(line);
            fclose(file);
            printf("Error reading remaining characters!\n");
            // Handle error reading remaining characters (e.g., print an error message)
            return;
        }
    }

    // Remove the trailing newline character (if present)
    size_t last_char = strlen(line) - 1;
    if (line[last_char] == '\n') {
        line[last_char] = '\0';
    }

    // Check if the line starts with "sentence_index_"
    if (strncmp(line, "sentence_index_", 16) == 0) {
        free(line);
        fclose(file);
        printf("Error case where line doesn't start with sentence_index_!\n");
        // Handle case where line doesn't start with "sentence_index_" (optional)
        return;
    }

    // Extract the index (assuming valid format)
    int index;
    char *endptr;
    index = strtol(line + 15, &endptr, 10); // Skip "sentence_index_"

    // Check for valid extraction (entire string converted to number)
    if (*endptr != '\0') {
        free(line);
        fclose(file);
        printf("Error invalid index format!\n");
        // Handle invalid index format (optional)
        return;
    }

    // Process the extracted index (print, store in a variable, etc.)
    printf("Extracted index: %d\n", index);

    fclose(file);
    free(line);
}

char **split(const char *str, const char *delims, int *count) {
    char *token;
    char *str_copy = strdup(str);// Make a copy of the string to preserve the original
    char **result = NULL;
    int spaces = 0;

    //printf("Splitting string: '%s'\nUsing delimiters: '%s'\n", str, delims);
    token = strtok(str_copy, delims);
    while (token) {
        result = realloc(result, sizeof(char *) * (spaces + 1));// Allocate memory for new token
        result[spaces] = strdup(token);// Duplicate the token
        //printf("Token %d: %s\n", spaces, token);
        spaces++;
        token = strtok(NULL, delims);// Continue to tokenize the string
    }
    *count = spaces;
    free(str_copy);// Clean up the duplicated string
    return result;
}

void display_words(char **words, int count) {
    int i, j, k, start_col = 5, end_col = 55, word_length, wordsPerFrameTemp;

    wordsPerFrameTemp = wordsPerFrame;

    paused = 0;
    for (j = 0; j < count; ) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == ' ') { // Check if the spacebar is pressed
                paused = !paused; // Toggle pause state
                print_states();
            }
        }

        while (paused) { // If paused, loop until unpaused
            set_cursor(lineRSVP, (start_col + end_col) / 2);
            if (_kbhit()) {
                int ch = _getch();
                switch (ch) {
                    case 27: // ESC key
                        clear_screen();
                        automaticState = 0;
                        paused = 0;
                        printf("Exited from (display_words), ESC to exit from (process_sentences).");
                        return; // return from this method if ESC is pressed

                    case 97: // ASCII for 'a'
                    case 65: // ASCII for 'A'
                        automaticState = !automaticState;
                        break;

                    case ' ': // Space bar
                        paused = !paused; // Toggle pause state
                        break;

                    case 224: // Prefix for arrow keys
                        ch = _getch(); // Get the second part of the keycode for arrows
                        switch (ch) {
                            case 72: // Up arrow key
                                if(frame < 500000){
                                    frame += frameStep;
                                }
                                break;

                            case 80: // Down arrow key
                                if(frame > 100000){
                                    frame -= frameStep;
                                }
                                break;
                        }
                    break;

                    case 49:
                    case 50:
                    case 51:
                        wordsPerFrame = ch - 48;
                        wordsPerFrameTemp = wordsPerFrame;
                        break;
                    default:
                        set_cursor(1,70);
                        printf("Unknown char.");
                        // Optional: Handle other characters that might be input
                    break;
                }
                print_states();
            }
            usleep(100000); // Reduce CPU usage while paused
        }

        if(count - j < wordsPerFrame){
            wordsPerFrameTemp = count - j;
        }

      switch (wordsPerFrameTemp) {
            case 1:
                word_length = strlen(words[j]);                break;
            case 2:
                word_length = strlen(words[j]) + strlen(words[j+1]) + 1;
                break;
            case 3:
                word_length = strlen(words[j]) + strlen(words[j+1]) + strlen(words[j+2]) + 2;
                break;
        }
        int midpoint = (start_col + end_col) / 2;
        int start_point = midpoint - (word_length / 2);
        set_cursor(lineRSVP, start_point);

        switch (wordsPerFrameTemp) {
            case 1:
                printf("%s", words[j]);
                free(words[j]);
                j++;
                break;
            case 2:
                printf("%s ", words[j]);
                printf("%s", words[j+1]);
                free(words[j]);
                free(words[j+1]);
                j = j+2;
                break;
            case 3:
                printf("%s ", words[j]);
                printf("%s ", words[j+1]);
                printf("%s", words[j+2]);
                free(words[j]);
                free(words[j+1]);
                free(words[j+2]);
                j = j +3;
                break;
        }

        set_cursor(lineRSVP,1);
        usleep(frame); // Sleep for frame/1000 ms

        //set_cursor(lineRSVP, start_point);
        for (k = 0; k < word_length + midpoint; k++) {
            printf(" ");
        }
        //free(words[j]);
    }
    free(words);
    usleep(frame*2);
}


void process_sentences(char *text, const char* filepath) {
    int sentences_count, words_count, i;
    char **sentences = split(text, ".", &sentences_count);

    char input;
     do {
        printf("Start from sentence %d [y/n]", previousSentenceBookmark);
        fflush(stdout);  // Flush stdout to ensure prompt is displayed before input

        // Get user input (case-insensitive)
        input = tolower(getch());

    } while (input != 'y' && input != 'n');

    if (input == 'y') {
        i = previousSentenceBookmark;
    } else {
        i = 0;
    }

    if(i<0 || previousSentenceBookmark > sentences_count){
        i = 0;
    }

    while (i < sentences_count) {
        clear_screen();
        set_cursor(lineSentence, 1);
        printf("%s", sentences[i]);

        char **words = split(sentences[i], " ,:;!\n?\r ", &words_count);
        print_states();
        display_words(words, words_count);

        if(automaticState == 1){
            if (i < sentences_count - 1){
                    i++;
                    continue;
            }
        }

        int ch = 0;
        while (!_kbhit()) {
            usleep(100000);  // Reduce CPU usage
        }
        ch = _getch();
        if (ch == 27) { // Check if the ESC key (ASCII 27) is pressed
            clear_screen();
            get_and_update_sentence_index(filepath, i);
            break; // Break the loop if ESC is pressed
        }
            if (ch == 224) { // Arrow keys are preceded by a 224 in the extended ASCII table
                ch = _getch();
                switch (ch) {
                    case 75: // Left arrow key
                        if (i > 0) i--;
                        break;
                    case 77: // Right arrow key
                        if (i < sentences_count - 1) i++;
                        break;
                }
            }

    }

    // Clean up sentences
    for (i = 0; i < sentences_count; i++) {
        free(sentences[i]);
    }
    free(sentences);
}

char* extractTextFromFileLastStable(const char* filepath) {
    FILE *file = fopen(filepath, "rb");  // Open in binary mode to avoid text translation issues
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize == -1) {
        perror("Failed to determine file size");
        fclose(file);
        return NULL;
    }
    rewind(file);

    char *text = malloc(fileSize + 1);
    if (text == NULL) {
        fclose(file);
        perror("Failed to allocate memory");
        return NULL;
    }

    size_t bytesRead = fread(text, 1, fileSize, file);
    if (bytesRead != fileSize) {
        if (feof(file)) {
            printf("Unexpected end of file.\n");
        }
        if (ferror(file)) {
            perror("Error reading file");
        }
        fclose(file);
        free(text);
        return NULL;
    }

    text[fileSize] = '\0';
    fclose(file);
    return text;
}

char* extractTextFromFile(const char* filepath) {
  FILE *file = fopen(filepath, "rb"); // Open in binary mode

  if (file == NULL) {
    perror("Failed to open file");
    return NULL;
  }

  // Detect and handle potential UTF-8 Byte Order Mark (BOM)
  int bom_size = 0;
  unsigned char bom[3];
  bom_size = fread(bom, 1, sizeof(bom), file);
  if (bom_size > 0) {
    // Check for UTF-8 BOM (EF BB BF)
    if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
      printf("Detected UTF-8 BOM. Skipping 3 bytes.\n");
      fseek(file, 3, SEEK_SET); // Skip the BOM
    } else {
      // Unknown BOM or data at the beginning, rewind
      rewind(file);
    }
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  if (fileSize == -1) {
    perror("Failed to determine file size");
    fclose(file);
    return NULL;
  }
  rewind(file);

  // Allocate memory for text (add space for null terminator)
  char *text = malloc(fileSize + 1);
  if (text == NULL) {
    fclose(file);
    perror("Failed to allocate memory");
    return NULL;
  }

  // Read the file content
  size_t bytesRead = fread(text, 1, fileSize, file);
  if (bytesRead != fileSize) {
    if (feof(file)) {
      printf("Unexpected end of file.\n");
    }
    if (ferror(file)) {
      perror("Error reading file");
    }
    fclose(file);
    free(text);
    return NULL;
  }

  // Null-terminate the string
  text[fileSize] = '\0';

  fclose(file);
  return text;
}


char* removeExtraNewlines(char* text) {
    if (text == NULL) return NULL;

    size_t len = strlen(text);
    char* cleanedText = malloc(len + 1); // Allocate memory for the cleaned text
    if (cleanedText == NULL) {
        perror("Failed to allocate memory for cleaned text");
        return NULL;
    }

    int i = 0, j = 0;
    while (text[i] != '\0') {
        if (text[i] == '\n' && (text[i + 1] == '\n' || text[i + 1] == '\r')) {
            // Skip consecutive newline characters
            i++;
        } else {
            cleanedText[j++] = text[i++];
        }
    }
    cleanedText[j] = '\0'; // Null-terminate the cleaned text

    return cleanedText;
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    char filepath[256];
    printf("Enter the file path: ");
    if (!fgets(filepath, sizeof(filepath), stdin)) {
        perror("Error reading filepath");
        return EXIT_FAILURE;
    }

    // Remove newline character
    filepath[strcspn(filepath, "\n")] = '\0';

    // Check if the input is enclosed in quotes
    size_t len = strlen(filepath);
    if (len > 0 && filepath[0] == '"' && filepath[len - 1] == '"') {
        filepath[len - 1] = '\0';  // Remove the trailing quote
        memmove(filepath, filepath + 1, len - 1);  // Shift the content to remove the leading quote
    }

    previousSentenceBookmark = get_sentence_index(filepath);

    char* text = extractTextFromFile(filepath);

     if (text) {
        char* cleanedText = removeExtraNewlines(text);
        if (cleanedText) {
            //printf("Text extracted from file:\n%s\n", cleanedText);
            process_sentences(cleanedText, filepath);
            free(cleanedText);
        } else {
            printf("Failed to clean text.\n");
        }
        free(text);
    } else {
        printf("Failed to extract text from file.\n");
    }

    printf("Press ENTER to exit...");
    getchar();  // Wait for key press

    return 0;
}
