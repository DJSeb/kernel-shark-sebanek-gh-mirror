#include <stdio.h>
#include <unistd.h>
#include <time.h>

void recursive_function(int depth) {
    if (depth > 0) {
        char msg[] = "Kernel stack baggage!\n";
        // Write syscall (syscall number 1) to stdout (file descriptor 1)
        write(1, msg, sizeof(msg) - 1); // sizeof(msg) - 1 to exclude the null terminator

        if (depth == 5) { // Do something special at depth 5
            char special_msg[] = "You are at depth 5.\n";
            write(1, special_msg, sizeof(special_msg) - 1);
        }

        // Use nanosleep to sleep for 1 second
        struct timespec req;
        req.tv_sec = (depth / 2);  // seconds
        req.tv_nsec = 1; // nanoseconds
        nanosleep(&req, NULL);

        recursive_function(depth - 1);
    }
}

int main() {
    // Start the recursive function with a certain depth
    recursive_function(10);
    return 0;
}