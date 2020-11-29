#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


// Helper function to fill up keyCharacters buffer
void keyCharactersSetup(char *buff)
{
  int i = 0;
  int keyChar = 65;
  // Inputs letters A-Z
  for(i; i < 26; i++)
  {
    buff[i] = keyChar;
    keyChar++;
  }
  // fill last character as SPACE char
  buff[i] = 32;
}

int main(int argc, char *argv[])
{
  char *key;
  char keyCharacters[27];
  if(argc < 2)
  {
    printf("Please input a keylength next time\n");
    return 0;
  }
  else
  {
    keyCharactersSetup(keyCharacters);
    int keyLength = atoi(argv[1]);
    key = calloc(keyLength+1, 1);
    srand(time(0));
    int i = 0;
    for(i; i < keyLength; i++)
    {
      // Select a random index from keyCharacters buff
      int randIdx = (rand() % 27);
      // Add a random letter to the key
      key[i] = keyCharacters[randIdx];
    }
    // Add new line to last character
    key[i] = '\n';
    printf("%s", key);
    free(key);
    key = NULL;
  }
  return 0;
}