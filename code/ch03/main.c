#include <stdio.h>

int flip_coin(void)
{
    /* Not random yet -- always lands on heads. We fix this for real in
       Chapter 15, once you know how to generate random numbers. */
    return 0;   /* 0 = heads, 1 = tails */
}

char ask_call(void)
{
    char call = '\0';

    while (call != 'h' && call != 't') {
        printf("Call it: (h)eads or (t)ails? ");

        int typed = getchar();

        switch (typed) {
            case 'h':
            case 'H':
                call = 'h';
                break;
            case 't':
            case 'T':
                call = 't';
                break;
            default:
                break;
        }

        /* Discard the rest of the line, including the newline, so the
           next trip through the loop starts clean. */
        while (typed != '\n' && typed != EOF) {
            typed = getchar();
        }

        if (call != 'h' && call != 't') {
            printf("Didn't catch that -- type h or t.\n");
        }
    }

    return call;
}

void run_duel(void)
{
    printf("\n");
    printf("A stranger flips an old coin and grins.\n");

    char call = ask_call();

    printf("The coin spins");
    for (int i = 0; i < 3; i++) {
        printf(".");
    }
    printf("\n");

    int result = flip_coin();
    char landed = 'h';
    if (result == 1) {
        landed = 't';
    }

    if (landed == 'h') {
        printf("It lands on heads. ");
    } else {
        printf("It lands on tails. ");
    }

    if (call == landed) {
        printf("You called it! You win the duel.\n");
    } else {
        printf("Not what you called. You lose the duel.\n");
    }
}

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*          UNTITLED JRPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
    printf("\n");

    char name[64];
    printf("What is your name, hero? ");
    fgets(name, sizeof name, stdin);

    int hp = 20;
    int mp = 8;
    int attack = 6;
    int defense = 4;
    char rank = 'E';
    float power_rating = (float)(attack + defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", name);
    printf("----------------------------------------\n");
    printf("HP:     %d\n", hp);
    printf("MP:     %d\n", mp);
    printf("ATK:    %d\n", attack);
    printf("DEF:    %d\n", defense);
    printf("Rank:   %c\n", rank);
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");

    run_duel();

    return 0;
}
