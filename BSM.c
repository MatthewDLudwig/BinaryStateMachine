#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "termios.h"
#include "fcntl.h"
#include "errno.h"
#include "signal.h"
#include "unistd.h"
const char NEXTSTATE = 0;
const char STAYSTATE = 1;
const char ROOTSTATE = 2;

typedef struct BinaryStateLink {
        int (*decision)(char);

        void (*left)(void *, char);
        char leftMask;
        void (*right)(void *, char);
        char rightMask;

        struct BinaryStateLink * next;
} BSL;

typedef struct BinaryStateMachine {
        BSL * root;
        BSL * current;
        void * data;
} BSM;

BSM * makeMachine(void * d) {
        BSM * machine = malloc(sizeof(BSM));
        machine->root = NULL;
        machine->current = NULL;
        machine->data = d;
        return machine;
}

void addState(BSM * machine, int (*d)(char), void (*l)(void *, char), char lm, void (*r)(void *, char), char rm) {
        BSL * link = malloc(sizeof(BSL));
        link->decision = d;

        link->left = l;
        link->leftMask = lm;
        link->right = r;
        link->rightMask = rm;

        link->next = NULL;

        BSL * walker = machine->root;

        if (walker == NULL) {
                machine->root = link;
        } else {
                while (walker->next != NULL) {
                        walker = walker->next;
                }

                walker->next = link;
        }
}

void restart(BSM * machine) {
        machine->current = machine->root;
}

void decide(BSM * machine, char c) {
        if (machine->current == NULL) {
                fprintf(stderr, "STATE MACHINE NOT STARTED YET!");
                return;
        }

        char mask;

        if (machine->current->decision(c) == 0) { //Left
                machine->current->left(machine->data, c);
                mask = machine->current->leftMask;
        } else { //Right
                machine->current->right(machine->data, c);
                mask = machine->current->rightMask;
        }

        if (mask == NEXTSTATE) {
                machine->current = machine->current->next;
        } else if (mask == STAYSTATE) {
                machine->current = machine->current;
        } else if (mask == ROOTSTATE) {
                machine->current = machine->root;
        } else {
                fprintf(stderr, "UNDEFINED MASK!");
        }
}
#include "stdio.h"

int always_right(char c) {
        return 1;
}

int always_left(char c) {
        return 0;
}

int match_escape(char c) {
        return c == 27;
}

int match_bracket(char c) {
        return c == '[';
}

int match_arrows(char c) {
        if (c == 'A') {
                fprintf(stdout, "Up Arrow");
        } else if (c == 'B') {
                fprintf(stdout, "Down Arrow");
        } else if (c == 'C') {
                fprintf(stdout, "Right Arrow");
        } else if (c == 'D') {
                fprintf(stdout, "Left Arrow");
        } else {
                return 0;
        }

        return 1;
}

void doNothing(void * mem, char c) {
        //I do nothing!
}

void printChar(void * mem, char c) {
        if (c<' ' || c>'~' || c=='[' || c==']' || c=='"') {
                fprintf(stdout, "[%d]", c);
        } else {
                fprintf(stdout, "%c", c);
        }
}

typedef struct MachineMemory {
        char * line;
        int appendPos;
} MachineMemory;

BSM * createMachine() {
        MachineMemory * mem = malloc(sizeof(MachineMemory));
        mem->line = malloc(sizeof(char) * 100);
        mem->appendPos = 0;

        BSM * machine = makeMachine(mem);
        addState(machine, match_escape, printChar, ROOTSTATE, doNothing, NEXTSTATE);
        addState(machine, match_bracket, printChar, ROOTSTATE, doNothing, NEXTSTATE);
        addState(machine, match_arrows, doNothing, ROOTSTATE, doNothing, ROOTSTATE);
        restart(machine);
        return machine;
}

struct termios oldSettings;

void reset_terminal() {
        tcsetattr(0, TCSANOW, &oldSettings);
}

int main() {
        tcgetattr(0, &oldSettings);
        atexit(reset_terminal);

        struct termios settings;
        tcgetattr(0, &settings);
        settings.c_lflag &= ~ ECHO;
        settings.c_lflag &= ~ ICANON;
        settings.c_cc[VMIN] = 1;
        settings.c_cc[VTIME] = 0;
        tcsetattr(0, TCSAFLUSH, &settings);

        BSM * machine = createMachine();

        while (1) {
                char line[1000];
                int r = read(0, line, 999);
                if (r < 0) {
                        if (errno == EAGAIN) {
                                usleep(1000);
                                continue;
                        }

                        fprintf(stderr, "errno is %d\n", errno);
                        perror("read error: ");
                } else if (r == 0) {
                        //Nothing read.
                } else {
                        line[r] = 0; //Gives the null terminating character at the end.

                        for (int i = 0; i < strlen(line); ++i) {
                                decide(machine, line[i]);
                        }

                        fprintf(stdout, "\n");
                }
        }

        return 0;
}
