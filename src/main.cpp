#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <algorithm>
#include <random>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <time.h>
#include "curses.h"
#include "Timer.h"

static bool QUIT = false;
static bool LOST = false;
struct Enemy;
std::vector<Enemy> enemies;
std::vector<char> typed_chars;
std::vector<std::string> word_list;
double level_freq = 10;
double move_freq = 0.5;
double spawn_freq = 3.0;
double move_accu = 0.0;
double spawn_accu = 0.0;
uint32_t word_inc = 0;
uint32_t kill_inc = 0;
uint32_t kill_accu = 0;
Timer timer;
short width, height;

uint32_t score = 0;
short lives = 3;
struct Enemy
{
    std::string value;
    int x;
    int y;
    uint32_t id = 0;
};

#define CLR_WHITE 1
#define CLR_CYAN 2
#define CLR_RED 3
#define CLR_YELLOW 4
#define CLR_STATUS_BAR 5
#define CLR_ALERT 6
void ImportWordList();
void InitEnemies();
void KillEnemy(size_t idx);
void UpdateEnemies();
void CheckSpelling();
void DisplayStatusBar();
int main()
{

    timer.Start();
    ImportWordList();

    InitEnemies();
    initscr();
    getmaxyx(stdscr, height, width);
    start_color();
    init_pair(CLR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(CLR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(CLR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(CLR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(CLR_STATUS_BAR, COLOR_YELLOW, COLOR_BLUE);
    init_pair(CLR_ALERT, COLOR_WHITE, COLOR_RED);
    cbreak();
    keypad(stdscr, true);
    nodelay(stdscr, true);
    noecho();
    while (!QUIT)
    {
        int c = getch();
        if (c == 27) // ESCAPE KEY
        {
            QUIT = true;
        }
        else if ((c >= 97 && c <= 122) || c == 45) // between a and z or 45 '-' ( minus sign)
        {
            typed_chars.push_back(c);
        }
        else if (c == 8) // backspace
        {
            if (typed_chars.size() > 0)
            {
                typed_chars.pop_back();
            }
        }
        if (!LOST)
        {

            timer.Update();
            UpdateEnemies();
            clear();
            DisplayStatusBar();

            uint32_t inc = 0;
            for (auto &enemy : enemies)
            {

                int pair_num = (int)(enemy.id % 4) + 1;

                if (enemy.x + enemy.value.length() > width - 30)
                {
                    pair_num = CLR_ALERT;
                }
                attr_on(COLOR_PAIR(pair_num), (void *)0);
                mvprintw(enemy.y, enemy.x, enemy.value.c_str());
                attr_off(COLOR_PAIR(pair_num), (void *)0);

                if (enemy.x + enemy.value.length() > width - 1)
                {
                    enemies.erase(enemies.begin() + inc);
                    if (lives > 0)
                    {

                        lives -= 1;
                    }
                    else
                    {
                        LOST = true;
                    }
                }
                inc++;
            }
            CheckSpelling();
            refresh();
        }

        DisplayStatusBar();
        Sleep(15);
    }
    clear();
    refresh();
    endwin();
    return 0;
}

void ImportWordList()
{
    std::fstream infile("C:/gui2one/CODE/Typo/resources/clean_words.txt", std::ios::in);

    std::string line;
    while (std::getline(infile, line))
    {
        if (line.length() > 3)
        {
            // std::cout << " LINE : " << line << std::endl;

            line.erase(std::remove(line.begin(), line.end(), '\n'),
                       line.end());
            word_list.push_back(line);
            // std::cout << " size : " << word_list.size() << std::endl;
        }
    }

    std::random_device rd;
    std::mt19937 generator(rd());
    generator.seed(time(nullptr));
    std::shuffle(word_list.begin(), word_list.end(), generator);
}

void InitEnemies()
{
    Enemy word1 = {word_list[0], 5, 5};
    word1.id = 0;
    Enemy word2 = {word_list[1], 3, 2};
    word2.id = 1;
    word_inc += 2;
    enemies.push_back(word1);
    enemies.push_back(word2);
}

void KillEnemy(size_t idx)
{
    enemies.erase(enemies.begin() + idx);
    typed_chars.clear();
}

void UpdateEnemies()
{
    move_accu += timer.delta_seconds;
    spawn_accu += timer.delta_seconds;
    // std::cout << move_accu << std::endl;
    if (move_accu > move_freq)
    {
        move_accu = 0.0;
        for (auto &enemy : enemies)
        {
            enemy.x += 1;
        }
    }

    if (spawn_accu > spawn_freq)
    {
        spawn_accu = 0.0;

        Enemy new_enemy = {word_list[word_inc], 1, rand() % 10 + 3};

        new_enemy.id = ++word_inc;

        enemies.push_back(new_enemy);
    }
}

void CheckSpelling()
{
    if (typed_chars.size() == 0)
        return;
    for (size_t enemy_idx = 0; enemy_idx < enemies.size(); enemy_idx++)
    {
        auto &enemy = enemies[enemy_idx];
        bool ok = true;
        int good_letters = 0;
        auto str = enemy.value.c_str();
        for (size_t i = 0; i < enemy.value.length(); i++)
        {
            if (i < typed_chars.size())
            {

                if (str[i] != typed_chars[i])
                {
                    ok = false;
                    break;
                }
                good_letters++;
            }
        }
        if (ok && good_letters == enemy.value.length())
        {
            // std::cout << "delete enemy !!!!!!" << std::endl;
            KillEnemy(enemy_idx);
            score += good_letters;
            kill_inc++;
            kill_accu++;
            if (kill_accu > level_freq)
            {
                kill_accu = 0;
                move_freq *= 0.7;
                spawn_freq *= 0.7;
            }
            break;
        }
    }
}

void DisplayStatusBar()
{

    // empty bar
    char empty[250];
    memset(empty, ' ', width);
    attr_on(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
    mvprintw(height - 1, 0, empty);
    attr_off(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);

    // display typed_chars
    char chars[512];
    for (size_t i = 0; i < typed_chars.size(); i++)
    {
        chars[i] = typed_chars[i];
    }
    chars[typed_chars.size()] = '\0';
    attr_on(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
    mvprintw(height - 1, 1, chars);
    attr_off(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);

    // display score

    char score_str[10];
    sprintf(score_str, "score : %d", score);
    attr_on(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
    mvprintw(height - 1, 20, score_str);
    attr_off(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);

    // display lives

    char lives_str[10];
    sprintf(lives_str, "--lives : %d", lives);
    attr_on(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
    mvprintw(height - 1, 40, lives_str);
    attr_off(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);

    // display stats
    char stats_str[30];
    float average = score / (timer.seconds / 60);
    sprintf(stats_str, "--stats : %.3f chars/mn", average);
    attr_on(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
    mvprintw(height - 1, 60, stats_str);
    attr_off(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
}