#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <graphics.h>
#include <conio.h>
#include <string.h>

#define WIDTH 640
#define HEIGHT 480

#define SPACESHIP 'A'
#define ENEMY 'V'
#define BULLET '*'
#define ENEMY_BULLET '|'
#define OBSTACLE '0'

#define GAME_SPEED 10
#define LEADERBOARD_SIZE 5

typedef struct GameObject {
    int x, y;
    char symbol;
    struct GameObject* next;
} GameObject;

typedef struct {
    char name[20];
    int score;
} LeaderboardEntry;

// Global variables
GameObject* spaceship = NULL;
GameObject* enemies = NULL;
GameObject* bullets = NULL;
GameObject* enemyBullets = NULL;
GameObject* obstacles = NULL;

int score = 0;
int life = 5;
int gamePaused = 0;
LeaderboardEntry leaderboard[LEADERBOARD_SIZE];

// Function prototypes
GameObject* createObject(int x, int y, char symbol);
void addObject(GameObject** list, GameObject* obj);
void removeObject(GameObject** list, GameObject* obj);
void deletegame();
void putgame();
char getInput();
void gamelogic();
void delay(unsigned int milliseconds);
void render();
void move(char input);
void game();
void sortLeaderboard();
void saveLeaderboard();
void loadLeaderboard();
void addScoreToLeaderboard(const char* name, int score);
void menu();
void renderMenu();
int getHighestScore();

GameObject* createObject(int x, int y, char symbol) {
    GameObject* obj = (GameObject*)malloc(sizeof(GameObject));
    obj->x = x;
    obj->y = y;
    obj->symbol = symbol;
    obj->next = NULL;
    return obj;
}

void addObject(GameObject** list, GameObject* obj) {
    if (*list == NULL) {
        *list = obj;
    } else {
        GameObject* temp = *list;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = obj;
    }
}

void removeObject(GameObject** list, GameObject* obj) {
    if (*list == obj) {
        *list = obj->next;
    } else {
        GameObject* temp = *list;
        while (temp != NULL && temp->next != obj) {
            temp = temp->next;
        }
        if (temp != NULL) {
            temp->next = obj->next;
        }
    }
    free(obj);
}

// Function to release memory allocated for game objects
void deletegame() {
    while (spaceship != NULL) {
        GameObject* temp = spaceship;
        spaceship = spaceship->next;
        free(temp);
    }

    while (enemies != NULL) {
        GameObject* temp = enemies;
        enemies = enemies->next;
        free(temp);
    }

    while (bullets != NULL) {
        GameObject* temp = bullets;
        bullets = bullets->next;
        free(temp);
    }

    while (enemyBullets != NULL) {
        GameObject* temp = enemyBullets;
        enemyBullets = enemyBullets->next;
        free(temp);
    }

    while (obstacles != NULL) {
        GameObject* temp = obstacles; 
        obstacles = obstacles->next;
        free(temp);
    }
}

void putgame() {
    int i; 

    spaceship = createObject(WIDTH / 2, HEIGHT - 40, SPACESHIP);
    for (i = 0; i < 3; i++) {
        addObject(&enemies, createObject(rand() % (WIDTH - 20) + 10, 20, ENEMY));
    }
}

char getInput() {
    if (kbhit()) {
        return getch();
    }
    return '\0';
}

