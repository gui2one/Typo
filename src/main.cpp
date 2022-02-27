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
#include <filesystem>
#include <time.h>
#include "ncurses/curses.h"
#include "Timer.h"

static bool QUIT = false;
static bool LOST = false;

char player_name[8] = {'g', 'u', 'i', '2', 'o', 'n', 'e', '\0'};
struct Enemy;
std::vector<Enemy> enemies;
std::vector<char> typed_chars;
std::vector<std::string> word_list;
double level_freq = 10;
double move_freq = 0.02;
double spawn_freq = 3.0;
double move_accu = 0.0;
double spawn_accu = 0.0;
uint32_t word_inc = 0;
uint32_t kill_inc = 0;
uint32_t kill_accu = 0;
Timer timer;
short width, height;
std::filesystem::path root_folder;

uint32_t score = 0;
short lives = 3;
struct Enemy
{
    std::string value;
    int x;
    int y;
    uint32_t id = 0;
};

struct ScoreRecord
{
    uint32_t score;
    uint32_t time_millis;
    std::string player_name;

    bool operator==(const ScoreRecord &other)
    {
        return score == other.score && time_millis == other.time_millis && player_name == other.player_name;
    }
};

std::ostream &operator<<(std::ostream &os, ScoreRecord &record)
{
    os << record.score << "," << record.time_millis << "," << record.player_name << std::endl;

    return os;
}
#define CLR_WHITE 1
#define CLR_CYAN 2
#define CLR_RED 3
#define CLR_YELLOW 4
#define CLR_STATUS_BAR 5
#define CLR_ALERT 6
void ImportWordList();
void ProcessKeys();
void InitGame();
void InitEnemies();
void KillEnemy(size_t idx);
void UpdateEnemies();
void CheckSpelling();
void DisplayStatusBar();

std::vector<ScoreRecord> LoadOldScores();
void DisplayScoreboard();
void SaveScore();
int main()
{
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    root_folder = std::filesystem::path(exe_path).parent_path();

    // SaveScore();
    // return 0;

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

    InitGame();
    while (!QUIT)
    {
        timer.Update();
        ProcessKeys();
        if (!LOST)
        {

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
                        if (score > 0)
                        {
                            SaveScore();
                        }
                    }
                }
                inc++;
            }
            CheckSpelling();
            refresh();
            Sleep(15);
        }
        else
        {
            clear();
            DisplayStatusBar();
            DisplayScoreboard();
            Sleep(200);
        }
    }
    clear();
    refresh();
    endwin();
    return 0;
}

void ImportWordList()
{
    std::filesystem::path words_list_path = root_folder / "../resources/clean_words.txt";
    std::fstream infile(words_list_path, std::ios::in);

    std::string line;
    while (std::getline(infile, line))
    {
        if (line.length() > 3)
        {

            line.erase(std::remove(line.begin(), line.end(), '\n'),
                       line.end());
            word_list.push_back(line);
        }
    }

    infile.close();

    std::random_device rd;
    std::mt19937 generator(rd());
    generator.seed(time(nullptr));
    std::shuffle(word_list.begin(), word_list.end(), generator);
}

void ProcessKeys()
{
    int ch = getch();
    if (ch == 27) // ESCAPE KEY
    {
        QUIT = true;
    }
    else if ((ch >= 97 && ch <= 122) || ch == 45) // between a and z or 45 '-' ( minus sign)
    {
        if (!LOST)
        {

            typed_chars.push_back(ch);
        }
        else
        {
            if (ch == 114) // 'r' key
            {
                InitGame();
            }
        }
    }
    else if (ch == 8) // backspace
    {
        if (typed_chars.size() > 0)
        {
            typed_chars.pop_back();
        }
    }
}

void InitGame()
{
    LOST = false;
    lives = 3;
    score = 0;
    level_freq = 10;
    move_freq = 0.1;
    spawn_freq = 1.0;
    enemies.clear();
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
    if (move_accu > move_freq)
    {

        int num_steps = std::floor(move_accu / move_freq);
        for (auto &enemy : enemies)
        {
            enemy.x += num_steps;
        }

        move_accu = move_accu - (move_freq * num_steps);
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

std::vector<ScoreRecord> LoadOldScores()
{

    // load old scores
    std::filesystem::path score_file_path = root_folder / "../resources/scoreboard.txt";
    std::fstream score_file(score_file_path, std::ios::in);
    std::string line;
    std::vector<ScoreRecord> old_scores;

    while (std::getline(score_file, line))
    {
        if (line.length() > 0)
        {
            // sore chars
            auto token_score = line.substr(0, line.find(','));
            line.erase(0, line.find(",") + 1);
            auto token_millis = line.substr(0, line.find(','));
            line.erase(0, line.find(",") + 1);

            // remove newline character
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
            auto token_name = line;

            ScoreRecord score_record = {
                (uint32_t)atoi(token_score.c_str()),
                (uint32_t)atoi(token_millis.c_str()),
                (char *)token_name.c_str()};

            old_scores.push_back(score_record);
        }
    }
    score_file.close();

    return old_scores;
}

void DisplayScoreboard()
{
    auto old_scores = LoadOldScores();
    int inc = 0;
    // get last recorded
    auto last_recorded = old_scores[old_scores.size() - 1];
    bool last_record_displayed = false;
    std::sort(old_scores.begin(), old_scores.end(), [](auto a, auto b)
              { return b.score < a.score; });
    for (size_t i = 0; i < 5; i++)
    {
        auto &score_record = old_scores[i];

        if (score_record == last_recorded)
        {
            char score_str[256];
            sprintf(score_str, "%s -- score : %d -- time : %d", score_record.player_name.c_str(), score_record.score, score_record.time_millis);
            attr_on(COLOR_PAIR(CLR_ALERT), (void *)0);
            mvprintw(inc + 1, 20, score_str);
            attr_off(COLOR_PAIR(CLR_ALERT), (void *)0);

            last_record_displayed = true;
        }
        else
        {
            char score_str[256];
            sprintf(score_str, "%s -- score : %d -- time : %d", score_record.player_name.c_str(), score_record.score, score_record.time_millis);
            attr_on(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
            mvprintw(inc + 1, 20, score_str);
            attr_off(COLOR_PAIR(CLR_STATUS_BAR), (void *)0);
        }

        inc++;
    }

    if (!last_record_displayed)
    {
        char score_str[256];
        sprintf(score_str, "%s -- score : %d -- time : %d", last_recorded.player_name.c_str(), last_recorded.score, last_recorded.time_millis);
        attr_on(COLOR_PAIR(CLR_ALERT), (void *)0);

        mvprintw(inc + 1, 20, "...");
        mvprintw(inc + 2, 20, score_str);
        attr_off(COLOR_PAIR(CLR_ALERT), (void *)0);
    }
}

void SaveScore()
{
    auto old_scores = LoadOldScores();

    std::filesystem::path score_file_path = root_folder / "../resources/scoreboard.txt";

    std::fstream score_out(score_file_path, std::ios::out);

    // rewrite old_scores
    for (auto &old_score_record : old_scores)
    {
        score_out << old_score_record;
    }
    // insert current score
    ScoreRecord cur_score = {score, (uint32_t)(timer.seconds * 1000.0), std::string(player_name)};
    score_out << cur_score;
    score_out.close();
}