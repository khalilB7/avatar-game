#include "perso.h"

void initPerso(Personne *p, SDL_Renderer *renderer) {
    char *paths[17] = {"./image/jou.png", "./image/jou1.png", /* ... others ... */};
    for (int i = 0; i < 17; i++) {
        SDL_Surface *temp = IMG_Load(paths[i]);
        p->images[i] = SDL_CreateTextureFromSurface(renderer, temp);
        if (i == 0) { p->position.w = temp->w; p->position.h = temp->h; }
        SDL_FreeSurface(temp);
    }
    p->healthpoints = 3;
}

void afficherPersoWithCamera(Personne p, SDL_Renderer *renderer, SDL_Rect camera) {
    SDL_Rect pos = p.position;
    pos.x -= camera.x;
    pos.y -= camera.y;
    
    int imgIdx = p.current_image;
    if (p.is_attacking) imgIdx = (p.direction >= 0) ? 14 : 15;
    
    SDL_RenderCopy(renderer, p.images[imgIdx], NULL, &pos);
}

void updateCamera(SDL_Rect *camera, Personne p, int level_width) {
    camera->x = (p.position.x + (p.position.w / 2)) - (SCREEN_W / 2);
    if (camera->x < 0) camera->x = 0;
    if (camera->x > level_width - camera->w) camera->x = level_width - camera->w;
}