void gamelogic() {
    GameObject* enemy = enemies;
    GameObject* bullet = bullets;
    GameObject* enemyBullet = enemyBullets;
    GameObject* obstacle = obstacles;

    if (gamePaused) return;

    while (bullet != NULL) {
        bullet->y -= 8;
        if (bullet->y <= 0) {
            GameObject* temp = bullet->next;
            removeObject(&bullets, bullet);
            bullet = temp;
        } else {
            bullet = bullet->next;
        }
    }

    while (enemyBullet != NULL) {
        enemyBullet->y += 5;
        if (enemyBullet->y >= HEIGHT) {
            GameObject* temp = enemyBullet->next;
            removeObject(&enemyBullets, enemyBullet);
            enemyBullet = temp;
        } else {
            enemyBullet = enemyBullet->next;
        }
    }

    while (enemy != NULL) {
        enemy->y += 4;
        if ((rand() % 100) > 97) {
            addObject(&enemyBullets, createObject(enemy->x, enemy->y + 5, ENEMY_BULLET));
        }
        if (enemy->y >= HEIGHT) 
        {
            enemy->x = rand() % (WIDTH - 20) + 10;
            enemy->y = 20;
        }
        enemy = enemy->next;
    }

    while (obstacle != NULL) {
        obstacle->y += 3;
        if (obstacle->y >= HEIGHT) {
            GameObject* temp = obstacle->next;
            removeObject(&obstacles, obstacle);
            obstacle = temp;
        } else {
            obstacle = obstacle->next;
        }
    }

    bullet = bullets;
    while (bullet != NULL) {
        enemy = enemies;
        while (enemy != NULL) {
            if (bullet->x >= enemy->x - 10 && bullet->x <= enemy->x + 10 
                && bullet->y >= enemy->y - 10 && bullet->y <= enemy->y + 10) {
                GameObject* temp_bullet = bullet->next;
                GameObject* temp_enemy = enemy->next;
                removeObject(&bullets, bullet);
                removeObject(&enemies, enemy);
                addObject(&enemies, createObject(rand() % (WIDTH - 20) + 10, 20, ENEMY));
                score += 10;
                bullet = temp_bullet;
                enemy = temp_enemy;
                continue;
            }
            enemy = enemy->next;
        }
        bullet = bullet->next;
    }

    enemy = enemies;
    while (enemy != NULL) {
        if (spaceship->x >= enemy->x - 10 && spaceship->x <= enemy->x + 10 && spaceship->y >= enemy->y - 10 
            && spaceship->y <= enemy->y + 10) {
            life -= 1;
            removeObject(&enemies, enemy);
            addObject(&enemies, createObject(rand() % (WIDTH - 20) + 10, 20, ENEMY));
            break;
        }
        enemy = enemy->next;
    }

    enemyBullet = enemyBullets;
    while (enemyBullet != NULL) {
        if (spaceship->x >= enemyBullet->x - 5 && spaceship->x <= enemyBullet->x + 5 && spaceship->y >= enemyBullet->y - 10 
            && spaceship->y <= enemyBullet->y + 10) {
            life -= 1;
            removeObject(&enemyBullets, enemyBullet);
            break;
        }
        enemyBullet = enemyBullet->next;
    }

    obstacle = obstacles;
    while (obstacle != NULL) {
        if (spaceship->x >= obstacle->x - 10 && spaceship->x <= obstacle->x + 10 && spaceship->y >= obstacle->y - 10 
            && spaceship->y <= obstacle->y + 10) {
            life -= 1;
            removeObject(&obstacles, obstacle);
            break;
        }
        obstacle = obstacle->next;
    }
}

void delay(unsigned int milliseconds) {
    clock_t start_time = clock();
    while (clock() < start_time + milliseconds * CLOCKS_PER_SEC / 1000);
}

void render() {
    GameObject* temp = enemies;
    char scoreText[20];
    char lifeText[20];
    char highScoreText[30];

    int highestScore = getHighestScore();

    cleardevice();

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(spaceship->x - 10, spaceship->y - 10, "A");

    while (temp != NULL) {
        outtextxy(temp->x - 4, temp->y - 4, "V");
        temp = temp->next;
    }

    temp = bullets;
    while (temp != NULL) {
        outtextxy(temp->x - 5, temp->y - 10, "*");
        temp = temp->next;
    }

    temp = enemyBullets;
    while (temp != NULL) {
        outtextxy(temp->x - 5, temp->y - 10, "|");
        temp = temp->next;
    }

    temp = obstacles;
    while (temp != NULL) {
        outtextxy(temp->x - 4, temp->y - 4, "0");
        temp = temp->next;
    }

    sprintf(scoreText, "Score: %d", score);
    outtextxy(10, 10, scoreText);

    sprintf(lifeText, "Life: %d", life);
    outtextxy(WIDTH - 150, 10, lifeText);
 
    sprintf(highScoreText, "Highscore: %d", highestScore);
    outtextxy(WIDTH/2 - 100, 10, highScoreText);

    if (life <= 0) {
        outtextxy(WIDTH / 2 - 50, HEIGHT / 2, "GAME OVER");
    }

    if (gamePaused) {
        outtextxy(WIDTH / 2 - 50, HEIGHT / 2 - 50, "PAUSED");
    }

    delay(GAME_SPEED);
}

void move(char input) {
    switch (input) {
        case 'a':
        case 'A' :
            if (spaceship->x > 10) spaceship->x -= 10;
            break;
        case 'd':
        case 'D' :
            if (spaceship->x < WIDTH - 10) spaceship->x += 10;
            break;
        case 'w':
        case 'W' :
            if (spaceship->y > 10) spaceship->y -= 10;
            break;
        case 's':
        case 'S' :
            if (spaceship->y < HEIGHT - 10) spaceship->y += 10;
            break;
        case ' ':
            addObject(&bullets, createObject(spaceship->x, spaceship->y - 10, BULLET));
            break;
    }
}

