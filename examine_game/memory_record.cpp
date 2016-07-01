#include <ale_interface.hpp>
#include<emucore/M6502/src/System.hxx>
#include <SDL/SDL_events.h>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL_keysym.h>
#include<cstdio>
#include<sstream>
#include<iomanip>
#include<fstream>

int main(int argc, char *argv[]) {
	ALEInterface ale;

	ale.setInt("random_seed", 1234);
	ale.setBool("display_screen", true);
	ale.setBool("sound", false);
	ale.setInt("fragsize", 64);
// TFG_DIR is defined in the Makefile
	ale.loadROM(TFG_DIR "/montezuma_revenge.bin");

	System &sys = ale.theOSystem->console().system();
	ActionVect legal_actions = ale.getLegalActionSet();

	SDL_Event event;

	// Play a single episode, which we record. 
	int n_img = 0;
	bool activate = true;
	int lives = -1;
	sys.poke(0xc1, 0x1e);
	while (!ale.game_over()) {
		Uint8* keymap = SDL_GetKeyState(NULL);
		if(keymap[SDLK_j]) {
			if(activate) {
std::cout << "capturing\n";
				std::stringstream s;
				s << std::setfill('0') << std::setw(3) << (n_img++);
				ale.saveScreenPNG(s.str() + ".png");
				std::ofstream ram;
				ram.open((s.str() + ".ram").c_str(), std::ios::out | std::ios::binary);
				for(int i=0x80; i<0x100; i++)
					ram << sys.peek(i);
				ram.close();
				activate = false;
			}
		} else {
			activate = true;
		}
		if(lives != sys.peek(0x80+58)) {
			lives = sys.peek(0x80+58);
			std::cout << lives << " lives\n";
		}
//		Action a = legal_actions[0];
		// Apply the action (discard the resulting reward)
		ale.act(PLAYER_A_NOOP);
	}
	return 0;
}
