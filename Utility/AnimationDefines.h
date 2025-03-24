#pragma once 

namespace AnimationParameter {
	enum class AnimationParameterIndex : unsigned char {
		MoveDir = 0,	// BYTE type  
		Jump,			// BOOL type ( Trigger ) 	
		Attack,			// BOOL type ( Trigger )
		Attacked,		// BOOL type ( Trigger )
		Dead,			// BOOL type
	};

	enum class MoveDirection : unsigned char {
		Idle = 0, Forward, BackWard, Left, Right
	};
}