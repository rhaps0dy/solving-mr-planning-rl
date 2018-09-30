#include <ncurses.h>
#include <cstdlib>
#include <algorithm>
#include <ale_interface.hpp>

using namespace std;

#define COL_WIDTH 90

Action get_action() {
	int ch;
	unsigned a;

	timeout(-1);
	ch = getch();
	switch(ch) {
	case ' ': a=PLAYER_A_FIRE; break;
	case KEY_UP: a=PLAYER_A_UP; break;
	case KEY_RIGHT: a=PLAYER_A_RIGHT; break;
	case KEY_LEFT: a=PLAYER_A_LEFT; break;
	case KEY_DOWN: a=PLAYER_A_DOWN; break;
	case 'q':
		return SYSTEM_RESET;
	default:
		return PLAYER_A_NOOP;
	}
	timeout(50);
	ch = getch();
	switch(ch) {
	case ' ':
		if(a != PLAYER_A_FIRE)
			a += 8;
		break;

	case KEY_UP:
		if(a == PLAYER_A_RIGHT || a == PLAYER_A_LEFT)
			a += 3;
		else if(a == PLAYER_A_FIRE)
			a = PLAYER_A_UPFIRE;
		break;
	case KEY_DOWN:
		if(a == PLAYER_A_RIGHT || a == PLAYER_A_LEFT)
			a += 5;
		else if(a == PLAYER_A_FIRE)
			a = PLAYER_A_DOWNFIRE;
		break;

	case KEY_RIGHT:
		if(a == PLAYER_A_UP)
			a = PLAYER_A_UPRIGHT;
		else if(a == PLAYER_A_DOWN)
			a = PLAYER_A_DOWNRIGHT;
		else if(a == PLAYER_A_FIRE)
			a = PLAYER_A_RIGHTFIRE;
		break;
	case KEY_LEFT:
		if(a == PLAYER_A_UP)
			a = PLAYER_A_UPLEFT;
		else if(a == PLAYER_A_DOWN)
			a = PLAYER_A_DOWNLEFT;
		else if(a == PLAYER_A_FIRE)
			a = PLAYER_A_LEFTFIRE;
		break;

	default:
		return static_cast<Action>(a);
	}
	ch = getch();
	switch(ch) {
	case ' ':
		if(a < 10)
			a += 8;
		break;

	case KEY_UP:
		if(a == PLAYER_A_RIGHTFIRE || a == PLAYER_A_LEFTFIRE)
			a += 3;
		break;
	case KEY_DOWN:
		if(a == PLAYER_A_RIGHTFIRE || a == PLAYER_A_LEFTFIRE)
			a += 5;
		break;

	case KEY_RIGHT:
		if(a == PLAYER_A_UPFIRE)
			a = PLAYER_A_UPRIGHTFIRE;
		else if(a == PLAYER_A_DOWNFIRE)
			a = PLAYER_A_DOWNRIGHTFIRE;
		break;
	case KEY_LEFT:
		if(a == PLAYER_A_UPFIRE)
			a = PLAYER_A_UPLEFTFIRE;
		else if(a == PLAYER_A_DOWNFIRE)
			a = PLAYER_A_DOWNLEFTFIRE;
		break;
	default: break;
	}
	return static_cast<Action>(a);
}

bool still_valid[18][0x80];

const char *HEADER[] = {
	"NOP|", " # |", " ^ |", " > |", " < |", " v |", " ^>|", "<^ |", " v>|",
	"<v |", "#^ |", " #>|", "<# |", "#v |", "#^>|", "<^#|", "#v>|", "<v#|"};
#define SEPARATOR \
	"----------------------------------------------------------------------------\n"
byte_t ram[0x80], prev_ram[0x80];

void print_ram(int a, unsigned offset, unsigned memoffset) {
	mvprintw(0, offset, "  |");
	auto CURRENT_PRESS=COLOR_PAIR(1);
	auto VALID=COLOR_PAIR(2);
	for(int x=0; x<18; x++) {
		if(x==a) {
			attron(CURRENT_PRESS);
			printw(HEADER[x]);
			attroff(CURRENT_PRESS);
		} else
			printw(HEADER[x]);
	}
	printw("\n" SEPARATOR);
	for(int y=memoffset; y<0x40+memoffset; y++) {
		mvprintw(y-memoffset+2, offset, "%2x|", y);
		for(int x=0; x<18; x++) {
			bool color=false;
			if(still_valid[x][y]) {
				attron(VALID);
				printw(" %2x|", ram[y]);
				attroff(VALID);
			} else if(x==a) {
				attron(CURRENT_PRESS);
				printw(" %2x|", ram[y]);
				attroff(CURRENT_PRESS);
			} else
				printw(" %2x|", ram[y]);
		}
	}
}

int ram_direction[0x80] = {0};

void update(const Action action, byte_t prev_ram[0x80]) {
	static unsigned counter=0;
	if(!(action == PLAYER_A_RIGHT || action == PLAYER_A_DOWN || action == PLAYER_A_DOWNRIGHT || action == PLAYER_A_LEFT || action == PLAYER_A_UP || action == PLAYER_A_UPLEFT))
		return;
	for(size_t i=0; i<0x80; i++) {
		int current_direction = ram[i]-prev_ram[i];
		if(action == PLAYER_A_LEFT || action == PLAYER_A_UP || action == PLAYER_A_UPLEFT)
			current_direction *= -1;

		mvprintw(2+i%0x40, 76+COL_WIDTH*(i/0x40), "%2x - %3d %3d|", prev_ram[i], ram_direction[i], current_direction);


		if(ram_direction[i] == 0) {
			ram_direction[i] = current_direction;
			if(counter >= 5)
				still_valid[action][i] = false;
		} else if(ram_direction[i] > 0) {
			if(current_direction < 0)
				still_valid[action][i] = false;
		} else {
			if(current_direction > 0)
				still_valid[action][i] = false;
		}
	}
	counter++;
}

int main()
{
	initscr();
	if(has_colors() == FALSE) {
		endwin();
		puts("Color is not supported");
		return 1;
	}
	start_color();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_BLACK, COLOR_RED);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);

	ALEInterface ale;
	ale.loadROM(ale.getString("rom_file"));
	fill(&still_valid[0][0], &still_valid[0][0] + sizeof(still_valid)/sizeof(still_valid[0][0]), true);
	for(size_t i=0; i<10; i++)
		ale.act(PLAYER_A_NOOP);
	for(size_t i=0; i<0x80; i++)
		ram[i] = ale.getRAM().get(i);

	for(;;) {
		Action a = get_action();
		if(a==SYSTEM_RESET) break;
		memcpy(prev_ram, ram, sizeof(ram));
		ale.act(a);
		for(size_t i=0; i<0x80; i++)
			ram[i] = ale.getRAM().get(i);
		update(a, prev_ram);
		print_ram(static_cast<int>(a), 0, 0);
		print_ram(static_cast<int>(a), COL_WIDTH, 0x40);
		refresh();
	}
	endwin();
	return 0;
}
