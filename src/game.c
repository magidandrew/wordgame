#include "globals.h"
#include "mylist.h"
#include "word_data.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 48 //longest word in allnouns.txt
#define NUM_WORDS 90963    //based off allnouns.txt

int letterValues[26] = {LETTER_A, LETTER_B, LETTER_C, LETTER_D, LETTER_E, LETTER_F, LETTER_G, LETTER_H, LETTER_I,
                        LETTER_J, LETTER_K, LETTER_L, LETTER_M, LETTER_N, LETTER_O, LETTER_P, LETTER_Q, LETTER_R,
                        LETTER_S, LETTER_T, LETTER_U, LETTER_V, LETTER_W, LETTER_X, LETTER_Y, LETTER_Z};

void printNode(void *data)
{
    printf("entry: %s\n", (char *)data);
}

void freeUsedWords(struct List *list)
{
    while (!isEmptyList(list)) {
        free(popFront(list));
    }
}

void sanitize(char *word)
{
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        word[i] = tolower(word[i]);
        if (isspace(word[i]) != 0) {
            word[i] = '\0';
        }
    }
}

int compareStringNodeData(const void *word1, const void *word2)
{
    return strcmp((char *)word1, (char *)word2);
}

int containsWord(struct List *list, char **buf, char *word)
{
//    for (int i = 0; i < NUM_WORDS; i++) {
//        //-2 to avoid \n after fgets
//        if (strcmp(buf[i], word) == 0) {
//            return 1;
//        }
//    }

    struct Node *curNode = list->head;
    while(curNode != NULL){
        if(strcmp((char *) curNode->data, word) == 0){
            return 1;
        }
        else{
            curNode = curNode->next;
        }
    }

    return 0;
}

void modifyPlayerScore(int *scorearray, int playerfd, int delta)
{
    scorearray[playerfd] += delta;
}

int wordToScore(char *word)
{
    int score = 0;
    int len = strlen(word);
    for(int i=0; i<len; i++){
        //convert ascii to 0-25 and lookup that letter value if it's a letter
        if(isalpha(word[i])) {
            score += letterValues[(int) (word[i] - 97)];
        }
    }
    return score;
}



int main(int argc, char **argv)
{
//    FILE *dictionary = fopen("../allnouns.txt", "r"); //no need for this now because hardcoding all strings inside file
//    FILE *dictionary = fopen(argv[1], "r"); //might be a security vulnerability letting '..'

    // char buf[NUM_WORDS][MAX_WORD_LENGTH];
    // char **buf = (char **)malloc(sizeof(char) * MAX_WORD_LENGTH * NUM_WORDS);
//    char **arr = (char **)calloc(NUM_WORDS, sizeof(char *));
//
//    unsigned int index = 0;
//    char buf[MAX_WORD_LENGTH];
//    while (fgets(buf, MAX_WORD_LENGTH, dictionary) != NULL) {
//        arr[index] = (char *)calloc(strlen(buf) - 1, sizeof(char));
//        buf[strlen(buf) - 1] = '\0';
//        strncpy(arr[index++], buf, strlen(buf) - 1);
//    }

    //populate dictionary
    struct List allwords;
    initList(&allwords);
    char dictionaryBuf[100];
//    while (fgets(dictionaryBuf, sizeof(dictionaryBuf), dictionary) != NULL){
//        sanitize(dictionaryBuf);
//        char *word = (char *) malloc(sizeof(char) * strlen(dictionaryBuf) + 1);
//        strcpy(word, dictionaryBuf);
//        //note: addBack too damn expensive
//        addFront(&allwords, (void *) word);
//    }
    int counter = 0;
    while(all_words[counter] != NULL){
        strcpy(dictionaryBuf, all_words[counter]);
        sanitize(dictionaryBuf);
        char *word = (char *) malloc(sizeof(char) * strlen(dictionaryBuf) + 1);
        strcpy(word, dictionaryBuf);
        addFront(&allwords, (void *) word);
        counter++;
    }

    //SCORE KEEPING
    //array of all the scores
    int scores[MAX_CLIENTS];
    int totalscore = 0;

    struct List usedwords;
    initList(&usedwords);

    char userinput[100];
    char newword[100];
    printf("enter word: ");
    fgets(userinput, sizeof(userinput), stdin);
    sanitize(userinput);
    struct Node *usedWordNode;

    while (1) {
        printf("Find a noun that starts with the letter \'%c\': ", userinput[strlen(userinput) - 1]);
        //get input from user
        fgets(newword, sizeof(newword), stdin);
        sanitize(newword);

        //check 3 conditions: 1. last letter of old word is first letter of new word
        if (newword[0] != userinput[strlen(userinput) - 1]) {
            printf("The first letter of your word must must be the last letter of the previous word.\n");
            continue;
        }
        //2. word hasn't been used before
        else if ((usedWordNode = findNode(&usedwords, (void *)newword, &compareStringNodeData)) != NULL) {
            printf("We've used \"%s\" before! Pick a new word.\n", (char *)usedWordNode->data);
            continue;
        }//    while (fgets(buf, MAX_WORD_LENGTH, dictionary) != NULL) {

            //3. the word is a real word checked against a dictionary
//        else if (containsWord(arr, newword) != 1) {
        else if ((usedWordNode = findNode(&allwords, (void *) newword, &compareStringNodeData)) == NULL) {
            printf("\"%s\" isn't a real noun!\n", newword);
            continue;
        }
        else {
            //clear the userinput buffer
            memset(userinput, 0, sizeof(userinput));
            strcpy(userinput, newword);

//            //malloc memory for the new word in the linked list and +1 for null terminator
//            char *wordtosave = (char *) malloc(sizeof(char) * (strlen(newword) + 1));
//            strcpy(wordtosave, newword);
//            if (addFront(&usedwords, (void *)wordtosave) == NULL) {
//                perror("ADDFRONT FAILED!");
//                exit(1);
//            }

            //we can store pointer to the data instead
            addFront(&usedwords, (void *) usedWordNode->data);
            int scoreDelta = wordToScore((char *) usedWordNode->data);
            printf("word score: %d\n", scoreDelta);
            totalscore += scoreDelta;
            printf("total score: %d\n", totalscore);
        }
    }

    // free(buf);
//    for (int i = 0; i < NUM_WORDS; i++) {
//        free(arr[i]);
//    }
//    free(arr);

    //free used words
    removeAllNodes(&usedwords);
    freeUsedWords(&allwords);

    //close file descriptor
    fclose(dictionary);
    return 0;
}
