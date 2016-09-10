#include <ale_interface.hpp>
#include<emucore/m6502/src/System.hxx>
#include <SDL/SDL_events.h>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL_keysym.h>
#include<cstdio>
#include<sstream>
#include<iostream>
#include<iomanip>
#include<fstream>
#include <utility>
#include <vector>

Action get_user_action(bool &is_arrows) {
    Uint8* keymap = SDL_GetKeyState(NULL);
    // Break out of this loop if the 'p' key is pressed
	Action a = PLAYER_A_NOOP;
	is_arrows = false;
    if (keymap[SDLK_p]) {
      return PLAYER_A_NOOP;
      // Triple Actions
/*    } else if (keymap[SDLK_UP] && keymap[SDLK_RIGHT] && keymap[SDLK_SPACE]) {
      a = PLAYER_A_UPRIGHTFIRE;
    } else if (keymap[SDLK_UP] && keymap[SDLK_LEFT] && keymap[SDLK_SPACE]) {
      a = PLAYER_A_UPLEFTFIRE;
    } else if (keymap[SDLK_DOWN] && keymap[SDLK_RIGHT] && keymap[SDLK_SPACE]) {
      a = PLAYER_A_DOWNRIGHTFIRE;
    } else if (keymap[SDLK_DOWN] && keymap[SDLK_LEFT] && keymap[SDLK_SPACE]) {
	a = PLAYER_A_DOWNLEFTFIRE;*/
      // Double Actions
/*    } else if (keymap[SDLK_UP] && keymap[SDLK_LEFT]) {
      a = PLAYER_A_UPLEFT;
    } else if (keymap[SDLK_UP] && keymap[SDLK_RIGHT]) {
      a = PLAYER_A_UPRIGHT;
    } else if (keymap[SDLK_DOWN] && keymap[SDLK_LEFT]) {
      a = PLAYER_A_DOWNLEFT;
    } else if (keymap[SDLK_DOWN] && keymap[SDLK_RIGHT]) {
      a = PLAYER_A_DOWNRIGHT;
    } else if (keymap[SDLK_UP] && keymap[SDLK_SPACE]) {
      a = PLAYER_A_UPFIRE;
    } else if (keymap[SDLK_DOWN] && keymap[SDLK_SPACE]) {
	a = PLAYER_A_DOWNFIRE;*/
    } else if (keymap[SDLK_LEFT] && keymap[SDLK_SPACE]) {
      a = PLAYER_A_LEFTFIRE;
    } else if (keymap[SDLK_RIGHT] && keymap[SDLK_SPACE]) {
      a = PLAYER_A_RIGHTFIRE;
      // Single Actions
    } else if (keymap[SDLK_SPACE]) {
      a = PLAYER_A_FIRE;
    } else if (keymap[SDLK_RETURN]) {
      a = PLAYER_A_NOOP;
    } else if (keymap[SDLK_LEFT]) {
      a = PLAYER_A_LEFT;
	  is_arrows = true;
    } else if (keymap[SDLK_RIGHT]) {
      a = PLAYER_A_RIGHT;
	  is_arrows = true;
    } else if (keymap[SDLK_UP]) {
      a = PLAYER_A_UP;
	  is_arrows = true;
    } else if (keymap[SDLK_DOWN]) {
      a = PLAYER_A_DOWN;
	  is_arrows = true;
    }
    return a;
}

const std::vector<Action> all_actions = {
    PLAYER_A_NOOP,
    PLAYER_A_UP,
    PLAYER_A_RIGHT
};

const std::vector<Action> vertical_actions = {PLAYER_A_UP, PLAYER_A_DOWN};
const std::vector<Action> horizontal_actions = {
	PLAYER_A_LEFT, PLAYER_A_RIGHT};

constexpr unsigned int ADDR_X = 0x2a;
constexpr unsigned int ADDR_Y = 0x2b;

bool is_state_controllable(ALEInterface &ale,
						   const std::vector<Action> &possible_actions) {
	ALEState s0 = ale.cloneSystemState();
	ale.act(possible_actions[0]);
	const byte_t x0 = ale.getRAM().get(ADDR_X);
	const byte_t y0 = ale.getRAM().get(ADDR_Y);
	bool controllable = false;
	for(size_t i=1; !controllable && i<possible_actions.size(); i++) {
		ale.restoreSystemState(s0);
		ale.act(possible_actions[i]);
		const byte_t xi = ale.getRAM().get(ADDR_X);
		const byte_t yi = ale.getRAM().get(ADDR_Y);
		if(x0 != xi || y0 != yi) {
			controllable = true;
		}
	}
	ale.restoreSystemState(s0);
	return controllable;
}

