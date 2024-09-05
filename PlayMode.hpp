#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <random>

enum SpriteIdx {
    LIGHTNING = 0,
    GROUND1 = 12,
    GROUND2 = 13,
    P_WHEELSR = 14,
    P_BODYR = 15,
    P_HEADR = 16
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//some weird background animation:
	float background_fade = 0.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(128.0f);
	glm::vec2 player_vel = glm::vec2(0.0f);

	const uint32_t GROUND_HEIGHT = 4;
 
	//----- drawing handled by PPU466 -----

	PPU466 ppu;

	std::mt19937 random;
	int randomRange(int min, int max);

	void printTileTable(uint32_t limit);

	struct LightningTile{
		int16_t x;
		int16_t y;
		uint8_t startPos;
		uint8_t endPos;
		LightningTile(int16_t _x, int16_t _y, uint8_t _s, uint8_t _e) : x(_x), y(_y), startPos(_s), endPos(_e) {}
	};
	struct Lightning {
		std::vector<LightningTile> tiles;
		uint32_t branches = 0;
		float age = 0.0f;
	};

	std::deque<Lightning> lightning;
	float skyLightT = 0.0f;
	void lightningBranchRecursion(uint32_t idx, int x, int y, uint32_t startPt, Lightning *lightningVec);
	void spawnLightning();
	uint32_t updateLightning(float elapsed);
	void drawLightning();

	void drawPlayer();
};
