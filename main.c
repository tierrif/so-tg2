#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_ENERGY 5000
#define DEFAULT_CELL 1
#define AMOUNT_OF_CELLS 20
#define AMOUNT_OF_OBJECTS 10

struct Cell {
  int north,
      south,
      east,
      west,
      up,
      down,
      hasObject,
      hasTreasure;
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

void generateCellForDirection(int* direction, int i, struct Cell cell) {
  *direction = rand() % AMOUNT_OF_CELLS;
  while (*direction == i || *direction == cell.north
    || *direction == cell.south || *direction == cell.east
    || *direction == cell.west || *direction == cell.up
    || *direction == cell.down) {
    *direction = rand() % AMOUNT_OF_CELLS;
  }
}

void randomizeCells(struct Cell* cells) {
  for (int i = 0; i < AMOUNT_OF_CELLS; i++) {
    generateCellForDirection(&cells[i].north, i, cells[i]);
    generateCellForDirection(&cells[i].south, i, cells[i]);
    generateCellForDirection(&cells[i].east, i, cells[i]);
    generateCellForDirection(&cells[i].west, i, cells[i]);
    generateCellForDirection(&cells[i].up, i, cells[i]);
    generateCellForDirection(&cells[i].down, i, cells[i]);

    cells[i].hasObject = 0;
    cells[i].hasTreasure = 0;
    cells[i].description = malloc(100);
    sprintf(cells[i].description, "Cell %d", i);
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
  struct Cell cells[AMOUNT_OF_CELLS];
  randomizeCells(cells);
  char* name = promptString("What is your name?");
  struct Cell playerCell = cells[DEFAULT_CELL];
  struct Object* objects = initializeObjects(cells);
  int foundObjects[AMOUNT_OF_OBJECTS];
  struct Player player = {name, DEFAULT_ENERGY, &playerCell, -1, 0};
  struct Monster monster = {1000, &cells[AMOUNT_OF_CELLS - 1]};

  int gameOver = 0;
  while (!gameOver) {
    moveMonster(&monster, cells);
    printf("You are at %s and have %d energy. You did not meet the monster.\n",
      player.cell->description, player.energy);
    
    int inputInvalid = 0;
    do {
      inputInvalid = 0;
      char* input = promptString("Where to move? (n, s, e, w, u, d)");
      if (strcmp(input, "n") == 0) {
        player.cell = &cells[player.cell->north];
      } else if (strcmp(input, "s") == 0) {
        player.cell = &cells[player.cell->south];
      } else if (strcmp(input, "e") == 0) {
        player.cell = &cells[player.cell->east];
      } else if (strcmp(input, "w") == 0) {
        player.cell = &cells[player.cell->west];
      } else if (strcmp(input, "u") == 0) {
        player.cell = &cells[player.cell->up];
      } else if (strcmp(input, "d") == 0) {
        player.cell = &cells[player.cell->down];
      } else {
        printf("Invalid input. Try again.");
        inputInvalid = 1;
      }
    } while (inputInvalid);

    if (player.cell->hasObject) {
      char* input = promptString("You found an object! Do you want to pick it up? (y/n)");
      if (strcmp(input, "y") == 0) {
        struct Object found = randomizeObject(objects, &foundObjects);
        printf("You found a %s! It has a damage of %d energy!\n", found.name, found.energy);
      } else {
        printf("You dropped the object.");
      }
    }

    if (player.cell == monster.cell) {
      char* screamsOrSomething = promptString("<insert loud boss music here>");
    }
  }
}
