#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char buffer[100];
    int buffer_index;
    char current_filename[105];
} EditorState;

void rena_handle_char(EditorState *state, char c);
void rena_save_buffer(const EditorState *state, const char *filename);
void rena_prompt_save(EditorState *state);
void rena_exit(const EditorState *state);

typedef struct {
    void (*handle_char)(EditorState *state, char c);
    void (*prompt_save)(EditorState *state);
    void (*exit)(const EditorState *state);
} EditorInterface;

void save_to_file(const char *buffer, const char *filename) {
    char xor_key = 0x5A;
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s' for writing.\n", filename);
        return;
    }
    size_t len = strlen(buffer);
    for (size_t i = 0; i < len; ++i) {
        char encrypted_char = buffer[i] ^ xor_key;
        if (fwrite(&encrypted_char, 1, 1, file) != 1) {
            fprintf(stderr, "Error: Could not write to file '%s'.\n", filename);
            fclose(file);
            return;
        }
    }
    fclose(file);
    printf("Saved as file <%s> to disk.\n", filename);
}

void rena_handle_char(EditorState *state, char c) {
    if (state->buffer_index < sizeof(state->buffer) - 1) {
       state->buffer[state->buffer_index++] = c;
    } else {
       printf("Warning: Buffer full. Character '%c' ignored.\n", c);
    }
}

void rena_prompt_save(EditorState *state) {
    state->buffer[state->buffer_index] = '\0';

    printf("Name your file (current: %s): ", state->current_filename[0] ? state->current_filename : "none");
    char input_filename[105];
    if (fgets(input_filename, sizeof(input_filename), stdin) != NULL) {
        input_filename[strcspn(input_filename, "\n")] = 0;

        char *filename_to_use = NULL;
        if (strlen(input_filename) > 0) {
            // User entered a new name
            filename_to_use = input_filename;
            strncpy(state->current_filename, input_filename, sizeof(state->current_filename) - 1);
            state->current_filename[sizeof(state->current_filename) - 1] = '\0';
        } else if (state->current_filename[0] != '\0') {
            filename_to_use = state->current_filename;
            printf("Using current filename: %s\n", filename_to_use);
        } else {
             printf("Save cancelled (no filename provided).\n");
             return;
        }

        size_t len = strlen(filename_to_use);
        const char *extension = ".krl";
        size_t ext_len = strlen(extension);

        if (len > 0) {
             if (len < ext_len || strcmp(filename_to_use + len - ext_len, extension) != 0) {
                 if (len + ext_len < 105) {
                     strcat(filename_to_use, extension);
                     if (filename_to_use == input_filename) {
                        strncpy(state->current_filename, input_filename, sizeof(state->current_filename) - 1);
                        state->current_filename[sizeof(state->current_filename) - 1] = '\0';
                     }
                 } else {
                     printf("Filename too long to add extension.\n");
                     return;
                 }
             }
             save_to_file(state->buffer, filename_to_use);
        } else {
             printf("Save cancelled (empty filename).\n");
        }

    } else {
        printf("Error reading filename.\n");
    }
    state->buffer_index = 0;
    state->buffer[0] = '\0';
}

void rena_exit(const EditorState *state) {
    printf("Exiting ReNa editor.\n");
}


int main() {
    EditorState current_state = {0};
    current_state.current_filename[0] = '\0';

    EditorInterface rena_interface = {
        .handle_char = rena_handle_char,
        .prompt_save = rena_prompt_save,
        .exit = rena_exit
    };

    printf("ReNa - A simple text editor\n");
    printf("Type text. Press Enter to save, Esc to exit.\n");
    printf("> ");

    while (1) {
        char c = getchar();

        if (c == '\n') {
            rena_interface.prompt_save(&current_state);
            printf("> ");
        } else if (c == '\033') {
            rena_interface.exit(&current_state);
            break;
        } else if (c != EOF) {
             // Only add printable characters or handle others specifically if needed
            // if (isprint(c) || c == '\t' /* allow tabs? */) {
                 rena_interface.handle_char(&current_state, c);
            // }
        } else {
             fprintf(stderr, "\nInput error or EOF reached.\n");
             rena_interface.exit(&current_state);
             break;
        }
    }
    return 0;
}