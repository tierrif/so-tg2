#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEFAULT_ENERGY 5000
#define DEFAULT_CELL 0
#define AMOUNT_OF_CELLS 49
#define AMOUNT_OF_OBJECTS 10
#define MAP_WIDTH (int) floor(sqrt(AMOUNT_OF_CELLS))

struct Cell {
  int north,
      south,
      east,
      west,
      up,
      down,
      hasObject,
      hasTreasure,
      id;
  char* description;
};

struct Player {
  char* name;
  int energy;
  struct Cell* cell;
  int carriedObject; // -1 se não tem, senão código do objeto.
  int carriesTreasure;
};

struct Monster {
  int energy;
  struct Cell* cell;
};

struct Object {
  char* name;
  int energy;
  int id;
};

char* promptString(char* prompt) {
  char* buff = malloc(strlen(prompt) + 1);
  strcpy(buff, prompt);
  strcat(buff, " ");

  char* name = malloc(100);
  printf(buff);
  scanf("%s", name);
  return name;
}

void initializeCells(struct Cell** map, struct Cell* cells) {
  int ids = 0;
  for (int i = 0; i < MAP_WIDTH; i++) {
    for (int j = 0; j < MAP_WIDTH; j++) {
      struct Cell* cell = &map[i][j];
      cell->hasObject = 0;
      cell->hasTreasure = 0;
      cell->id = ids++;
      cell->description = malloc(100);
      sprintf(cell->description, "Cell %d", cell->id);
    }
  }

  ids = 0;
  for (int i = 0; i < MAP_WIDTH; i++) {
    for (int j = 0; j < MAP_WIDTH; j++) {
      struct Cell* cell = &map[i][j];

      if (i == 0) {
        cell->north = -1;
      } else {
        cell->north = map[i - 1][j].id;
      }

      if (i == MAP_WIDTH - 1) {
        cell->south = -1;
      } else {
        cell->south = map[i + 1][j].id;
      }

      if (j == 0) {
        cell->west = -1;
      } else {
        cell->west = map[i][j - 1].id;
      }

      if (j == MAP_WIDTH - 1) {
        cell->east = -1;
      } else {
        cell->east = map[i][j + 1].id;
      }

      cell->up = -1;
      cell->down = -1;

      cells[ids++] = map[i][j];
    }
  }

  int treasureCell = rand() % AMOUNT_OF_CELLS;
  cells[treasureCell].hasTreasure = 1;
}

struct Object* initializeObjects(struct Cell* cells) {
  struct Object* objects = malloc(sizeof(struct Object) * AMOUNT_OF_OBJECTS);
  for (int i = 0; i < AMOUNT_OF_OBJECTS; i++) {
    objects[i].name = malloc(100);
    sprintf(objects[i].name, "Object %d", i);
    objects[i].energy = rand() % 100;
    objects[i].id = i;

    int index = rand() % AMOUNT_OF_CELLS;
    while (cells[index].hasObject || cells[index].hasTreasure) {
      index = rand() % AMOUNT_OF_CELLS;
    }
    cells[index].hasObject = 1;
  }

  return objects;
}

void moveMonster(struct Monster* monster, struct Cell* cells) {
  int direction = rand() % 6;
  while (direction == 0 && monster->cell->north == -1 ||
         direction == 1 && monster->cell->south == -1 ||
         direction == 2 && monster->cell->east == -1 ||
         direction == 3 && monster->cell->west == -1 ||
         direction == 4 && monster->cell->up == -1 ||
         direction == 5 && monster->cell->down == -1) {
    direction = rand() % 6;
  }
  switch (direction) {
    case 0:
      monster->cell = &cells[monster->cell->north];
      break;
    case 1:
      monster->cell = &cells[monster->cell->south];
      break;
    case 2:
      monster->cell = &cells[monster->cell->east];
      break;
    case 3:
      monster->cell = &cells[monster->cell->west];
      break;
    case 4:
      monster->cell = &cells[monster->cell->up];
      break;
    case 5:
      monster->cell = &cells[monster->cell->down];
      break;
  }
}

struct Object randomizeObject(struct Object* objects, int* foundObjects) {
  int index = rand() % AMOUNT_OF_OBJECTS;
  struct Object obj = objects[index];

  foundObjects[index] = 1;

  return obj;
}

