#include <stdio.h>
#include <stdlib.h>
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

int p_offset[80];
int width;
int player_move_mult;
int y_player1;
int y_player2;
int ball_position;
int old_ball_position;
int ball_dimensions[2];  //ind0 = width, ind1 = height
int ball_direction[2] = {-10, -10};
int fbfd;
int gamepad_driver;
uint16_t *map;
int test;

static void game_init(){
	printf("init begun");
	width = 8;
	y_player1 = 50;
	y_player2 = 270;
	ball_position = 12920;  // 38560 approx middle
	ball_dimensions[0] = 12; // Ball widht
	ball_dimensions[1] = 12; // Ball height
	player_move_mult = 10;
	int i;
	for(i = 0; i < (sizeof(p_offset) / sizeof(*p_offset)); i++){
		p_offset[i] = 320 * i;
	}
	fbfd = open("/dev/fb0", O_RDWR, (mode_t)0600);
	map = (uint16_t*)mmap(NULL, 320*240*2, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	printf("init completed");
}

static void GPIO_interrupt_handler(int unused)
{
	int buff = 0;
	if (test){
		read(gamepad_driver, &buff, sizeof(int));
	}
	// This is run whenever we recieve a GPIO interrupt
	//printf("i actually work though");

	
	if ((~buff & 0b10) == 2){
		if (y_player1 - 320 * player_move_mult > 0){
			y_player1 -= 320 * player_move_mult;
		}	
	}
	if ((~buff & 0b1000) == 8){
		if (y_player1 + 320 * (sizeof(p_offset) / sizeof(*p_offset)) + (320 * player_move_mult) <= 76800 + 320 * player_move_mult){
			y_player1 += 320 * player_move_mult;
		}	
	
	}
	if((~buff & 0b100000) == 0b100000){
		if (y_player2 - 320 * player_move_mult > 0){
			y_player2 -= 320 * player_move_mult;
		}	
	}
	if((~buff & 0b10000000) == 0b10000000){
		if (y_player2 + 320 * (sizeof(p_offset) / sizeof(*p_offset)) + (320 * player_move_mult) <= 76800 + 320 * player_move_mult){
			y_player2 += 320 * player_move_mult;
		}	
	}
}

int main(int argc, char *argv[])
{
	printf("main started");
	game_init();
	printf("game init completed");
	// Import gamepad-driver
	//system("modprobe gamepad-driver");
	// Set SIGIO signal to trigger handler
	struct sigaction GPIO_action = {}; 
	GPIO_action.sa_handler = &GPIO_interrupt_handler; 
	sigaction(SIGUSR1, &GPIO_action, NULL);
	printf("Signal set up");
	char command[50];
	strcpy(command, "modprobe driver-gamepad");
	system(command);
	printf("Driver started");
	gamepad_driver = open("/dev/GPIO", O_RDWR, (mode_t)0600);
	printf("%d \n", gamepad_driver);
	
	// black the screen
	int i;
	for (i = 0; i < 320*240; i++){
		map[i] = 0x0;
	}
	rect.dx = 0;
	rect.dy = 0;
	rect.width = 80;
	rect.height = 120;
	ioctl(fbfd, 0x4680, &rect);
	
	printf("Hello World, I'm game!\n");
	test = 1;
	while (1) {
		usleep(100000);
		tick();
	}
	exit(EXIT_SUCCESS);
}

static void tick(){
	//Move the ball
	move_ball();
	draw_map();	
}

static void draw_map(){
	int i;  // C doesnt like it in the loop declaration =(
	int j;
	
	// Turn screen black
	/*
	for (i = 0; i < 320*240; i++){
		map[i] = 0x0;
	}
	*/
	// Add blackie for the players
	for(i = 0; i < width; i++){
		for(j = 0; j < 240; j++){
			map[y_player1 % 320 + i + 320 * j] = 0x0;
			map[y_player2 % 320 + i + 320 * j] = 0x0;
		}
	}
	// Add blackie for the ball
	for(i = 0; i < ball_dimensions[0]; i++){
		for(j = 0; j < ball_dimensions[1]; j++){
			map[old_ball_position + i * 320 + j] = 0x0;
		}
	}
	
	// Add whitie for the players
	for(i = 0; i < width; i++){
		for(j = 0; j < (sizeof(p_offset) / sizeof(*p_offset)); j++){
			map[y_player1 + i + p_offset[j]] = 0xffff;
			map[y_player2 + i + p_offset[j]] = 0xffff;
		}
	}
	// Add whitie for the ball
	for(i = 1; i < ball_dimensions[0]; i++){
		for(j = 1; j < ball_dimensions[1]; j++){
			map[ball_position + i * 320 + j] = 0xffff;
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
	
	// remove ball old ball
	rect.dx = (old_ball_position % 320);
	rect.dy = (old_ball_position / 320);
	rect.width = ball_dimensions[0];
	rect.height = ball_dimensions[1];
	ioctl(fbfd, 0x4680, &rect);
	
	// draw ball
	rect.dx = ball_position % 320;
	rect.dy = ball_position / 320;
	rect.width = ball_dimensions[0];
	rect.height = ball_dimensions[1];
	ioctl(fbfd, 0x4680, &rect);

}
static void move_ball(){

	// check roof collision
	int temp_ball_pos_roof = ball_position + 360 * 5;
	temp_ball_pos_roof -= ball_direction[0] + ball_direction[1] * 320;
	int temp_ball_pos_floor = temp_ball_pos_roof + 320 * (ball_dimensions[1] - 5);
	if ((temp_ball_pos_roof < 320) || (temp_ball_pos_floor > 76800)){
		ball_direction[1] *= -1;
		old_ball_position = ball_position;
		ball_position -= ball_direction[0] + ball_direction[1] * 320;
		return;
	}
	
	// check player1 collision
	/*
	int i;
	int j;
	int b_l_upper_corner = ball_position + 5;
	b_l_upper_corner -= ball_direction[0] + ball_direction[1] * 320;
	int b_l_lower_corner = b_l_upper_corner + 320 * ball_dimensions[1];
	int player_indice1;
	int b_r_upper_corner = ball_position + ball_dimensions[0] - 5;
	b_r_upper_corner -= ball_direction[0] + ball_direction[1] * 320;
	int b_r_lower_corner = b_r_upper_corner + 320 * ball_dimensions[1];
	int player_indice2;
	for(int i = 1; i < width; i++){
		for(j = 0; j < (sizeof(p_offset) / sizeof(*p_offset)); j++){
			player_indice1 = y_player1 + 320 * j + i;
			player_indice2 = y_player2 + 320 * j + i;
			if ((b_l_upper_corner == player_indice1) || (b_l_lower_corner == player_indice1) || (b_r_upper_corner == player_indice2) || (b_r_lower_corner == player_indice2)){
			ball_direction[0] *= -1;
			ball_position -= ball_direction[0] + ball_direction[1] * 320;
			return;
			}
		}
	}
	*/
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
			ball_direction[0] *= -1;
			old_ball_position = ball_position;
			ball_position -= ball_direction[0] + ball_direction[1] * 320;
			return;
			}
		}
	}
	
	//No collision
	old_ball_position = ball_position;
	ball_position -= ball_direction[0] + ball_direction[1] * 320;
}