bool does_value_change(ALEInterface &ale,
					   const std::vector<Action> &possible_actions,
					   unsigned int addr) {
	ALEState s0 = ale.cloneSystemState();
	ale.act(possible_actions[0]);
	printf("initial X: %d\n", ale.getRAM().get(addr));
	const byte_t x0 = ale.getRAM().get(addr);
	bool controllable = false;
	for(size_t i=1; !controllable && i<possible_actions.size(); i++) {
		ale.restoreSystemState(s0);
		ale.act(possible_actions[i]);
		printf("X: %d %d\n", i, ale.getRAM().get(addr));
		const byte_t xi = ale.getRAM().get(addr);
		if(x0 != xi) {
			controllable = true;
		}
	}
	ale.restoreSystemState(s0);
	return controllable;
}
constexpr size_t N_BACK_FRAMES = 5;
constexpr int FRAME_SKIP = 4;
constexpr int NOT_MOVING_FRAMES = 10;
constexpr int MAX_FRAMES = 600;
#define MP std::make_pair

static int ABHG=0;
reward_t move_to_the(ALEInterface &ale, DisplayScreen *display, const Action action) {
	const std::vector<Action> *axis_actions;
	unsigned int unchanging_addr, changing_addr;
	if(action == PLAYER_A_LEFT || action == PLAYER_A_RIGHT) {
		axis_actions = &vertical_actions;
		unchanging_addr = ADDR_Y;
		changing_addr = ADDR_X;
	} else {
		axis_actions = &horizontal_actions;
		unchanging_addr = ADDR_X;
		changing_addr = ADDR_Y;
	}

	std::vector<std::pair<reward_t, ALEState> > frames;
	size_t last_controllable;
	const bool initial_cannot_change_axis =
		!does_value_change(ale, *axis_actions, unchanging_addr);
	byte_t prev_changing = ale.getRAM().get(changing_addr);
	const int initial_lives = ale.lives();
	int n_frames_unchanged;
	for(int max_n_iterations=0; max_n_iterations<MAX_FRAMES; max_n_iterations++) {
		reward_t reward = ale.act(action);
		frames.push_back(MP(reward, ale.cloneSystemState()));
		display->display_screen();
		bool controllable = is_state_controllable(ale, all_actions);
		bool stop_for_axis_change = initial_cannot_change_axis &&
			does_value_change(ale, *axis_actions, unchanging_addr);
		if(controllable)
			last_controllable = frames.size();
		if(stop_for_axis_change) {
			printf("Break because axis change possibility %d\n", ABHG++);
			break;
		}
//		printf("X: %d Y: %d\n", ale.getRAM().get(ADDR_X), ale.getRAM().get(ADDR_Y));
		byte_t new_changing = ale.getRAM().get(changing_addr);
		if(new_changing == prev_changing && controllable) {
			n_frames_unchanged++;
			if(n_frames_unchanged >= NOT_MOVING_FRAMES) {
				printf("Break because not moving %d\n", ABHG++);
				break;
			}
		} else {
			n_frames_unchanged = 0;
			prev_changing = new_changing;
		}
		if(ale.lives() < initial_lives) {
			while(!is_state_controllable(ale, all_actions)) {
				reward = ale.act(action);
				frames.push_back(MP(reward, ale.cloneSystemState()));
				display->display_screen();
			}
			break;
		}
	}
	if(ale.lives() < initial_lives && last_controllable > N_BACK_FRAMES) {
		frames.resize(last_controllable - N_BACK_FRAMES);
		ale.restoreSystemState(frames.rbegin()->second);
		display->display_screen();
	}
	// Wait for 1 extra frame
	reward_t reward = 0;
	for(size_t i=0; i<=frames.size(); i++)
		reward += frames[i].first;
	return reward;
}


int main(int argc, char *argv[]) {
	ALEInterface ale;

	ale.setInt("random_seed", 1234);
	ale.setBool("display_screen", true);
	ale.setBool("sound", false);
	ale.setInt("fragsize", 64);
	ale.setFloat("repeat_action_probability", 0.25);
// TFG_DIR is defined in the Makefile
	ale.loadROM(TFG_DIR "/montezuma_revenge_original.bin");

//	ActionVect legal_actions = ale.getLegalActionSet();
//	System &sys = ale.theOSystem->console().system();
	DisplayScreen *display = ale.theOSystem->p_display_screen;
	ale.theOSystem->p_display_screen = NULL;

	while (!ale.game_over()) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_c) {
					bool is_arrows;
					Action a = get_user_action(is_arrows);
					for(int i=0; i<FRAME_SKIP; i++) {
						ale.act(a);
						display->display_screen();
					}
					if(is_arrows) {
						move_to_the(ale, display, a);
					} else {
						while(!is_state_controllable(ale, all_actions)) {
							ale.act(PLAYER_A_NOOP);
							display->display_screen();
						}
						ale.act(PLAYER_A_NOOP);
					}
				} else if(event.key.keysym.sym == SDLK_q) {
					exit(0);
				}
			}
		}
		SDL_PumpEvents();
	}
	return 0;
}
