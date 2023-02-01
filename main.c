#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define DEFAULT_ENERGY 50
#define DEFAULT_CELL 0
#define NONE (-1)
#define MONSTER_INITIAL_CELL 6
#define MONSTER_ENERGY 2000

#define OBJECT_T_HEALTH 0
#define OBJECT_T_WEAPON 1
#define OBJECT_T_TRAP 2
#define MAX_CELLS 5000
#define MAX_OBJECTS 500

struct Cell {
  int north,
      south,
      east,
      west,
      up,
      down,
      object,
      hasTreasure,
      isDefault;
  char *description;
};

struct Player {
  char *name;
  int energy;
  struct Cell *cell;
  int carriedObject; // -1 se não tem, senão código do objeto.
  int carriesTreasure;
};

struct Monster {
  int energy;
  struct Cell *cell;
};

struct Object {
  char *name;
  int energy;
  int type;
  int id;
  char *trapDescription; // NULL se não é do tipo 'armadilha' (trap).
};

struct Game {
  struct Player *player;
  struct Monster *monster;
  struct Cell *cells;
  struct Object *objects;
  int *monsterDead;
};

char *promptString(char *prompt);

void initializeCells(struct Cell *cells);

void initializeObjects(struct Object *objects);

void moveMonster(struct Monster *monster, struct Cell *cells);

void *player_thread(void *ptr);

void *monster_thread(void *ptr);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main() {
  printf("Welcome to Aventure Game v0.0.1.\n");
  printf("The objective is to defeat the monster and steal the treasure. Good luck!\n");
  struct Cell cells[MAX_CELLS];
  struct Object objects[MAX_OBJECTS];
  initializeCells(cells);
  initializeObjects(objects);
  char *name = promptString("What is your name?");
  struct Player player = {name, DEFAULT_ENERGY, &cells[DEFAULT_CELL], NONE, 0};
  struct Monster monster = {MONSTER_ENERGY, &cells[MONSTER_INITIAL_CELL]};

  int monsterDead = 0;

  pthread_t playerT, monsterI;
  struct Game game = {
      &player,
      &monster,
      cells,
      objects,
      &monsterDead
  };

  pthread_create(&playerT, NULL, player_thread, (void *) &game);
  pthread_create(&monsterI, NULL, monster_thread, (void *) &game);

  pthread_join(playerT, NULL);
  pthread_join(monsterI, NULL);

  return 0;
}