int main() {
  struct Cell** map = (struct Cell**) malloc(sizeof(struct Cell*) * MAP_WIDTH);
  for (int i = 0; i < MAP_WIDTH; i++) {
    map[i] = (struct Cell*) malloc(sizeof(struct Cell) * MAP_WIDTH);
  }
  struct Cell cells[AMOUNT_OF_CELLS];
  initializeCells(map, cells);
  char* name = promptString("What is your name?");
  struct Cell playerCell = cells[DEFAULT_CELL];
  struct Object* objects = initializeObjects(cells);
  int foundObjects[AMOUNT_OF_OBJECTS];
  struct Player player = {name, DEFAULT_ENERGY, &playerCell, -1, 0};
  struct Monster monster = {1000, &cells[AMOUNT_OF_CELLS - 1]};

  int gameOver = 0;
  while (!gameOver) {
    moveMonster(&monster, cells);
    printf("You are at %s and have %d energy. You did not meet the monster. The monster is at %s.\n",
      player.cell->description, player.energy, monster.cell->description);
    
    int inputInvalid = 0;
    do {
      inputInvalid = 0;
      char* possibleMoves = malloc(100);
      if (player.cell->north != -1) {
        strcat(possibleMoves, "n");
      }
      if (player.cell->south != -1) {
        strcat(possibleMoves, "s");
      }
      if (player.cell->east != -1) {
        strcat(possibleMoves, "e");
      }
      if (player.cell->west != -1) {
        strcat(possibleMoves, "w");
      }
      if (player.cell->up != -1) {
        strcat(possibleMoves, "u");
      }
      if (player.cell->down != -1) {
        strcat(possibleMoves, "d");
      }

      char* buffer = malloc(100);
      sprintf(buffer, "Where do you want to go? (%s)", possibleMoves);
      char* input = promptString(buffer);

      if (strcmp(input, "n") == 0 && player.cell->north != -1) {
        player.cell = &cells[player.cell->north];
      } else if (strcmp(input, "s") == 0 && player.cell->south != -1) {
        player.cell = &cells[player.cell->south];
      } else if (strcmp(input, "e") == 0 && player.cell->east != -1) {
        player.cell = &cells[player.cell->east];
      } else if (strcmp(input, "w") == 0 && player.cell->west != -1) {
        player.cell = &cells[player.cell->west];
      } else if (strcmp(input, "u") == 0 && player.cell->up != -1) {
        player.cell = &cells[player.cell->up];
      } else if (strcmp(input, "d") == 0 && player.cell->down != -1) {
        player.cell = &cells[player.cell->down];
      } else {
        printf("Invalid input. Try again.\n");
        inputInvalid = 1;
      }
    } while (inputInvalid);

    if (player.cell->hasObject) {
      struct Object found = randomizeObject(objects, foundObjects);
      char buffer[256];
      sprintf(buffer, "You found %s! It deals %d energy. Do you want to pick it up? (y/n)", 
        found.name, found.energy);
      char* input = promptString(buffer);
      if (strcmp(input, "y") == 0) {
        printf("You now own the %s!\n", found.name);
        player.carriedObject = found.id;
      } else {
        printf("You dropped the object.\n");
      }
    }

    if (player.cell == monster.cell) {
      printf("You met the monster! It's time to fight!\n");
      int monsterDead = 0;
      while (!monsterDead) {
        char* input = promptString("Do you want to attack? (y/n)");
        if (strcmp(input, "y") == 0) {
          int maxDealtEnergy = player.carriedObject ? objects[player.carriedObject].energy : 10;
          int dealtEnergy = rand() % maxDealtEnergy;
          monster.energy -= dealtEnergy;
          printf("You dealt %d energy to the monster! The monster now has %d energy.\n", dealtEnergy, monster.energy);
          if (monster.energy <= 0) {
            monsterDead = 1;
          } else {
            dealtEnergy = rand() % 100;
            player.energy -= dealtEnergy;
            printf("The monster dealt %d energy to you! You now have %d energy.\n", dealtEnergy, player.energy);
            if (player.energy <= 0) {
              gameOver = 1;
              printf("You died!\n");
            }
          }
        } else {
          printf("You ran away!\n");
          break;
        }
      }

      if (monsterDead) {
        printf("You killed the monster! Congratulations!\n");
        gameOver = 1;
      } else {
        printf("The monster's energy is now %d.", monster.energy);
      }
    }
  }

  printf("Game over!\n");

  return 0;
}
