#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include "data_path.hpp"

#include <random>
#include <chrono>


void PlayMode::printTileTable(uint32_t limit = 20) {
	for(uint32_t t = 0; t < 256 && t < limit; t++) {
		for(uint32_t y = 0; y < 8; y++) {
			for(uint32_t x = 0; x < 8; x++) {
				uint8_t bit0 = (ppu.tile_table[t].bit0[y] >> x) & 1;
				uint8_t bit1 = (ppu.tile_table[t].bit1[y] >> x) & 1;
				std::cout << ((bit1 << 1) | bit0);
			}
			std::cout << std::endl;
		}
		std::cout << "\n\n\n";
	}
};

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:
	std::ifstream atlasBin(data_path("assets/atlas.bin"), std::ios::binary);
	atlasBin.read(reinterpret_cast<char *>(ppu.tile_table.data()), 16*16*8*2);
	atlasBin.close();

	printTileTable();
	ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0xff, 0xff),
		glm::u8vec4(0x00, 0xff, 0x00, 0xff),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
	};
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(89, 16, 21, 0xff),
		glm::u8vec4(194, 128, 37, 0xff),
		glm::u8vec4(242, 212, 128, 0xff),
	};

  uint32_t seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
  random = std::mt19937(seed);  // mt19937 is a standard mersenne_twister_engine


  for(uint16_t i = 0; i < PPU466::BackgroundHeight*PPU466::BackgroundWidth; i++) {
  	ppu.background[i] = 13u;
  }

  lightning = std::deque<Lightning>{};
  //spawnLightning();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	
	//spawnLightning();
	if(! (random() % static_cast<uint32_t>(200.0f*(std::clamp(1.0f - elapsed, 0.0f, 1.0f)))) ) {
		spawnLightning();
	}

	//slowly rotates through [0,1):
	// (will be used to set background color)
	uint32_t most_tiles = updateLightning(elapsed);

	float zenoMult = 0.95f + std::min(elapsed, 1.0f)*0.049f;
	skyLightT = (zenoMult)*skyLightT + (1 - zenoMult)*((float)most_tiles/(PPU466::ScreenHeight / 6));
	//skyLightT = std::clamp(skyLightT, 0.0f, 1.0f);

	//background_fade += elapsed / 10.0f;
	//background_fade -= std::floor(background_fade);

	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	//if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	//if (up.pressed) player_at.y += PlayerSpeed * elapsed;
	const float JumpVelocity = 50.0f;
	constexpr float gravity = 1.0f;
	if(space.pressed && player_at.y <= GROUND_HEIGHT*8) {
		player_vel.y += JumpVelocity*elapsed;
	} else {
		player_vel.y -= gravity*elapsed;
	}
	
	player_at.y += player_vel.y;
	//snap back to ground
	if(player_at.y < GROUND_HEIGHT*8) {
		player_at.y = static_cast<float>(GROUND_HEIGHT*8);
		player_vel.y = 0;
	}

	//std::cout << " | " << player_at.x << ", " << player_at.y << ", " << player_vel.x << ", " << player_vel.y << std::endl;
	

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
}

//random into from min to max, inclusive
int PlayMode::randomRange(int min = 0, int max = 1) {
	assert(max >= min);
	//std::uniform_int_distribution<> random(min, max);
	return (std::uniform_int_distribution<>(min, max))(random);
}

//idx is loop guard, 
void PlayMode::lightningBranchRecursion(uint32_t idx, int x, int y, uint32_t startPt, Lightning *lightningVec) {
	if(idx <= 0) return;
	uint16_t endPt = static_cast<uint16_t>(randomRange(0, 3));
	if(endPt == 3) {
		//roll again to be less likely
		endPt = static_cast<uint16_t>(randomRange(0, 3));
		lightningVec->branches++;
	}

	
	//TODO choose pallette

	//uint16_t bgIdx = (static_cast<uint16_t>(startPt) << 2) | endPt;
	//if(0 <= x && x < PPU466::BackgroundWidth && 0 <= y && y <= PPU466::BackgroundHeight) {
		//ppu.background[y*PPU466::BackgroundWidth + x] = bgIdx;
	lightningVec->tiles.emplace_back(LightningTile(static_cast<int16_t>(x), static_cast<int16_t>(y), static_cast<uint8_t>(startPt), static_cast<uint8_t>(endPt)));
	//}

	//dtd::cout << x << ", " << y << std::endl;
	if(endPt == 3) {
		if(random() % 2) {
			lightningBranchRecursion(randomRange(0, idx-1), x-1, y-1, 2, lightningVec);
			lightningBranchRecursion(idx-1, x+1, y-1, 0, lightningVec);
		} else {
			lightningBranchRecursion(idx-1, x-1, y-1, 2, lightningVec);
			lightningBranchRecursion(randomRange(0, idx-1), x+1, y-1, 0, lightningVec);
		}
	} else if (endPt == 1) { //goes down middle
		lightningBranchRecursion(idx-1, x, y-1, 1, lightningVec);
	} else if (endPt == 0  && (random() % 2)) {
		lightningBranchRecursion(idx-1, x, y-1, 0, lightningVec);
	} else if (endPt == 0) {
		lightningBranchRecursion(idx-1, x-1, y-1, 2, lightningVec);
	} else if (endPt == 2 && (random() % 2)) {
		lightningBranchRecursion(idx-1, x, y-1, 2, lightningVec);
	} else {
		lightningBranchRecursion(idx-1, x+1, y-1, 0, lightningVec);
	}
}