void game() {
    char input;
    char name[50];

    int gd = DETECT, gm;
    initgraph(&gd, &gm, "");

    putgame();

    while (1) 
    {
        if (life <= 0)
        {
            cleardevice();
            outtextxy(WIDTH / 2 - 100, HEIGHT / 2, "Enter your name: ");
            scanf("%s", name);
            addScoreToLeaderboard(name, score);
            deletegame();
            renderMenu();
        }

        input = getInput();
        if (input == 'p' || input == 'P') 
        {
            gamePaused = !gamePaused;
        }

        if (input == 'q' || input == 'Q')
        {
            
            deletegame();
            cleardevice();
            renderMenu();
        }

            if (input == 'e' || input == 'E')
        {
            
            cleardevice();
            outtextxy(WIDTH / 2 - 100, HEIGHT / 2, "Enter your name: ");
            scanf("%s", name);
            addScoreToLeaderboard(name, score);
            deletegame();
            renderMenu();
        }

        if (!gamePaused) 
        {
            move(input);
            gamelogic();
        }
        render();
    }
}

void sortLeaderboard() {
    int i, j;

    for (i = 0; i < LEADERBOARD_SIZE - 1; i++) {
        for (j = i + 1; j < LEADERBOARD_SIZE; j++) {
            if (leaderboard[j].score > leaderboard[i].score) {
                LeaderboardEntry temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }
}

void saveLeaderboard() {
    int i;

    FILE* file = fopen("C:\\TURBOC3\\Projects\\space.txt", "w");
    for (i = 0; i < LEADERBOARD_SIZE; i++) {
        fprintf(file, "%s %d\n", leaderboard[i].name, leaderboard[i].score);
    }
    fclose(file);
}

void loadLeaderboard() {
    int i;

    FILE* file = fopen("C:\\TURBOC3\\Projects\\space.txt", "r");
    if (file != NULL) {
        for (i = 0; i < LEADERBOARD_SIZE; i++) {
            fscanf(file, "%s %d", leaderboard[i].name, &leaderboard[i].score);
        }
        fclose(file);
    }
}

void addScoreToLeaderboard(const char* name, int score) {
    int i, j;

    for (i = 0; i < LEADERBOARD_SIZE; i++) {
        if (score > leaderboard[i].score) {
            for (j = LEADERBOARD_SIZE - 1; j > i; j--) {
                leaderboard[j] = leaderboard[j - 1];
            }
            strcpy(leaderboard[i].name, name);
            leaderboard[i].score = score;
            break;
        }
    }
    saveLeaderboard();
}

void renderMenu() {
    char choice;
    int i;
    char buffer[100];

    while (1){
        cleardevice();
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
        outtextxy(WIDTH / 2 - 150, HEIGHT / 2 - 100, "SPACE IMPACT");

        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(WIDTH / 2 - 50, HEIGHT / 2, "1. Start Game");
        outtextxy(WIDTH / 2 - 50, HEIGHT / 2 + 30, "2. Challange Mode");
        outtextxy(WIDTH / 2 - 50, HEIGHT / 2 + 60, "3. High Scores");
        outtextxy(WIDTH / 2 - 50, HEIGHT / 2 + 90, "4. Exit");

        choice = getch();
        switch (choice) 
        {
            case '1':

                score = 0; // Reset score
                life = 5;  // Reset life

                game();
                break;
            
            case '2':

                score = 0; // Reset score
                life = 1;  // Reset life

                game();
                break;
                
            case '3':
                cleardevice();
                outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 100, "TOP 5 HIGHSCORER");

                for (i = 0; i < LEADERBOARD_SIZE; i++) {
                    sprintf(buffer, "%d. %s - %d", i + 1, leaderboard[i].name, leaderboard[i].score);
                    outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 70 + (i * 30), buffer);
                }
                getch();
                break;
            case '4':
                cleardevice();
                outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 100, "exiting..............");
                delay(1500);
                closegraph();
        }
    }
}

void menu() {
    int gd = DETECT, gm;
    initgraph(&gd, &gm, "");
    loadLeaderboard();
    renderMenu();
    closegraph();
}

int getHighestScore() 
{
    int i;
    int highestScore = 0;
    for (i = 0; i < LEADERBOARD_SIZE; i++) 
    {
        if (leaderboard[i].score > highestScore) 
        {
            highestScore = leaderboard[i].score;
        }
    }
    return highestScore;
}

int main() {
    srand(time(NULL));
    menu();
    deletegame();
    return 0;
}