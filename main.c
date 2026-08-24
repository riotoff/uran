#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#define VERSION "1.0"
#define MAX_UINT32 0xFFFFFFFF
#define URANDOM_PATH "/dev/urandom"

void print_binary(unsigned int n) {
    char buffer[33];
    buffer[32] = '\0';
    for (int i = 31; i >= 0; i--) {
        buffer[31 - i] = (n >> i) & 1 ? '1' : '0';
    }
    fputs(buffer, stdout);
}

unsigned int get_random_uint32(void) {
    unsigned int value;
    int fd = open(URANDOM_PATH, O_RDONLY);
    
    if (fd < 0) {
        fprintf(stderr, "Error: cannot open %s: %s\n", URANDOM_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    ssize_t ret = read(fd, &value, sizeof(value));
    if (ret != sizeof(value)) {
        if (ret < 0) {
            fprintf(stderr, "Error: read from %s failed: %s\n", URANDOM_PATH, strerror(errno));
        } else {
            fprintf(stderr, "Error: incomplete read from %s (got %zd bytes, expected %zu)\n",
                    URANDOM_PATH, ret, sizeof(value));
        }
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    return value;
}

unsigned int random_in_range(unsigned int min, unsigned int max) {
    uint64_t range = (uint64_t)max - (uint64_t)min + 1;
    uint64_t limit = (uint64_t)MAX_UINT32 + 1;
    
    if (range == 0) {
        fprintf(stderr, "Error: invalid range (overflow detected)\n");
        exit(EXIT_FAILURE);
    }
    
    if (range == limit) {
        return get_random_uint32();
    }
    
    uint64_t threshold = limit - (limit % range);
    unsigned int value;
    
    do {
        value = get_random_uint32();
    } while ((uint64_t)value >= threshold);
    
    return min + (value % range);
}

unsigned int parse_uint(const char *str, const char *name) {
    if (!str || !*str) {
        fprintf(stderr, "Error: %s argument is empty\n", name);
        exit(EXIT_FAILURE);
    }
    
    char *endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, 10);
    
    if (errno == ERANGE || val > UINT32_MAX) {
        fprintf(stderr, "Error: %s value out of range (0-%u)\n", name, UINT32_MAX);
        exit(EXIT_FAILURE);
    }
    if (endptr == str || *endptr != '\0') {
        fprintf(stderr, "Error: %s argument is not a valid number: %s\n", name, str);
        exit(EXIT_FAILURE);
    }
    
    return (unsigned int)val;
}

int parse_count(const char *str) {
    if (!str || !*str) {
        fprintf(stderr, "Error: count argument is empty\n");
        exit(EXIT_FAILURE);
    }
    
    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);
    
    if (errno == ERANGE || val <= 0 || val > INT_MAX) {
        fprintf(stderr, "Error: count must be between 1 and %d\n", INT_MAX);
        exit(EXIT_FAILURE);
    }
    if (endptr == str || *endptr != '\0') {
        fprintf(stderr, "Error: count argument is not a valid number: %s\n", str);
        exit(EXIT_FAILURE);
    }
    
    return (int)val;
}

void print_help(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Generate random numbers using %s\n\n", URANDOM_PATH);
    printf("Options:\n");
    printf("  -n, --count=N     Generate N numbers (default: 1)\n");
    printf("  -m, --min=N       Minimum value (default: 0)\n");
    printf("  -M, --max=N       Maximum value (default: UINT32_MAX)\n");
    printf("  -x, --hex         Output in hexadecimal\n");
    printf("  -o, --octal       Output in octal\n");
    printf("  -b, --binary      Output in binary\n");
    printf("  -s, --separator   Separator between numbers (default: newline)\n");
    printf("  --help            Show this help\n");
    printf("  --version         Show version information\n");
}

int main(int argc, char **argv) {
    int count = 1;
    unsigned int min_val = 0;
    unsigned int max_val = MAX_UINT32;
    int hex_output = 0;
    int octal_output = 0;
    int binary_output = 0;
    const char *separator = "\n";
    int opt;

    struct option long_options[] = {
        {"count",     required_argument, 0, 'n'},
        {"min",       required_argument, 0, 'm'},
        {"max",       required_argument, 0, 'M'},
        {"hex",       no_argument,       0, 'x'},
        {"octal",     no_argument,       0, 'o'},
        {"binary",    no_argument,       0, 'b'},
        {"separator", required_argument, 0, 's'},
        {"help",      no_argument,       0, 'h'},
        {"version",   no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "n:m:M:xobs:hv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'n':
                count = parse_count(optarg);
                break;
            case 'm':
                min_val = parse_uint(optarg, "min");
                break;
            case 'M':
                max_val = parse_uint(optarg, "max");
                break;
            case 'x':
                hex_output = 1;
                break;
            case 'o':
                octal_output = 1;
                break;
            case 'b':
                binary_output = 1;
                break;
            case 's':
                separator = optarg ? optarg : "\n";
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            case 'v':
                printf("uran %s\n", VERSION);
                printf("License: GPL-3.0\n");
                return 0;
            default:
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return 1;
        }
    }

    if (min_val > max_val) {
        fprintf(stderr, "Error: min value (%u) cannot be greater than max value (%u)\n", 
                min_val, max_val);
        return 1;
    }

    if (count == 0) {
        return 0;
    }

    for (int i = 0; i < count; i++) {
        unsigned int num = random_in_range(min_val, max_val);

        if (hex_output) {
            printf("%x", num);
        } else if (octal_output) {
            printf("%o", num);
        } else if (binary_output) {
            print_binary(num);
        } else {
            printf("%u", num);
        }

        if (i < count - 1) {
            fputs(separator, stdout);
        }
    }
    putchar('\n');

    return 0;
}