void *player_thread(void *ptr) {
  struct Game *game = ((struct Game *) ptr);
  struct Cell *cells = game->cells;
  struct Player *player = game->player;
  struct Object *objects = game->objects;
  struct Monster *monster = game->monster;

  int gameOver = 0;
  int foundObjects[MAX_OBJECTS];
  printf("> %s\n", cells[DEFAULT_CELL].description);
  while (!gameOver) {
    if (*game->monsterDead && player->carriesTreasure) {
      if (player->cell->isDefault) {
        printf("You escaped!\n");
        break;
      }

      printf("!!! You are now ready to leave to complete your mission.\n");
    }

    printf("You have %d energy.\n", player->energy);

    int inputInvalid;
    do {
      inputInvalid = 0;
      char *possibleMoves = malloc(100);
      if (player->cell->north != NONE) {
        strcat(possibleMoves, "(n)orth,");
      }
      if (player->cell->south != NONE) {
        strcat(possibleMoves, "(s)outh,");
      }
      if (player->cell->east != NONE) {
        strcat(possibleMoves, "(e)ast,");
      }
      if (player->cell->west != NONE) {
        strcat(possibleMoves, "(w)est,");
      }
      if (player->cell->up != NONE) {
        strcat(possibleMoves, "(u)p,");
      }
      if (player->cell->down != NONE) {
        strcat(possibleMoves, "(d)own,");
      }
      size_t size = strlen(possibleMoves);
      possibleMoves[size - 1] = '\0';

      char *buffer = malloc(100);
      sprintf(buffer, "Where do you want to go? (%s)", possibleMoves);
      char *input = promptString(buffer);

      if (strcmp(input, "n") == 0 && player->cell->north != -1) {
        player->cell = &cells[player->cell->north];
      } else if (strcmp(input, "s") == 0 && player->cell->south != -1) {
        player->cell = &cells[player->cell->south];
      } else if (strcmp(input, "e") == 0 && player->cell->east != -1) {
        player->cell = &cells[player->cell->east];
      } else if (strcmp(input, "w") == 0 && player->cell->west != -1) {
        player->cell = &cells[player->cell->west];
      } else if (strcmp(input, "u") == 0 && player->cell->up != -1) {
        player->cell = &cells[player->cell->up];
      } else if (strcmp(input, "d") == 0 && player->cell->down != -1) {
        player->cell = &cells[player->cell->down];
      } else {
        printf("Invalid input. Try again.\n");
        inputInvalid = 1;
      }
    } while (inputInvalid);

    if (player->cell->object != NONE) {
      struct Object found = objects[player->cell->object];
      if (found.type == OBJECT_T_WEAPON && !foundObjects[found.id]) {
        char buffer[256];
        sprintf(buffer, "You found a %s! It deals %d energy. Do you want to pick it up? (y/n)",
                found.name, found.energy);
        char *input = promptString(buffer);
        if (strcmp(input, "y") == 0) {
          foundObjects[found.id] = 1;
          printf("You now own this %s!\n", found.name);
          player->carriedObject = found.id;
        }
      } else if (found.type == OBJECT_T_TRAP) {
        printf("%s\n", found.trapDescription);
        player->energy -= found.energy;
        if (player->energy <= 0) {
          printf("You died!\n");
          break;
        }
      } else if (found.type == OBJECT_T_HEALTH && !foundObjects[found.id]) {
        printf("You found a %s! Your energy increased by %d.\n", found.name, found.energy);
        player->energy += found.energy;
        foundObjects[found.id] = 1;
      }
    } else if (player->cell->hasTreasure) {
      printf("You found the treasure!\n");
      if (*game->monsterDead) {
        printf("You can escape now.\n");
      } else {
        printf("Defeat the monster and escape.\n");
      }

      player->carriesTreasure = 1;
    }

    printf("> %s\n", player->cell->description);

    if (player->cell == monster->cell) {
      printf("You met the monster! It's time to fight!\n");
      while (!(*game->monsterDead)) {
        char *input = promptString("What to do? ((w)eak attack,(s)trong attack,(r)un away)");
        if (strcmp(input, "w") == 0 || strcmp(input, "s") == 0) {
          int maxDealtEnergy = player->carriedObject ? objects[player->carriedObject].energy : 10;
          int failAttack = (rand() % 2);
          int dealtEnergy = (strcmp(input, "w") == 0)
                            ? rand() % (maxDealtEnergy - 5)
                            : rand() % (maxDealtEnergy + 200);

          // Apenas falhar se for "strong attack".
          if (failAttack && strcmp(input, "s") == 0) {
            printf("You missed.\n");
          } else {
            monster->energy -= dealtEnergy;
            printf("You dealt %d energy to the monster! The monster now has %d energy.\n", dealtEnergy,
                   monster->energy);
          }
          if (monster->energy <= 0) {
            *game->monsterDead = 1;
            pthread_mutex_lock(&lock);
            monster->cell = NULL;
            pthread_mutex_unlock(&lock);
          } else {
            failAttack = (rand() % 2);
            if (failAttack) {
              printf("The monster missed.\n");
              continue;
            }

            dealtEnergy = rand() % 50;
            player->energy -= dealtEnergy;
            printf("The monster dealt %d energy to you! You now have %d energy.\n", dealtEnergy, player->energy);
            if (player->energy <= 0) {
              gameOver = 1;
              printf("You died!\n");
              break;
            }
          }
        } else {
          printf("You ran away!\n");
          break;
        }
      }

      if (*game->monsterDead) {
        printf("You killed the monster! Congratulations!\n");
      } else {
        printf("The monster's energy is now %d.\n", monster->energy);
      }
    }
  }

  printf("Game over!\n");

  return (void *) NULL;
}

void* monster_thread(void* ptr) {
  struct Game *game = ((struct Game *) ptr);
  struct Cell *cells = game->cells;
  struct Monster *monster = game->monster;

  while (!*game->monsterDead && game->player->energy > 0) {
    pthread_mutex_lock(&lock);
    sleep(5);
    moveMonster(monster, cells);
    pthread_mutex_unlock(&lock);
  }

  return (void*) NULL;
}

char *promptString(char *prompt) {
  char *buff = malloc(strlen(prompt) + 1);
  strcpy(buff, prompt);
  strcat(buff, " ");

  char *name = malloc(100);
  printf("%s", buff);
  scanf("%s", name);
  return name;
}

