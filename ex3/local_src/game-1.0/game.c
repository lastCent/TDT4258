#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/fb.h>


static void tick();
static void game_init();
static void move_ball();
static void draw_map();

struct fb_copyarea rect;

int p_offset[80]; // length of each player
int width; // width of each player
int player_move_mult; // how much each player moves in the y direnction
int y_player1; // position of player 1 in the mmap
int y_player2; // position of player 2 in the mmap
int ball_position; // position of ball in mmap
int old_ball_position; // ball position before movement, used to erase from  fb
int ball_dimensions[2];  //ind0 = width, ind1 = height
// the direction the ball is taking based on origo, where {x = 1, y = 1} goes towards upper right north-east
// and the value is how many indexes it moves
int ball_direction[2] = {-10, -10};
int fbfd; // framebuffer var
int gamepad_driver; // gamepad var
int sound_driver; //driver for TIMER3 and DAC

uint16_t *map; // mmap var
int interrupt_enabled; // issues where interrupt is called before setup is done == error, this prevents it
int game_over; // variable to keep track of game
int player_win[2]; // index = 0 == player 1, index = 1 == player 2

static void game_init(){
	width = 8;
	y_player1 = 50 + 320 * 60;
	y_player2 = 270 + 320 * 60;
	player_win[0] = 0;
	player_win[1] = 0;
	ball_position = 12920;  // 38560 approx middle
	ball_dimensions[0] = 12; // Ball widht
	ball_dimensions[1] = 12; // Ball height
	player_move_mult = 15;
	game_over = 0;
	// put variables in the length array
	int i;
	for(i = 0; i < (sizeof(p_offset) / sizeof(*p_offset)); i++){
		p_offset[i] = 320 * i;
	}
	// black the screen
	for (i = 0; i < 320*240; i++){
		map[i] = 0x0;
	}
	rect.dx = 0;
	rect.dy = 0;
	rect.width = 320;
	rect.height = 240;
	ioctl(fbfd, 0x4680, &rect);
}

static void GPIO_interrupt_handler(int unused)
{
	int buff = 0;
	// read interrupt message only if interrupts are enabled and game is not over
	if (interrupt_enabled && !game_over){
		read(gamepad_driver, &buff, sizeof(int));
	}
	
	// check if button 2 is pressed
	if ((~buff & 0b10) == 0b10){
		// check if the new movement goes out of bound
		if (y_player1 - 320 * player_move_mult > 0){
			y_player1 -= 320 * player_move_mult;
		}	
	}
	
	// check if button 4 is pressed
	if ((~buff & 0b1000) == 0b1000){
		// check if new movement goes out of bound
		if (y_player1 + 320 * (sizeof(p_offset) / sizeof(*p_offset)) + (320 * player_move_mult) <= 76800 + 320 * player_move_mult){
			y_player1 += 320 * player_move_mult;
		}	
	
	}
	
	// check if button 6 is pressed
	if((~buff & 0b100000) == 0b100000){
	 	// check if new movement goes out of bound
		if (y_player2 - 320 * player_move_mult > 0){
			y_player2 -= 320 * player_move_mult;
		}	
	}
	
	// check if button 8 is pressed
	if((~buff & 0b10000000) == 0b10000000){
		// check if new movement goes out of bound
		if (y_player2 + 320 * (sizeof(p_offset) / sizeof(*p_offset)) + (320 * player_move_mult) <= 76800 + 320 * player_move_mult){
			y_player2 += 320 * player_move_mult;
		}	
	}
}

