#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    unsigned int random_number;
    int fd = open("/dev/urandom", O_RDONLY);
    
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    
    if (read(fd, &random_number, sizeof(random_number)) != sizeof(random_number))
    {
        perror("read");
        close(fd);
        return 1;
    }
    
    close(fd);
    printf("%u\n", random_number);
    
    return 0;
}
