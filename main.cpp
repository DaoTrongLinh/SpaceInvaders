#include <iostream>
#include <sstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "graphics.h"
#include "defs.h"
#include "game.h"

using namespace std;

int main(int argc, char *argv[])
{
    //graphics để khởi tạo SDL,SDL_image,... và render hình ảnh
    Graphics graphics;
    graphics.init();

    //Tải nhạc
    Mix_Music *gMusic = graphics.loadMusic("Pufino - Enjoy.mp3");
    graphics.play(gMusic);
    //Mix_Chunk *gJump = graphics.loadSound("whoosh.mp3");

    //Nền cuộn
    ScrollingBackground background;
    background.setTexture(graphics.loadTexture(BACKGROUND_IMG));

    //Nhân vật
    Mouse mouse;
    mouse.x = (SCREEN_WIDTH / 2)-350;
    mouse.y = (SCREEN_HEIGHT / 2)-130;

    //Ảnh động nhân vật->đè lên nhân vật
    Sprite character;
    SDL_Texture* characterTexture = graphics.loadTexture(CHARACTER_SPRITE_FILE);
    character.init(characterTexture, CHARACTER_FRAMES, CHARACTER_CLIPS);

    //Kẻ thù
    SDL_Texture* enemyTexture = graphics.loadTexture("enemy.png");
    vector<Enemy> enemies;

    //Ảnh đạn
    SDL_Texture* bulletTexture = graphics.loadTexture("bullet.png");
    //Không khai báo vector<Bullet> bullets vì bullets sẽ được gọi trong struct Mouse

    //Logic game: vẽ nhân vật, kẻ thù, điểm số, va chạm,...
    Game game;

    //Font chữ cho hiển thị điểm số, game over, start,... ra màn hình
    TTF_Font* font = graphics.loadFont("Pixel Game.otf", 50);
    SDL_Color color = {255, 255, 0, 255};

    //Phần mở đầu
    SDL_Texture* startText = graphics.renderText("Press SPACE to start", font, color);
    int tw, th;
    SDL_QueryTexture(startText, NULL, NULL, &tw, &th);
    //Căn chỉnh dòng
    int tx = (SCREEN_WIDTH - tw) / 2 ;
    int ty = ((SCREEN_HEIGHT - th) / 2) - 10;

    bool startGame = false;
    bool quit = false;
    SDL_Event e;

    while (!startGame && !quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                startGame = true;
            }
        }

        //Vẽ nền và dòng chữ
        graphics.prepareScene();
        background.scroll(1); //Nền cuộn
        graphics.render(background);
        graphics.renderTexture(startText, tx, ty);
        graphics.presentScene();
        SDL_Delay(16);
    }
    SDL_DestroyTexture(startText);


    //Game loop
    while( !quit && !gameOver(mouse, enemies, game)) {
        while( SDL_PollEvent(&e) ) {
            if(e.type == SDL_QUIT) quit = true;
        }

        //Render background
        background.scroll(1);
        graphics.render(background);

        //Event bàn phím
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        mouse.turn_South();
        if (currentKeyStates[SDL_SCANCODE_UP]) mouse.turnNorth();
        if (currentKeyStates[SDL_SCANCODE_DOWN]) mouse.turnSouth();
        //if (currentKeyStates[SDL_SCANCODE_UP]) graphics.play(gJump); //Mỗi khi nhấn phím thì tạo tiếng
        if (currentKeyStates[SDL_SCANCODE_SPACE]) mouse.shoot(bulletTexture); //Bắn ra đạn
        mouse.updateBullets();

        //Tạo thêm kẻ thù ngẫu nhiên
        if (rand() % 100 < 5) { //Khoảng 5% khả năng mỗi khung hình
            game.spawnEnemy(enemies, enemyTexture);
        }

        //Kiểm tra va chạm giữa đạn và kẻ thù
        for (auto& bullet : mouse.bullets) {
            for (auto& enemy : enemies) {
                if (game.checkBulletEnemyCollision(bullet, enemy)) {
                    break;  //Nếu đạn va chạm, không cần kiểm tra các kẻ thù khác
                }
            }
        }

        //Kiểm tra va chạm giữa nhân vật và kẻ thù
        for (auto& enemy : enemies) {
            if (game.checkPlayerEnemyCollision(mouse, enemy)) {
                quit=true; //Nếu va chạm thì game over
            }
            //Render kẻ thù
            enemy.move();
            enemy.render(graphics);
        }

        //Render nhân vật
        character.tick();
        mouse.move();
        //render(mouse, graphics); //Render ô vuông xanh lá nhỏ nằm ở dưới ảnh sprite của nhân vật
        graphics.renderCharacter(mouse.x-76, mouse.y-67 , character);

        //Hiển thị điểm số
        graphics.renderScore(game.score, font, color, 10, 10); //Vị trí hiển thị điểm số góc trái trên

        graphics.presentScene();
        SDL_Delay(16);
    }

    if (gMusic != nullptr) Mix_FreeMusic( gMusic );
    //if (gJump != nullptr) Mix_FreeChunk( gJump);

    //Phần kết thúc
    if (gameOver(mouse, enemies, game)) {
    //Dòng "Game Over!"
    SDL_Color red = {255, 0, 0, 255};
    SDL_Texture* gameOverText = graphics.renderText("Game Over!", font, red);

    //Dòng điểm số
    stringstream s;
    s <<"Your score: "<< game.score;
    string Score = s.str();
    SDL_Texture* finalScoreText = graphics.renderText(Score.c_str(), font, red); //Phải thêm c_str() vì nó là const char*

    //Kích thước của từng dòng text
    int gw, gh, sw, sh;
    SDL_QueryTexture(gameOverText, NULL, NULL, &gw, &gh);
    SDL_QueryTexture(finalScoreText, NULL, NULL, &sw, &sh);

    //Căn giữa cả hai dòng, dòng thứ hai nằm dưới dòng đầu 10 pixel
    int cx = SCREEN_WIDTH / 2;
    int gy = (SCREEN_HEIGHT - gh - sh - 10) / 2;

    //Dòng Game Over với nền là background game
    graphics.prepareScene();
    graphics.render(background);

    //Render dòng "Game over" và dòng điểm số
    graphics.renderTexture(gameOverText, cx - gw / 2, gy);
    graphics.renderTexture(finalScoreText, cx - sw / 2, gy + gh + 10);
    graphics.presentScene();

    SDL_Delay(3000); //Giữ lại màn hình 3 giây

    SDL_DestroyTexture(gameOverText);
    SDL_DestroyTexture(finalScoreText);
}

    SDL_DestroyTexture(characterTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(background.texture);
    SDL_DestroyTexture(bulletTexture);
    characterTexture = nullptr;
    enemyTexture = nullptr;
    bulletTexture = nullptr;
    TTF_CloseFont(font);

    graphics.quit();
    return 0;
}
