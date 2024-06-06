#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <assert.h>
#include <termios.h>
#include <fcntl.h>

#define PORT 8457

// Function to set the terminal to non-canonical mode
void setNonCanonicalMode(struct termios *original) {
    struct termios newSettings;

    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, original);

    // Copy the settings to modify them
    newSettings = *original;

    // Disable canonical mode and echo
    newSettings.c_lflag &= ~(ICANON | ECHO);

    // Set the new terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
}

// Function to reset the terminal to the original settings
void resetTerminalMode(struct termios *original) {
    // Restore the original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, original);
}

// Function to check if a key has been pressed
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Set the file status flags to non-blocking
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // Restore the original settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        //ungetc(ch, stdin);
        return ch;
    }

    return 0;
}

int main() {
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt Failure");
        exit(1);
    }

    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &sockAddr.sin_addr)
        <= 0) {
        perror("Address Failure");
        exit(1);
    }

    // Connect the socket to server
    if (connect(sockfd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0) {
        perror("Connection Failure");
        exit(1);
    }

    struct termios originalSettings;

    // Set the terminal to non-canonical mode
    setNonCanonicalMode(&originalSettings);

    while(1) {
        char key = kbhit();
        if(key) {
            if (key == 9 || key == 10 || (key >= 32 && key <= 127)) {
                printf("Key pressed: %c (%d)\n", key, key);
                send(sockfd, &key, 1, 0);
                //break;
            }
            
        }
    }

    // Reset the terminal to the original settings
    resetTerminalMode(&originalSettings);
}