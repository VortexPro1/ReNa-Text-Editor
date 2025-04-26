#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* read_and_decrypt(const char *filename, long *out_size) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s' for reading.\n", filename);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
         fprintf(stderr, "Error: Could not seek to end of file '%s'.\n", filename);
         fclose(file);
         return NULL;
    }
    long file_size = ftell(file);
    rewind(file);

    if (file_size < 0) {
        fprintf(stderr, "Error: Could not determine size of file '%s' (ftell failed).\n", filename);
        fclose(file);
        return NULL;
    }
    if (file_size == 0) {
        fclose(file);
        char* empty_buffer = (char*)malloc(1);
        if (empty_buffer) {
            empty_buffer[0] = '\0';
            if(out_size) *out_size = 0;
            return empty_buffer;
        }
        fprintf(stderr, "Error: Could not allocate memory for empty file buffer.\n");
        return NULL;
    }

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate %ld bytes memory to read file '%s'.\n", file_size + 1, filename);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Error: Could not read entire file '%s'. Expected %ld bytes, got %zu.\n", filename, file_size, bytes_read);
        free(buffer);
        return NULL;
    }

    buffer[file_size] = '\0';

    char xor_key = 0x5A;
    for (long i = 0; i < file_size; i++) {
        buffer[i] ^= xor_key;
    }

    if (out_size) {
        *out_size = file_size;
    }

    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename.krl>\n", argv[0]);
        getchar();
        return 1;
    }

    const char *filename = argv[1];
    long decrypted_size = 0;
    char *decrypted_content = read_and_decrypt(filename, &decrypted_size);

    if (decrypted_content != NULL) {
        printf("Decrypted content of '%s' (%ld bytes):\n---\n", filename, decrypted_size);
        // Print the buffer content safely (handles potential null bytes if it wasn't text)
        // fwrite(decrypted_content, 1, decrypted_size, stdout);
        // Or if known to be null-terminated text:
        printf("%s\n", decrypted_content);
        printf("---\n");

        free(decrypted_content);

        printf("\nPress Enter to close...");
        getchar();

        return 0;
    } else {
        printf("\nFatal error\n");
        getchar();
        printf(":(\n");
        return 1;
    }
}