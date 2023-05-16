#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include<pthread.h>
#include<semaphore.h>

#define SCREEN_WIDTH 728
#define SCREEN_HEIGHT 455
#define PLANE_SIZE 5
#define COLLISION_DISTANCE 20
#define WAIT_TIME 2000

sem_t semaphore;

void* airplane_threads(void* arg) {

    sem_wait(&semaphore);
    float distance; //= SDL_sqrt(SDL_pow(plane2.position.x - plane1.position.x, 2) + SDL_pow(plane2.position.y , 2));
  //  return distance - COLLISION_DISTANCE;
    sem_post(&semaphore);

    return NULL;
}



typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    Point start;
    Point end;
} Line;

typedef struct {
    Point position;
    Line path;
    float progress;
    bool waiting;
    Uint32 waiting_time;
} Plane;

int xl(int i){
//SDL_Rect rect = {(int)position.x - PLANE_SIZE / 2, (int)position.y - PLANE_SIZE / 2, PLANE_SIZE, PLANE_SIZE};
return i;
}

void draw_line(SDL_Renderer* renderer, Line line) {
    SDL_RenderDrawLine(renderer, (int)line.start.x, (int)line.start.y, (int)line.end.x, (int)line.end.y);
}

void draw_plane(SDL_Renderer* renderer, Point position) {
    SDL_Rect rect = {(int)position.x - PLANE_SIZE / 2, (int)position.y - PLANE_SIZE / 2, PLANE_SIZE, PLANE_SIZE};
    SDL_RenderFillRect(renderer, &rect);
}

void draw_bg(SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void *drawplane(void* arg);

bool check_collision(Plane plane1, Plane plane2) {
    float distance = SDL_sqrt(SDL_pow(plane2.position.x - plane1.position.x, 2) + SDL_pow(plane2.position.y - plane1.position.y, 2));
    return distance < COLLISION_DISTANCE;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    // Create a window
    SDL_Window* window = SDL_CreateWindow(
        "ATC Simulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface* surface = IMG_Load("map.jpg");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    // Create some planes
    Plane planes[] = {
        // pak-uk
        {
            {460, 200},
            {{460, 200}, {330, 150}},
            0,
            false
        },
        // can--afr
        {
            {150, 150},
            {{150, 150}, {350, 230}},
            0,
            false
        },
        // pak-ind
        {
            {460, 200},
            {{460, 200}, {482, 220}},
            0,
            false
        },
        // eur-uk
        {
            {400, 130},
            {{400, 130}, {330, 150}},
            0,
            false
        },
        // rus-ind
        {
            {500, 120},
            {{500, 120}, {482, 220}},
            0,
            false
        },
        // rus-eur
        {
            {500, 160},
            {{500, 120}, {400, 130}},
            0,
            false
        },
        // can--us
        {
            {150, 150},
            {{150, 150}, {145, 185}},
            0,
            false
        },
        // china-jap
        {
            {480, 180},
            {{480, 180}, {587, 193}},
            0,
            false
        },
        // safr to afr
        {
            {390, 310},
            {{390, 310}, {350, 230}},
            0,
            false
        },
        // samr--us
        {
            {240, 300},
            {{240, 300}, {145, 185}},
            0,
            false
        },
        // samr--uk
        {
            {240, 300},
            {{240, 300}, {330, 150}},
            0,
            false
        },
        // china-pak
        {
            {480, 180},
            {{480, 180}, {460, 200}},
            0,
            false
        },
        // pak-afr
        {
            {460, 200},
            {{460, 200}, {350, 230}},
            0,
            false
        },
        // ind - jap
        {
            {482, 220},
            {{482, 220}, {587, 193}},
            0,
            false
        },
        // aus - jap
        {
            {580, 300},
            {{580, 300}, {587, 193}},
            0,
            false
        },
        // aus - ind
        {
            {580, 300},
            {{580, 300}, {482, 220}},
            0,
            false
        }
    };
    int num_planes = sizeof(planes) / sizeof(planes[0]);

    // Start the simulation loop
    Uint32 last_frame_time = SDL_GetTicks();
    while (1) {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // Quit the program if the window is closed
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
        }

        // Calculate the time since the last frame
        Uint32 current_time = SDL_GetTicks();
        Uint32 elapsed_time = current_time - last_frame_time;
        last_frame_time = current_time;

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_bg(renderer, texture);

        // Draw the lines
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
        for (int i = 0; i < num_planes; i++) {
            draw_line(renderer, planes[i].path);
        }

	//--------THREADS----------
	
	pthread_t p[num_planes];
	
	for(int i=0;i<num_planes;i++){
		pthread_create(&p[i],NULL,drawplane,&i);
	}


        // Update and draw the planes
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < num_planes; i++) {
            // Skip the plane if it is already waiting
            if (planes[i].waiting) {
            if(planes[i].waiting_time>0){
            planes[i].waiting_time-=elapsed_time;
            }else{
            planes[i].waiting=false;
            }
            draw_plane(renderer,planes[i].position);
            continue;
            }
            
            // Update the plane's position
            float distance = elapsed_time / 1000.0 * 25;  // 50 pixels per second
            planes[i].progress += distance / SDL_sqrt(SDL_pow(planes[i].path.end.x - planes[i].path.start.x, 2) + SDL_pow(planes[i].path.end.y - planes[i].path.start.y, 2));
            if (planes[i].progress > 1) {
                planes[i].progress = 0;
                Point tmp = planes[i].path.start;
                planes[i].path.start = planes[i].path.end;
                planes[i].path.end = tmp;
            }
            planes[i].position.x = planes[i].path.start.x + planes[i].progress * (planes[i].path.end.x - planes[i].path.start.x);
            planes[i].position.y = planes[i].path.start.y + planes[i].progress * (planes[i].path.end.y - planes[i].path.start.y);

            // Check for collisions with other planes
            for (int j = 0; j < num_planes; j++) {
                if (i != j && !planes[j].waiting && check_collision(planes[i], planes[j])) {
                    // Plane collision detected, make one of them wait
                    planes[i].waiting = true;
                    planes[i].waiting_time=WAIT_TIME;
                    planes[i].progress = 0;
                    //SDL_Delay(WAIT_TIME);
                    //planes[i].waiting = false;
                    break;
                }
            }

            // Draw the plane
            draw_plane(renderer, planes[i].position);
        }

        // Present the rendered image to the screen
        SDL_RenderPresent(renderer);
    }
}














void *drawplane(void* arg){
int i =*(int*)arg;
int x = xl(i);
int y = xl(i);
}