int main(int argc, char *argv[])
{
	// setup of drivers
	struct sigaction GPIO_action = {}; 
	GPIO_action.sa_handler = &GPIO_interrupt_handler; 
	sigaction(SIGUSR1, &GPIO_action, NULL);
	char command1[50];
	strcpy(command1, "modprobe driver-gamepad");
	system(command1);
	char command2[50];
	strcpy(command2, "modprobe driver-sound");
	system(command2);
	
	gamepad_driver = open("/dev/Gamepad", O_RDWR, (mode_t)0600);
	sound_driver = open("/dev/SOUND", O_RDWR, (mode_t)0600);
	
	// enable interrupt as setup is ready
	interrupt_enabled = 1;
	
	// setup mmap
	fbfd = open("/dev/fb0", O_RDWR, (mode_t)0600);
	map = (uint16_t*)mmap(NULL, 320*240*2, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	
	// setup game
	game_init();
	
	// time vars for "wait"
	struct timeval t0;
	struct timeval t1;
	float elapsed;
	
	// main loop
	while (1) {
		gettimeofday(&t0, 0);
		while(1){
			gettimeofday(&t1, 0);
			elapsed = (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
			// check elapsed time, and break if more than cond. this is in milliseconds
			if(elapsed > 80){
				break;
			}
			
		}
		// if game not over, tick/update
		if (!game_over){
			tick();
		}else{
			// game is over, therefore wait for 3 seconds and then reset game
			write(sound_driver, 0, 5); // Play end tune
			gettimeofday(&t0, 0);
			while(1){
				gettimeofday(&t1, 0);
				elapsed = (t1.tv_sec - t0.tv_sec);
				if(elapsed > 3){
					game_init();
					break;
				}
			}
		}
	}
	exit(EXIT_SUCCESS);
}

static void tick(){
	move_ball();
	draw_map();	
}

static void draw_map(){
	int i;  // C doesnt like it in the loop declaration =(
	int j;
	
	// Turn screen black
	for (i = 0; i < 320*240; i++){
		map[i] = 0x0;
	}
	
	// remove old ball
	rect.dx = old_ball_position % 320;
	rect.dy = old_ball_position / 320;
	rect.width = ball_dimensions[0];
	rect.height = ball_dimensions[1];
	ioctl(fbfd, 0x4680, &rect);

	// Add color for the players
	for(i = 0; i < width; i++){
		for(j = 0; j < (sizeof(p_offset) / sizeof(*p_offset)); j++){
			if (player_win[0]){
				// player 1 won, player1 = green, player2 = red
				map[y_player1 + i + p_offset[j]] = 0x07E0;
				map[y_player2 + i + p_offset[j]] = 0x7800;
			}else if (player_win[1]){
				// player 2 won, player1 = red, player 2, green
				map[y_player1 + i + p_offset[j]] = 0x7800;
				map[y_player2 + i + p_offset[j]] = 0x07E0;
			}else{
				// no one won yet, keep em white
				map[y_player1 + i + p_offset[j]] = 0xffff;
				map[y_player2 + i + p_offset[j]] = 0xffff;
			}
		}
	}
	// Add whitie for the ball
	for(i = 0; i <= ball_dimensions[0]; i++){
		for(j = 0; j <= ball_dimensions[1]; j++){
			map[ball_position + i + 320 * j] = 0xffff;
		}
	}
	
	// Draw player1
	rect.dx = y_player1 % 320;
	rect.dy = 0;
	rect.width = width;
	rect.height = 240;
	ioctl(fbfd, 0x4680, &rect);
	
	// draw player2
	rect.dx = y_player2 % 320;
	rect.dy = 0;
	rect.width = width;
	rect.height = 240;
	ioctl(fbfd, 0x4680, &rect);
	
	// draw ball
	rect.dx = ball_position % 320;
	rect.dy = ball_position / 320;
	rect.width = ball_dimensions[0];
	rect.height = ball_dimensions[1];
	ioctl(fbfd, 0x4680, &rect);
}
static void move_ball(){

	// check roof/floor collision: check if the left bottom corner and top corner 
	// exceeds or is less than the roof or floor
	int temp_ball_pos_roof = ball_position + 360 * 5;
	temp_ball_pos_roof -= ball_direction[0] + ball_direction[1] * 320;
	int temp_ball_pos_floor = temp_ball_pos_roof + 320 * (ball_dimensions[1] - 5);
	if ((temp_ball_pos_roof < 320) || (temp_ball_pos_floor > 76800)){
		write(sound_driver, 0, 3); // play bounce sound
		// change direction of y value, ie direction from up to down, vice-versa
		ball_direction[1] *= -1;
		
		// set current ball pos to old ball pos
		old_ball_position = ball_position;
		
		// set new ball pos
		ball_position -= ball_direction[0] + ball_direction[1] * 320;
		return;
	}
	
	// check player collision: check all top parts of the ball and all bottom parts of the ball to see if
	// any of those points collide with the players
	int i;
	int j;
	int player_1_pos;
	int player_2_pos;
	int ball_top = ball_position;
	int ball_bottom = ball_position + 320 * ball_dimensions[1];
	for (i = -1; i < ball_dimensions[0]; i++){
		ball_top += 1;
		ball_bottom += 1;
		for (j = 0; j < (sizeof(p_offset) / sizeof(*p_offset)); j++){
			player_1_pos = y_player1 + width + 320 * j;
			player_2_pos = y_player2 + 320 * j;
			if ((ball_top == player_1_pos) || (ball_top == player_2_pos) || (ball_bottom == player_1_pos) || (ball_bottom == player_2_pos)){
				// change the direction of the x value, ie direction from right to left, vice-versa
				write(sound_driver, 0, 3); // play bounce sound
				ball_direction[0] *= -1;
				
				// set current ball position to old position
				old_ball_position = ball_position;
				
				// set new ball position
				ball_position -= ball_direction[0] + ball_direction[1] * 320;
				return;
			}
		}
	}
	
	// check winning: check if any parts of the top part of the ball hits any of the end walls
	for (i = 0; i < 240; i++){
		int left_side = 0 + 320 * i;
		int right_side = 319 + 320 * i;
		for (j = 0; j <= ball_dimensions[0]; j++){
			int temp_ball_pos = ball_position + j;
			if(temp_ball_pos == left_side){
				// player 2 won
				game_over = 1;
				player_win[1] = 1;
				return;
			}
			if(temp_ball_pos == right_side){
				// player 1 won
				game_over = 1;
				player_win[0] = 1;
				return;
			}
		}
		
	}

	//No collision
	old_ball_position = ball_position;
	ball_position -= ball_direction[0] + ball_direction[1] * 320;
}