void PlayMode::spawnLightning() {
	int xStart = random() % (PPU466::ScreenWidth / 8);
	int yStart = PPU466::ScreenHeight / 8;
	Lightning newLightning{};
	lightningBranchRecursion(randomRange(3, PPU466::ScreenHeight/8), xStart, yStart, randomRange(0, 2), &newLightning);
	lightning.push_front(newLightning);
}

uint32_t PlayMode::updateLightning(float elapsed){
	uint32_t most_tiles = 0;;
	for(uint32_t i = 0; i < lightning.size(); i++) {
		Lightning l = lightning.front();
		lightning.pop_front();

		l.age += elapsed;
		//if lightning over a certain age kill it
		if(l.age < ((float)l.tiles.size() / (float)(PPU466::ScreenHeight/4))) { 
			lightning.push_back(l);
			most_tiles = static_cast<uint32_t>(l.tiles.size()) > most_tiles ? static_cast<uint32_t>(l.tiles.size()) : most_tiles;
		} 
	}
	return most_tiles;
}

void PlayMode::drawLightning(){
	static const float LIGHTNING_SPEED_MULT = 6.0f;
	//make bg lighter based on lightning age
	for(uint32_t i = 0; i < lightning.size(); i++) {
		Lightning l = lightning.front();
		lightning.pop_front();
		uint32_t tiles_to_draw = static_cast<uint32_t>(l.age*LIGHTNING_SPEED_MULT*static_cast<float>(PPU466::ScreenHeight / 8)) + l.branches/2;
		for(uint32_t j = 0; j < std::min(tiles_to_draw, static_cast<uint32_t>(l.tiles.size())); j++) {
			LightningTile t = l.tiles[j];
			uint16_t bgIdx = (static_cast<uint16_t>(t.startPos) << 2) | t.endPos;
			if(0 <= t.x && t.x < PPU466::BackgroundWidth && 0 <= t.y && t.y <= PPU466::BackgroundHeight) {
				ppu.background[t.y*PPU466::BackgroundWidth + t.x] = bgIdx;
			}
		}
		lightning.push_back(l);
	}
}

void PlayMode::drawPlayer() {
	/*Sprite wheel_sprite = {player_at.x, player_at.y, P_WHEELSR, 0};
	Sprite body_sprite = {player_at.x, player_at.y + 8, P_BODYR, 0};
	Sprite head_sprite = {player_at.x, player_at.y + 16, P_HEADR, 0};*/
	constexpr uint8_t heightOff = 4U;
	ppu.sprites[2] = PPU466::Sprite{static_cast<uint8_t>(player_at.x), static_cast<uint8_t>(player_at.y), P_WHEELSR, 0};
	ppu.sprites[1] = PPU466::Sprite{static_cast<uint8_t>(player_at.x), static_cast<uint8_t>(player_at.y + 8 - heightOff), P_BODYR, 0};
	ppu.sprites[0] = PPU466::Sprite{static_cast<uint8_t>(player_at.x), static_cast<uint8_t>(player_at.y + 16 - heightOff), P_HEADR, 0};

};

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	auto lerp = [](uint8_t a, uint8_t b, float t)->uint8_t {
		a = t > 1 ? 0 : a;
		float ret = (1-t)*(float)a + t*(float)b;
		return static_cast<uint8_t>(std::clamp(ret, 0.0f, 255.f));
	};

	uint8_t bgRDark = 43;
	uint8_t bgGDark = 21;
	uint8_t bgBDark = 69;
	uint8_t bgRLight = 160;
	uint8_t bgGLight = 118;
	uint8_t bgBLight = 173;
	
	ppu.background_color = glm::u8vec3(lerp(bgRDark, bgRLight, skyLightT), 
									   lerp(bgGDark, bgGLight, skyLightT), 
									   lerp(bgBDark, bgBLight, skyLightT));

	for(uint16_t i = 0; i < PPU466::BackgroundHeight*PPU466::BackgroundWidth; i++) {
  		ppu.background[i] = 255u;
  	}

	drawLightning();

	//drawGround();
	for(uint16_t i = 0; i < PPU466::BackgroundWidth; i++) {
		ppu.background[PPU466::BackgroundWidth*GROUND_HEIGHT + i] = GROUND2  | (1 << 8);
	}
	for(uint16_t i = 0; i < GROUND_HEIGHT; i++){
		for(uint16_t j = 0; j < PPU466::BackgroundWidth; j++) {
			ppu.background[PPU466::BackgroundWidth*i + j] = GROUND1 | (1 << 8);
		}

	}

	//drawPlayer
	drawPlayer();
	
	//--- actually draw ---
	ppu.draw(drawable_size);
}
 