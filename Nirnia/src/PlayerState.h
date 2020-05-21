#pragma once

enum class PlayerState {
	Idle0,
	Idle1,
	Idle2,
	Idle3,
	WalkLeft,
	WalkRight,
	WalkUp,
	WalkDown,

	NumStates
};


inline bool IsIdle(PlayerState state) {
	return (state >= PlayerState::Idle0) && (state <= PlayerState::Idle3);
}


inline PlayerState SetBlinkState(PlayerState state) {
	switch (state) {
		case PlayerState::Idle0:
			state = PlayerState::Idle1;
			break;
		case PlayerState::Idle1:
			state = PlayerState::Idle1;
			break;
		case PlayerState::Idle2:
			state = PlayerState::Idle3;
			break;
		case PlayerState::Idle3:
			state = PlayerState::Idle3;
			break;
	}
	return state;
}


inline PlayerState ClearBlinkState(PlayerState state) {
	switch (state) {
		case PlayerState::Idle0:
			state = PlayerState::Idle0;
			break;
		case PlayerState::Idle1:
			state = PlayerState::Idle0;
			break;
		case PlayerState::Idle2:
			state = PlayerState::Idle2;
			break;
		case PlayerState::Idle3:
			state = PlayerState::Idle2;
			break;
	}
	return state;
}


inline PlayerState SwapFootTapState(PlayerState state) {
	switch (state) {
		case PlayerState::Idle0:
			state = PlayerState::Idle2;
			break;
		case PlayerState::Idle1:
			state = PlayerState::Idle3;
			break;
		case PlayerState::Idle2:
			state = PlayerState::Idle0;
			break;
		case PlayerState::Idle3:
			state = PlayerState::Idle1;
			break;
	}
	return state;
}