void initializeCells(struct Cell *cells) {
  // Cell 0
  cells[0].north = NONE;
  cells[0].south = NONE;
  cells[0].west = NONE;
  cells[0].east = 1;
  cells[0].up = NONE;
  cells[0].down = NONE;
  cells[0].object = NONE;
  cells[0].description = (char *) malloc(100);
  cells[0].isDefault = 1;
  sprintf(cells[0].description, "You're in front of a big castle. You hear strange noises coming from inside.");

  // Cell 1
  cells[1].north = 3;
  cells[1].south = 4;
  cells[1].west = 0;
  cells[1].east = 2;
  cells[1].up = NONE;
  cells[1].down = 5;
  cells[1].object = NONE;
  cells[1].description = (char *) malloc(100);
  strcpy(cells[1].description, "You're at the castle's hall. You see a lot of doors and stairs that seem to go down.");

  // Cell 2
  cells[2].north = NONE;
  cells[2].south = NONE;
  cells[2].west = 1;
  cells[2].east = NONE;
  cells[2].up = 6;
  cells[2].down = NONE;
  cells[2].object = NONE;
  cells[2].description = (char *) malloc(100);
  strcpy(cells[2].description, "You see very long stairs that seem to go up.");

  // Cell 3
  cells[3].north = NONE;
  cells[3].south = 1;
  cells[3].west = NONE;
  cells[3].east = NONE;
  cells[3].up = NONE;
  cells[3].down = NONE;
  cells[3].object = 0;
  cells[3].description = (char *) malloc(100);
  strcpy(cells[3].description, "You're in a small room with a chest in the center.");

  // Cell 4
  cells[4].north = 1;
  cells[4].south = NONE;
  cells[4].west = NONE;
  cells[4].east = NONE;
  cells[4].up = NONE;
  cells[4].down = 7;
  cells[4].object = NONE;
  cells[4].description = (char *) malloc(100);
  strcpy(cells[4].description, "A very, very dark set of stairs goes down, deep underground. I wouldn't go there.");

  // Cell 5
  cells[5].north = NONE;
  cells[5].south = NONE;
  cells[5].west = NONE;
  cells[5].east = 8;
  cells[5].up = 1;
  cells[5].down = NONE;
  cells[5].object = NONE;
  cells[5].description = (char *) malloc(100);
  strcpy(cells[5].description, "There's a very fancy door right in front of you.");

  // Cell 6
  cells[6].north = 9;
  cells[6].south = NONE;
  cells[6].west = NONE;
  cells[6].east = NONE;
  cells[6].up = NONE;
  cells[6].down = 2;
  cells[6].object = 3;
  cells[6].description = (char *) malloc(100);
  strcpy(cells[6].description, "You're at what seems to be the throne room.");

  // Cell 7
  cells[7].north = NONE;
  cells[7].south = NONE;
  cells[7].west = NONE;
  cells[7].east = NONE;
  cells[7].up = 4;
  cells[7].down = NONE;
  cells[7].object = 1;
  cells[7].description = (char *) malloc(100);
  strcpy(cells[7].description, "Spikes hit you. It was a trap. You were warned.");

  // Cell 8
  cells[8].north = NONE;
  cells[8].south = NONE;
  cells[8].west = 5;
  cells[8].east = NONE;
  cells[8].up = NONE;
  cells[8].down = NONE;
  cells[8].object = NONE;
  cells[8].hasTreasure = 1;
  cells[8].description = (char *) malloc(100);
  strcpy(cells[8].description, "You find a very good-looking treasure chest.");

  // Cell 9
  cells[9].north = NONE;
  cells[9].south = 6;
  cells[9].west = NONE;
  cells[9].east = NONE;
  cells[9].up = NONE;
  cells[9].down = NONE;
  cells[9].object = 2;
  cells[9].description = (char *) malloc(100);
  strcpy(cells[9].description, "This seems to be the monster's storage room.");
}

void initializeObjects(struct Object *objects) {
  // Object 0
  objects[0].type = OBJECT_T_WEAPON;
  objects[0].name = "Ancient Sword";
  objects[0].energy = 40;
  objects[0].id = 0;

  // Object 1
  objects[1].type = OBJECT_T_TRAP;
  objects[1].name = "Spikes";
  objects[1].trapDescription = "You got spiked to death.";
  objects[1].energy = 999999;
  objects[1].id = 1;

  // Object 2
  objects[2].type = OBJECT_T_WEAPON;
  objects[2].name = "Honjo Masamune";
  objects[2].energy = 150;
  objects[2].id = 2;

  // Object 3
  objects[3].type = OBJECT_T_HEALTH;
  objects[3].name = "Health Potion";
  objects[3].energy = 450;
  objects[3].id = 3;
}

void moveMonster(struct Monster *monster, struct Cell *cells) {
  int invalid = 1;
  while (invalid) {
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
      default:
        break;
    }
    if (monster->cell->object == 1) {
      invalid = 1;
    } else {
      invalid = 0;
    }
  }
}
