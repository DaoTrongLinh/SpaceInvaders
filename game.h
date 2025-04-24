#ifndef GAME_H
#define GAME_H

#define INITIAL_SPEED 3

#include<vector>
#include<algorithm>
#include<cstdlib>
using namespace std;

struct Bullet {
    int x, y;
    int speed = 5;
    int direction; //Hướng của viên đạn: direction=1 thì đạn bắn sang phải; direction =-1 thì đạn bắn sang trái
    bool active = true; //Trạng thái hoạt động của đạn
    SDL_Texture* texture;

    Bullet(int startX, int startY, int dir, SDL_Texture* tex) {
        x = startX + 80; //Vị trí đạn spawn-phải cộng 80 pixel để trông như đang bắn ra từ mỏ con chim
        y = startY + 30; //Vị trí đạn spawn
        direction=dir;
        texture = tex; //Đè ảnh đạn lên đạn
    }

    void move() {
        x += speed*direction;
        if (x < 0|| x > SCREEN_WIDTH) active = false;
    }
};

struct Mouse {
    int x, y;
    int dx = 0, dy = 0;
    float speed = INITIAL_SPEED;
    int huong=1;
    vector<Bullet> bullets;
    unsigned int LanBanCuoi=0;
    const unsigned int ShotCoolDown=300; //Khoảng thời gian phải chờ giữa các lần bắn (ms)

    //Render đạn
    void shoot(SDL_Texture* bulletTexture) {
        //Delay sau mỗi lần bắn ra 1 viên đạn
        unsigned int currentTime = SDL_GetTicks(); //Lấy thời gian(số ms trôi qua kể từ khi bắt đầu chạy chương trình)
        if (currentTime - LanBanCuoi >= ShotCoolDown) {
            bullets.push_back(Bullet(x, y, huong, bulletTexture)); //Tạo viên đạn mới
            LanBanCuoi = currentTime;
        }
    }
    //Xóa đạn sau khi va chạm với kẻ thù hoặc sau khi ra khỏi màn hình
    void updateBullets() {
        for (auto &bullet : bullets) bullet.move(); //Duyệt từng viên đạn trong danh sách đạn
        bullets.erase(remove_if(bullets.begin(), bullets.end(),
                        [](const Bullet &b) {return !b.active;}), //Hàm lambda không tên, dùng luôn: kiểm tra xem đạn còn hoạt động không
                      bullets.end()); //Đưa những viên đạn không còn hoạt động về cuối danh sách đạn
    }

    //Di chuyển nhân vật
    void move() {
        y += dy;
    }
    void turnNorth() {
        dy = -speed;
        dx = 0;
    }
    void turnSouth() {
        dy = speed;
        dx = 0;
    }
    void turnWest() {
        dy = 0;
        dx = -speed;
    }
    void turnEast() {
        dy = 0;
        dx = speed;
    }
    void turn_South() { //Trọng lực
        dy = speed-1.5;
        dx = 0;
    }
};

struct Enemy {
    int x, y, speed;
    SDL_Texture* texture;

    //Kẻ thù di chuyển
    void move() {
        x -= speed; //Chạy sang trái
    }

    //Render kẻ thù
    void render(Graphics& graphics) {
        SDL_Rect dest = {x, y, 50, 50}; //Kích thước 50x50
        SDL_RenderCopy(graphics.renderer, texture, NULL, &dest);
    }
};

struct Game {
    int score = 0;

    //Tạo kẻ thù ngẫu nhiên
    void spawnEnemy(vector<Enemy>& enemies, SDL_Texture* enemyTexture) {
        int TocDo=rand()%5+2; //Tốc độ ngẫu nhiên của kẻ thù
        int y = rand() % (SCREEN_HEIGHT - 50); //Vị trí y ngẫu nhiên
        enemies.push_back({SCREEN_WIDTH, y, TocDo, enemyTexture}); //Thêm nhiều kẻ thù nữa
    }

    //Kiểm tra va chạm giữa đạn và kẻ thù
    bool checkBulletEnemyCollision(Bullet& bullet, Enemy& enemy) {
        SDL_Rect bulletRect = {bullet.x, bullet.y, 20, 20}; //Kích thước đạn
        SDL_Rect enemyRect = {enemy.x, enemy.y, 50, 50}; //Kích thước kẻ thù

        if (SDL_HasIntersection(&bulletRect, &enemyRect)) {
            bullet.active = false; //Đạn bị hủy
            enemy.x = -100; //Xóa kẻ thù (đưa nó ra khỏi màn hình)
            score += 10; //Tăng điểm
            return true;
        }
        return false;
    }

    //Kiểm tra va chạm giữa nhân vật và kẻ thù
    bool checkPlayerEnemyCollision(const Mouse& mouse, const Enemy& enemy) {
        SDL_Rect playerRect = {mouse.x, mouse.y, 65, 62}; //Kích thước nhân vật
        SDL_Rect enemyRect = {enemy.x, enemy.y, 50, 50}; //Kích thước kẻ thù

        return SDL_HasIntersection(&playerRect, &enemyRect);
    }
};


void render(const Mouse& mouse, const Graphics& graphics) {
    SDL_Rect filled_rect;
    filled_rect.x = mouse.x;
    filled_rect.y = mouse.y;
    filled_rect.w = 0;
    filled_rect.h = 0;
    SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(graphics.renderer, &filled_rect);

    //SDL_SetRenderDrawColor(graphics.renderer, 255, 0, 0, 255); //Đạn màu đỏ (dòng này cần khi không có ảnh đạn để đè lên thì sẽ render là ô chữ nhật màu đỏ)
    for (const auto& bullet : mouse.bullets) {
        SDL_Rect bullet_rect = {bullet.x, bullet.y, 20, 20}; //Kích thước đạn
        SDL_RenderCopy(graphics.renderer, bullet.texture, NULL, &bullet_rect);
    }
}

//Điều kiện game over
bool gameOver(const Mouse& mouse, const vector<Enemy>& enemies, Game& game) {
    //Kiểm tra nếu nhân vật đi ra khỏi màn hình
    if (mouse.x < 0 || mouse.x >= SCREEN_WIDTH ||
        mouse.y < 10 || mouse.y >= 640) {
        return true;
    }

    //Kiểm tra va chạm với từng kẻ thù
    for (const auto& enemy : enemies) {
        if (game.checkPlayerEnemyCollision(mouse, enemy)) {
            return true; //Game over nếu có va chạm
        }
    }

    return false; //Không có va chạm và vẫn trong màn hình

}

#endif //GAME_H
