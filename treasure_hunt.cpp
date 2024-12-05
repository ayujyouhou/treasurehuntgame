#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <cctype>

#define rep(i, n) for(int i = 0; i < (n); i++)

using namespace std;

// 定数定義
const int GRID_SIZE = 8;
const int NUM_TRAPS = 5;
const int NUM_CHALLENGES = 8;
const int MAX_TRAP_HITS = 3;
const int INITIAL_HEALTH = 80;

// プレイヤーの状態
enum PlayerStatus {
    PLAYING = 0,
    SUCCESS = 1,
    FAILURE = -1
};

// 座標構造体
struct Position {
    int x;
    int y;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// 問題構造体
struct Problem {
    string question;
    int answer;
};

// 関数プロトタイプ
Position generateRandomPosition();
bool isPositionUnique(const Position& pos, const vector<Position>& positions);
int calculateMinDistance(const Position& player, const Position& treasure);
bool areAllAdjacentTraps(const Position& treasure, const vector<Position>& traps);
vector<Position> getAdjacentPositions(const Position& pos);
void generateProblems(vector<Problem>& allProblems);
void displayGrid(const Position& player, const vector<vector<int>>& visited);

// メイン関数
int main() {
    // 乱数の初期化
    srand(static_cast<unsigned int>(time(0)));

    // ゲームの準備
    Position player = {0, 0};
    PlayerStatus status = PLAYING;
    int health = INITIAL_HEALTH;
    int trap_hits = 0;
    Position treasure;
    vector<Position> traps;
    vector<Position> challenges;
    vector<Problem> allProblems;
    vector<Problem> selectedProblems;

    // 問題の生成
    generateProblems(allProblems);
    if(allProblems.size() < NUM_CHALLENGES){
        cout << "十分な問題が用意されていません。\n";
        return 1;
    }

    // 宝と罠の生成（隣接マスがすべて罠にならないように）
    bool valid = false;
    int attempts = 0;
    const int MAX_ATTEMPTS = 1000;

    while(!valid && attempts < MAX_ATTEMPTS) {
        treasure = generateRandomPosition();
        traps.clear();
        // 罠の生成
        while(traps.size() < NUM_TRAPS) {
            Position trap = generateRandomPosition();
            if(!(trap == treasure) && isPositionUnique(trap, traps)) {
                traps.push_back(trap);
            }
        }
        // 宝の隣接マスがすべて罠でないことを確認
        if(!areAllAdjacentTraps(treasure, traps)) {
            valid = true;
        }
        attempts++;
    }

    if(!valid) {
        cout << "宝と罠の配置に失敗しました。再度実行してください。\n";
        return 1;
    }

    // チャレンジマスの生成
    attempts = 0;
    valid = false;
    while(!valid && attempts < MAX_ATTEMPTS) {
        challenges.clear();
        while(challenges.size() < NUM_CHALLENGES) {
            Position challenge = generateRandomPosition();
            // 宝、罠、既存のチャレンジマスと重複しないことを確認
            bool overlap = false;
            if(challenge == treasure) { overlap = true; }
            else {
                rep(i, traps.size()) {
                    if(challenge == traps[i]) {
                        overlap = true;
                        break;
                    }
                }
                if(!overlap) {
                    rep(i, challenges.size()) {
                        if(challenge == challenges[i]) {
                            overlap = true;
                            break;
                        }
                    }
                }
            }
            if(!overlap) {
                challenges.push_back(challenge);
            }
        }
        // チャレンジマスが正しく生成された
        if(challenges.size() == NUM_CHALLENGES) {
            valid = true;
        }
        attempts++;
    }

    if(!valid) {
        cout << "チャレンジマスの配置に失敗しました。再度実行してください。\n";
        return 1;
    }

    // 問題の選択（ランダムにNUM_CHALLENGES個選ぶ）
    vector<int> indices(allProblems.size());
    rep(i, allProblems.size()) { indices[i] = i; }
    random_shuffle(indices.begin(), indices.end());
    rep(i, NUM_CHALLENGES) {
        selectedProblems.push_back(allProblems[indices[i]]);
    }

    // 訪問履歴の初期化
    vector<vector<int>> visited(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    // スタート地点を訪問済みにする（安全なマス）
    visited[player.y][player.x] = 1;

    // ゲーム開始メッセージ
    cout << "宝探しゲームへようこそ！\n";
    cout << "操作方法:\n";
    cout << "  S: 左に1マス移動\n";
    cout << "  E: 上に1マス移動\n";
    cout << "  D: 右に1マス移動\n";
    cout << "  X: 下に1マス移動\n";
    cout << "  J: 罠探知モード\n";
    cout << "  H: 宝までの最短距離を表示\n";

    // 最初のグリッド表示
    displayGrid(player, visited);

    while(status == PLAYING) {
        cout << "\n現在位置: (" << player.x << ", " << player.y << ")\n";
        cout << "体力: " << health << "\n";
        cout << "操作を入力してください (S/E/D/X/J/H): ";
        char input;
        cin >> input;

        // 大文字小文字を区別しないように変換
        input = tolower(input);

        if(input == 's' || input == 'e' || input == 'd' || input == 'x') {
            // 移動処理
            Position newPos = player;
            if(input == 's') {
                newPos.x -= 1; // 左
            }
            else if(input == 'e') {
                newPos.y += 1; // 上
            }
            else if(input == 'd') {
                newPos.x += 1; // 右
            }
            else if(input == 'x') {
                newPos.y -= 1; // 下
            }

            // グリッド範囲内か確認
            if(newPos.x < 0 || newPos.x >= GRID_SIZE || newPos.y < 0 || newPos.y >= GRID_SIZE) {
                cout << "その方向には移動できません。別の操作を選んでください。\n";
                continue;
            }

            player = newPos;
            health -= 1;

            // スコープ修正: stepped_on_trap と stepped_on_challenge をここで宣言
            bool stepped_on_trap = false;
            bool stepped_on_challenge = false;
            int challenge_index = -1;

            // 罠のチェック
            rep(i, traps.size()) {
                if(player == traps[i]) {
                    trap_hits += 1;
                    cout << "罠にかかりました！ (" << trap_hits << "/" << MAX_TRAP_HITS << ")\n";
                    stepped_on_trap = true;
                    // 訪問履歴に罠マスとして記録
                    visited[player.y][player.x] = 2;
                    if(trap_hits >= MAX_TRAP_HITS) {
                        status = FAILURE;
                        cout << "罠に3回かかりました。ゲームオーバーです。\n";
                    }
                    break;
                }
            }

            if(!stepped_on_trap) {
                // チャレンジマスのチェック
                rep(i, challenges.size()) {
                    if(player == challenges[i]) {
                        stepped_on_challenge = true;
                        challenge_index = i;
                        break;
                    }
                }

                if(stepped_on_challenge) {
                    cout << "チャレンジマスに到達しました！問題を解いてください。\n";
                    // 訪問履歴にチャレンジマスとして記録
                    visited[player.y][player.x] = 3;

                    // 問題の取得
                    Problem currentProblem = selectedProblems[challenge_index];
                    bool solved = false;
                    while(!solved && health > 0) {
                        cout << "問題: " << currentProblem.question << "\n答え: ";
                        int userAnswer;
                        cin >> userAnswer;
                        if(userAnswer == currentProblem.answer) {
                            cout << "正解です！\n";
                            solved = true;
                        }
                        else {
                            cout << "間違えです。体力を1減らします。\n";
                            health -= 1;
                            if(health <= 0) {
                                status = FAILURE;
                                cout << "体力が尽きました。ゲームオーバーです。\n";
                                break;
                            }
                        }
                    }

                    // チャレンジを一度解決したらリストから削除
                    if(solved) {
                        challenges.erase(challenges.begin() + challenge_index);
                        selectedProblems.erase(selectedProblems.begin() + challenge_index);
                    }
                }
                else {
                    // 宝のチェック
                    if(player == treasure) {
                        status = SUCCESS;
                        cout << "おめでとうございます！宝を見つけました！\n";
                    }
                    else {
                        cout << "安全に移動しました。\n";
                        // 訪問履歴に安全なマスとして記録
                        visited[player.y][player.x] = 1;
                    }
                }
            }

            // 体力チェック（罠やチャレンジでなかった場合）
            if(health <= 0 && status != SUCCESS) {
                status = FAILURE;
                if(!stepped_on_trap && !stepped_on_challenge) { // 罠やチャレンジでなかった場合
                    cout << "体力が尽きました。ゲームオーバーです。\n";
                }
            }

            // グリッドの表示
            displayGrid(player, visited);

        }
        else if(input == 'j') {
            // 罠探知モード
            cout << "罠探知モードです。方向を入力してください (S: 左, E: 上, D: 右, X: 下): ";
            char direction;
            cin >> direction;
            direction = tolower(direction);
            Position checkPos = player;
            if(direction == 's') {
                checkPos.x -= 1; // 左
            }
            else if(direction == 'e') {
                checkPos.y += 1; // 上
            }
            else if(direction == 'd') {
                checkPos.x += 1; // 右
            }
            else if(direction == 'x') {
                checkPos.y -= 1; // 下
            }
            else {
                cout << "無効な方向です。\n";
                continue;
            }

            // グリッド範囲内か確認
            if(checkPos.x < 0 || checkPos.x >= GRID_SIZE || checkPos.y < 0 || checkPos.y >= GRID_SIZE) {
                cout << "その方向にはマスがありません。\n";
                continue;
            }

            // 罠のチェック
            bool isTrap = false;
            rep(i, traps.size()) {
                if(checkPos == traps[i]) {
                    isTrap = true;
                    break;
                }
            }

            if(isTrap) {
                cout << "そのマスには罠があります。\n";
            }
            else {
                cout << "そのマスには罠はありません。\n";
            }

            health -=1;
            // 体力チェック
            if(health <= 0 && status != SUCCESS) {
                status = FAILURE;
                cout << "体力が尽きました。ゲームオーバーです。\n";
            }

            // グリッドの表示
            displayGrid(player, visited);

        }
        else if(input == 'h') {
            // 最短距離表示
            int min_dist = calculateMinDistance(player, treasure);
            cout << "宝までの最短距離は " << min_dist << " マスです。\n";
            health -=1;
            // 体力チェック
            if(health <= 0 && status != SUCCESS) {
                status = FAILURE;
                cout << "体力が尽きました。ゲームオーバーです。\n";
            }

            // グリッドの表示
            displayGrid(player, visited);
        }
        else {
            cout << "無効な入力です。もう一度入力してください。\n";
            continue;
        }
    }

    if(status == FAILURE) {
        cout << "残念ながら失敗しました。\n";
    }

    cout << "ゲームを終了します。\n";
    // 最終グリッドの表示
    displayGrid(player, visited);
    return 0;
}

// ランダムな位置を生成する関数
Position generateRandomPosition() {
    Position pos;
    pos.x = rand() % GRID_SIZE;
    pos.y = rand() % GRID_SIZE;
    return pos;
}

// 位置が既にリストに存在するか確認する関数
bool isPositionUnique(const Position& pos, const vector<Position>& positions) {
    rep(i, positions.size()) {
        if(pos == positions[i]) {
            return false;
        }
    }
    return true;
}

// プレイヤーから宝までの最短距離を計算する関数（マンハッタン距離）
int calculateMinDistance(const Position& player, const Position& treasure) {
    return abs(player.x - treasure.x) + abs(player.y - treasure.y);
}

// 宝の隣接するマスがすべて罠かどうかを確認する関数
bool areAllAdjacentTraps(const Position& treasure, const vector<Position>& traps) {
    vector<Position> adjacent = getAdjacentPositions(treasure);
    rep(i, adjacent.size()) {
        bool isTrap = false;
        rep(j, traps.size()) {
            if(adjacent[i] == traps[j]) {
                isTrap = true;
                break;
            }
        }
        if(!isTrap) {
            return false; // 少なくとも1つは罠でない
        }
    }
    return true; // すべて罠
}

// 指定された位置の隣接するマスを取得する関数
vector<Position> getAdjacentPositions(const Position& pos) {
    vector<Position> adj;
    // 左
    if(pos.x - 1 >= 0) {
        adj.push_back(Position{pos.x - 1, pos.y});
    }
    // 右
    if(pos.x + 1 < GRID_SIZE) {
        adj.push_back(Position{pos.x + 1, pos.y});
    }
    // 上
    if(pos.y + 1 < GRID_SIZE) {
        adj.push_back(Position{pos.x, pos.y + 1});
    }
    // 下
    if(pos.y - 1 >= 0) {
        adj.push_back(Position{pos.x, pos.y - 1});
    }
    return adj;
}

// 問題を生成する関数
void generateProblems(vector<Problem>& allProblems) {
    // ここでは例として20個の簡単な数学の問題を用意しています。
    // 実際には、必要に応じて問題を追加または変更してください。
    allProblems.push_back(Problem{"9×9 のマス目があり，上から i 行目，左から j 列目のマスには整数 i×j が書き込まれています．ここから相異なる n 個のマスを選び，選んだマスすべてに書かれた数字を合計すると立方数となりました．このような正整数 n としてありうる最大値を求めてください．", 76});
    allProblems.push_back(Problem{"　A さん，B さん，C さんは，ある池の周りを常に一定の速さで移動し，池の周りを一周するのにそれぞれ 3 分， 5 分， 7 分かかります．3 人が池の周りの同じ地点から同時にスタートして同じ向きに移動するとき，x 分後に 3 人全員が同時にスタート地点ではないある地点で初めて出会いました．x は互いに素な正整数 a,b を用いて a/b​ と表されるので a+b の値を解答してください", 107});
    allProblems.push_back(Problem{"n 番目に小さい素数が 2n−1 であるような，正整数 n の総和を解答して下さい", 9});
    allProblems.push_back(Problem{"三角形 ABC において，A に対する傍心を I​ とします．∠BAC=6∘ のとき，∠BIC∠BI​C の大きさを度数法で求めて下さい", 87});
    allProblems.push_back(Problem{"　正の実数 a,b,c,dがab=10,bc=20,cd=30をみたしているとき，ad の値を求めてください．", 15});
    allProblems.push_back(Problem{"ある長方形について，その周長が 8sqrt6​，面積が 10 のとき，対角線の長さの 2 乗を求めてください", 76});
    allProblems.push_back(Problem{"フランス革命が起きたのは西暦何年？", 1789});
    allProblems.push_back(Problem{"大政奉還は西暦何年に行われた？", 1867});
    // allProblems.push_back(Problem{"5 + 7", 12});
    // allProblems.push_back(Problem{"9 - 3", 6});
    // allProblems.push_back(Problem{"4 * 6", 24});
    // allProblems.push_back(Problem{"20 / 4", 5});
    // allProblems.push_back(Problem{"15 + 10", 25});
    // allProblems.push_back(Problem{"18 - 9", 9});
    // allProblems.push_back(Problem{"7 * 5", 35});
    // allProblems.push_back(Problem{"16 / 2", 8});
    // allProblems.push_back(Problem{"3 + 14", 17});
    // allProblems.push_back(Problem{"12 - 4", 8});
    // allProblems.push_back(Problem{"6 * 7", 42});
    // allProblems.push_back(Problem{"24 / 3", 8});
    // allProblems.push_back(Problem{"8 + 9", 17});
    // allProblems.push_back(Problem{"14 - 5", 9});
    // allProblems.push_back(Problem{"5 * 8", 40});
    // allProblems.push_back(Problem{"30 / 5", 6});
    // allProblems.push_back(Problem{"11 + 6", 17});
    // allProblems.push_back(Problem{"19 - 7", 12});
    // allProblems.push_back(Problem{"9 * 4", 36});
    // allProblems.push_back(Problem{"28 / 4", 7});
}

// グリッドを表示する関数
void displayGrid(const Position& player, const vector<vector<int>>& visited) {
    cout << "\n現在のグリッド:\n";
    // ヘッダー
    cout << "   ";
    rep(x, GRID_SIZE) {
        cout << x << " ";
    }
    cout << "\n";

    rep(y, GRID_SIZE) {
        // 行のヘッダー
        cout << y << " | ";
        rep(x, GRID_SIZE) {
            if(player.x == x && player.y == y) {
                cout << "P ";
            }
            else {
                switch(visited[y][x]) { // 修正: visited[y][x] に変更
                    case 1:
                        cout << "S "; // 安全
                        break;
                    case 2:
                        cout << "T "; // 罠
                        break;
                    case 3:
                        cout << "C "; // チャレンジ
                        break;
                    default:
                        cout << "□ "; // 未訪問
                        break;
                }
            }
        }
        cout << "\n";
    }
}
