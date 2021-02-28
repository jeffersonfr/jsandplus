/**
 * This is a port of original project SDLSand <https://github.com/zear/SDLSand>.
 *
 */
#include "jcanvas/core/japplication.h"
#include "jcanvas/core/jwindow.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define STILLBORN_UPPER_BOUND 14
#define STILLBORN_LOWER_BOUND 1
#define FLOATING_UPPER_BOUND 35
#define FLOATING_LOWER_BOUND 32
#define PARTICLETYPE_ENUM_LENGTH 38

#define BUTTON_COUNT 19
#define BUTTON_SIZE 24
#define BUTTON_GAP 4
#define DASHBOARD_SIZE (BUTTON_SIZE + 4)

enum jparticle_type_t {
	// STILLBORN
	JPT_NOTHING = 0,
	JPT_WALL = 1,
	JPT_IRONWALL = 2,
	JPT_TORCH = 3,
	// ... = 4,
	JPT_STOVE = 5,
	JPT_ICE = 6,
	JPT_RUST = 7,
	JPT_EMBER = 8,
	JPT_PLANT = 9,
	JPT_VOID = 10,

	//SPOUTS
	JPT_WATERSPOUT = 11,
	JPT_SANDSPOUT = 12,
	JPT_SALTSPOUT = 13,
	JPT_OILSPOUT = 14,
	// ... = 15,

	//ELEMENTAL
	JPT_WATER = 16,
	JPT_MOVEDWATER = 17,
	JPT_DIRT = 18,
	JPT_MOVEDDIRT = 19,
	JPT_SALT = 20,
	JPT_MOVEDSALT = 21,
	JPT_OIL = 22,
	JPT_MOVEDOIL = 23,
	JPT_SAND = 24,
	JPT_MOVEDSAND = 25,

	//COMBINED
	JPT_SALTWATER = 26,
	JPT_MOVEDSALTWATER = 27,
	JPT_MUD = 28,
	JPT_MOVEDMUD = 29,
	JPT_ACID = 30,
	JPT_MOVEDACID = 31,

	//FLOATING
	JPT_STEAM = 32,
	JPT_MOVEDSTEAM = 33,
	JPT_FIRE = 34,
	JPT_MOVEDFIRE = 35,

	//Electricity
	JPT_ELEC = 36,
	JPT_MOVEDELEC = 37
};

// Button rectangle struct
typedef struct {
	jcanvas::jrect_t<int> rect;
	jparticle_type_t particleType;
} jbutton_rect_t;

class Screen : public jcanvas::Window {

	private:
		jparticle_type_t *_vs;
		jparticle_type_t _current_particle;
		jcanvas::jrect_t<int> _scene;
		jbutton_rect_t _buttons[BUTTON_COUNT];
		float _water_density;
		float _sand_density;
		float _salt_density;
		float _oil_density;
		int _slow;
		int _upper_row_y;
		int _middle_row_y;
		int _lower_row_y;
		int _particle_count;
		int _pen_size;
		int _old_x;
		int _old_y;
		int _mb_x;
		int _mb_y;
		int _can_move_x;
		int _can_move_y;
		int _speed_x;
		int _speed_y;
		bool _is_button_down;
		bool _emit_water;
		bool _emit_sand;
		bool _emit_salt;
		bool _emit_oil;
		bool _implement_particle_swaps;

	public:
		Screen():
			jcanvas::Window({720, 480})
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			_vs = new jparticle_type_t[size.x*size.y];

			_water_density = 0.3f;
			_sand_density = 0.3f;
			_salt_density = 0.3f;
			_oil_density = 0.3f;

			_emit_water = true;
			_emit_sand = true;
			_emit_salt = true;
			_emit_oil = true;

			_can_move_x = 0;
			_can_move_y = 0;
			_mb_x = 0;
			_mb_y = 0;
			_old_x = size.x/2;
			_old_y = size.y/2;
			_is_button_down = false;

			_particle_count = 0;
			_pen_size = 2;
			_slow = false;
			_speed_x = 0;
			_speed_y = 0;
			_current_particle = JPT_WALL;

			_implement_particle_swaps = true;

			_upper_row_y = size.y - BUTTON_SIZE - 1;
			_middle_row_y = size.y - BUTTON_SIZE - 1;
			_lower_row_y = size.y - BUTTON_SIZE - 1;

			init();
			Clear();
		}

		virtual ~Screen()
		{
		}

		//Checks wether a given particle type is a stillborn element
		bool IsStillborn(jparticle_type_t t)
		{
			return (t >= STILLBORN_LOWER_BOUND && t <= STILLBORN_UPPER_BOUND);
		}

		//Checks wether a given particle type is a floting type - like FIRE and STEAM
		bool IsFloating(jparticle_type_t t)
		{
			return (t >= FLOATING_LOWER_BOUND && t <= FLOATING_UPPER_BOUND);
		}

		//Checks wether a given particle type is burnable - like JPT_PLANT and OIL
		bool IsBurnable(jparticle_type_t t)
		{
			return (t == JPT_PLANT || t == JPT_OIL || t == JPT_MOVEDOIL);
		}

		//Checks wether a given particle type is burnable - like JPT_PLANT and OIL
		bool BurnsAsEmber(jparticle_type_t t)
		{
			return (t == JPT_PLANT); //Maybe we'll add a FUSE or WOOD
		}

		uint32_t colors[PARTICLETYPE_ENUM_LENGTH];

		// Initializing colors
		void initColors()
		{
			//STILLBORN
			colors[JPT_SAND] = 0xffeecc80;
			colors[JPT_WALL] = 0xff646464;
			colors[JPT_VOID] = 0xff3c3c3c;
			colors[JPT_IRONWALL] = 0xff6e6e6e;
			colors[JPT_TORCH] = 0xff8b4520;
			colors[JPT_STOVE] = 0xff4a4a4a;
			colors[JPT_ICE] = 0xffafeeee;
			colors[JPT_PLANT] = 0xff009600;
			colors[JPT_EMBER] = 0xff7f2020;
			colors[JPT_RUST] = 0xff6e280a;

			//ELEMENTAL
			colors[JPT_WATER] = 0xff2020ff;
			colors[JPT_DIRT] = 0xffcdaf96;
			colors[JPT_SALT] = 0xffffffff;
			colors[JPT_OIL] = 0xff804040;

			//COMBINED
			colors[JPT_MUD] = 0xff8b4515;
			colors[JPT_SALTWATER] = 0xff4070e0;
			colors[JPT_STEAM] = 0xff5f9ea0;

			//EXTRA
			colors[JPT_ACID] = 0xffadff2f;
			colors[JPT_FIRE] = 0xffff3232;
			colors[JPT_ELEC] = 0xffffff00;

			//SPOUTS
			colors[JPT_WATERSPOUT] = 0xff000080;
			colors[JPT_SANDSPOUT] = 0xfff0e68c;
			colors[JPT_SALTSPOUT] = 0xffeeeaea;
			colors[JPT_OILSPOUT] = 0xff6c2c2c;
		}

		// Emitting a given particletype at (x,o) width pixels wide and
		// with a p density (probability that a given pixel will be drawn 
		// at a given position withing the width)
		void Emit(int x, int width, jparticle_type_t type, float p)
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			for (int i=x-width/2; i<x+width/2; i++) {
				if (rand() < (int)(RAND_MAX * p)) {
					_vs[i + size.x] = type;
				}
			}
		}

		void StillbornParticleLogic(int x,int y,jparticle_type_t type)
		{
      jcanvas::jpoint_t<int>
        size = GetSize();
			int 
        index, 
        above, 
        left, 
        right, 
        below, 
        same, 
        abovetwo;

			switch (type) {
				case JPT_VOID:
					above = x + ((y - 1)*size.x);
					left = (x + 1) + (y*size.x);
					right = (x - 1) + (y*size.x);
					below = x + ((y + 1)*size.x);

					if (_vs[above] != JPT_NOTHING) {
						_vs[above] = JPT_NOTHING;
					}

					if (_vs[below] != JPT_NOTHING) {
						_vs[below] = JPT_NOTHING;
					}

					if (_vs[left] != JPT_NOTHING) {
						_vs[left] = JPT_NOTHING;
					}

					if (_vs[right] != JPT_NOTHING) {
						_vs[right] = JPT_NOTHING;
					}

					break;
				case JPT_IRONWALL:
					above = x + ((y - 1)*size.x);
					left = (x + 1) + (y*size.x);
					right = (x - 1)+(y*size.x);

					if (rand()%200 == 0 && (_vs[above] == JPT_RUST || _vs[left] == JPT_RUST || _vs[right] == JPT_RUST)) {
						_vs[x + (y*size.x)] = JPT_RUST;
					}

					break;
				case JPT_TORCH:
					above = x + ((y - 1)*size.x);
					left = (x + 1) + (y*size.x);
					right = (x - 1) + (y*size.x);

					if (rand()%2 == 0) { // Spawns fire
						if (_vs[above] == JPT_NOTHING || _vs[above] == JPT_MOVEDFIRE) { //Fire above
							_vs[above] = JPT_MOVEDFIRE;
						}

						if (_vs[right] == JPT_NOTHING || _vs[right] == JPT_MOVEDFIRE) { //Fire to the right
							_vs[right] = JPT_MOVEDFIRE;
						}

						if (_vs[left] == JPT_NOTHING || _vs[left] == JPT_MOVEDFIRE) { //Fire to the left
							_vs[left] = JPT_MOVEDFIRE;
						}
					}

					if (_vs[above] == JPT_MOVEDWATER || _vs[above] == JPT_WATER) { //Fire above
						_vs[above] = JPT_MOVEDSTEAM;
					}

					if (_vs[right] == JPT_MOVEDWATER || _vs[right] == JPT_WATER) { //Fire to the right
						_vs[right] = JPT_MOVEDSTEAM;
					}

					if (_vs[left] == JPT_MOVEDWATER || _vs[left] == JPT_WATER) { //Fire to the left
						_vs[left] = JPT_MOVEDSTEAM;
					}

					break;
				case JPT_PLANT:
					if (rand()%2 == 0) { //Making the plant grow _slowly
						index = 0;

						switch (rand()%4) {
							case 0: index = (x - 1)+(y*size.x); break;
							case 1: index = x + ((y - 1)*size.x); break;
							case 2: index = (x + 1) + (y*size.x); break;
							case 3:	index = x + ((y + 1)*size.x); break;
						}

						if (_vs[index] == JPT_WATER) {
							_vs[index] = JPT_PLANT;
						}
					}
					break;
				case JPT_EMBER:
					below = x + ((y + 1)*size.x);

					if (_vs[below] == JPT_NOTHING || IsBurnable(_vs[below])) {
						_vs[below] = JPT_FIRE;
					}

					index = 0;

					switch (rand()%4) {
						case 0: index = (x - 1) + (y*size.x); break;
						case 1: index = x + ((y - 1)*size.x); break;
						case 2: index = (x + 1) + (y*size.x); break;
						case 3:	index = x + ((y + 1)*size.x); break;
					}

					if (_vs[index] == JPT_PLANT) {
						_vs[index] = JPT_FIRE;
					}

					if (rand()%18 == 0) { // Making ember burn out _slowly
						_vs[x + (y*size.x)] = JPT_NOTHING;
					}

					break;
				case JPT_STOVE:
					above = x + ((y - 1)*size.x);
					abovetwo = x + ((y - 2)*size.x);

					if (rand()%4 == 0 && _vs[above] == JPT_WATER) { // Boil the water
						_vs[above] = JPT_STEAM;
					}

					if (rand()%4 == 0 && _vs[above] == JPT_SALTWATER) { // Saltwater separates
						_vs[above] = JPT_SALT;
						_vs[abovetwo] = JPT_STEAM;
					}

					if (rand()%8 == 0 && _vs[above] == JPT_OIL) { // Set oil aflame
						_vs[above] = JPT_EMBER;
					}

					break;
				case JPT_RUST:
					if (rand()%7000 == 0) { //Deteriate rust
						_vs[x + (y*size.x)] = JPT_NOTHING;
					}

					break;

					//####################### SPOUTS ####################### 
				case JPT_WATERSPOUT:
					if (rand()%6 == 0) { // Take it easy on the spout
						below = x + ((y + 1)*size.x);

						if (_vs[below] == JPT_NOTHING) {
							_vs[below] = JPT_MOVEDWATER;
						}
					}

					break;
				case JPT_SANDSPOUT:
					if (rand()%6 == 0) { // Take it easy on the spout
						below = x + ((y + 1)*size.x);

						if (_vs[below] == JPT_NOTHING) {
							_vs[below] = JPT_MOVEDSAND;
						}
					}

					break;
				case JPT_SALTSPOUT:
					if (rand()%6 == 0) { // Take it easy on the spout
						below = x + ((y + 1)*size.x);

						if (_vs[below] == JPT_NOTHING) {
							_vs[below] = JPT_MOVEDSALT;
						}

						if (_vs[below] == JPT_WATER || _vs[below] == JPT_MOVEDWATER) {
							_vs[below] = JPT_MOVEDSALTWATER;
						}
					}

					break;
				case JPT_OILSPOUT:
					if (rand()%6 == 0) { // Take it easy on the spout
						below = x + ((y + 1)*size.x);

						if (_vs[below] == JPT_NOTHING) {
							_vs[below] = JPT_MOVEDOIL;
						}
					}

					break;
				default:
					break;
			}
		}

		// Performing the movement logic of a given particle. The argument 'type' is passed so that we don't need a table 
		// lookup when determining the type to set the given particle to - i.e. if the particle is SAND then the passed type 
		// will be MOVEDSAND
		inline void MoveParticle(int x, int y, jparticle_type_t type)
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			type = (jparticle_type_t)(type+1);

			int above = x + ((y - 1)*size.x);
			int same = x + (size.x*y);
			int below = x + ((y + 1)*size.x);

			// If nothing below then just fall (gravity)
			if (!IsFloating(type)) {
				if ( (_vs[below] == JPT_NOTHING) && (rand() % 8)) { //rand() % 8 makes it spread
					_vs[below] = type;
					_vs[same] = JPT_NOTHING;
					return;
				}
			} else {
				if (rand()%3 == 0) { //Slow _is_button_down please
					return;
				}

				//If nothing above then rise (floating - or reverse gravity? ;))
				if ((_vs[above] == JPT_NOTHING || _vs[above] == JPT_FIRE) && (rand() % 8) && (_vs[same] != JPT_ELEC) && (_vs[same] != JPT_MOVEDELEC)) { //rand() % 8 makes it spread
					if (type == JPT_MOVEDFIRE && rand()%20 == 0) {
						_vs[same] = JPT_NOTHING;
					} else {
						_vs[above] = _vs[same];
						_vs[same] = JPT_NOTHING;
					}

					return;
				}
			}

			//Randomly select right or left first
			int sign = (rand() % 2 == 0)?-1:1;

			// We'll only calculate these indicies once for optimization purpose
			int first = (x + sign) + (size.x*y);
			int second = (x - sign) + (size.x*y);
			int index = 0;

			//Particle type specific logic
			switch (type) {
				case JPT_MOVEDELEC:
					if (rand()%2 == 0) {
						_vs[same] = JPT_NOTHING;
					}

					break;
				case JPT_MOVEDSTEAM:
					if (rand()%1000 == 0) {
						_vs[same] = JPT_MOVEDWATER;

						return;
					}

					if (rand()%500 == 0) {
						_vs[same] = JPT_NOTHING;

						return;
					}

					if (!IsStillborn(_vs[above]) && !IsFloating(_vs[above])) {
						if (rand()%15 == 0) {
							_vs[same] = JPT_NOTHING;

							return;
						} else {
							_vs[same] = _vs[above];
							_vs[above] = JPT_MOVEDSTEAM;

							return;
						}
					}

					break;
				case JPT_MOVEDFIRE:
					if (!IsBurnable(_vs[above]) && rand()%10 == 0) {
						_vs[same] = JPT_NOTHING;

						return;
					}

					// Let the snowman melt!
					if (rand()%4 == 0) {
						if (_vs[above] == JPT_ICE) {
							_vs[above] = JPT_WATER;
							_vs[same] = JPT_NOTHING;
						}

						if (_vs[below] == JPT_ICE) {
							_vs[below] = JPT_WATER;
							_vs[same] = JPT_NOTHING;
						}

						if (_vs[first] == JPT_ICE) {
							_vs[first] = JPT_WATER;
							_vs[same] = JPT_NOTHING;
						}

						if (_vs[second] == JPT_ICE) {
							_vs[second] = JPT_WATER;
							_vs[same] = JPT_NOTHING;
						}
					}

					//Let's burn whatever we can!
					index = 0;

					switch (rand()%4) {
						case 0: index = above; break;
						case 1: index = below; break;
						case 2: index = first; break;
						case 3:	index = second; break;
					}

					if (IsBurnable(_vs[index])) {
						if (BurnsAsEmber(_vs[index])) {
							_vs[index] = JPT_EMBER;
						} else {
							_vs[index] = JPT_FIRE;
						}
					}

					break;
				case JPT_MOVEDWATER:
					if (rand()%200 == 0 && _vs[below] == JPT_IRONWALL) {
						_vs[below] = JPT_RUST;
					}

					if (_vs[below]  == JPT_FIRE || _vs[above] == JPT_FIRE || _vs[first] == JPT_FIRE || _vs[second] == JPT_FIRE) {
						_vs[same] = JPT_MOVEDSTEAM;
					}

					//Making water+dirt into dirt
					if (_vs[below] == JPT_DIRT) {
						_vs[below] = JPT_MOVEDMUD;
						_vs[same] = JPT_NOTHING;
					}

					if (_vs[above] == JPT_DIRT) {
						_vs[above] = JPT_MOVEDMUD;
						_vs[same] = JPT_NOTHING;
					}

					//Making water+salt into saltwater
					if (_vs[above] == JPT_SALT || _vs[above] == JPT_MOVEDSALT) {
						_vs[above] = JPT_MOVEDSALTWATER;
						_vs[same] = JPT_NOTHING;
					}

					if (_vs[below] == JPT_SALT || _vs[below] == JPT_MOVEDSALT) {
						_vs[below] = JPT_MOVEDSALTWATER;
						_vs[same] = JPT_NOTHING;
					}

					if (rand()%60 == 0) { //Melting ice
						switch (rand()%4) {
							case 0:	index = above; break;
							case 1:	index = below; break;
							case 2:	index = first; break;
							case 3:	index = second; break;
						}

						if (_vs[index] == JPT_ICE) {
							_vs[index] = JPT_WATER;
						}
					}

					break;
				case JPT_MOVEDACID:
					switch (rand()%4) {
						case 0:	index = above; break;
						case 1:	index = below; break;
						case 2:	index = first; break;
						case 3:	index = second; break;
					}

					if (_vs[index] != JPT_WALL && _vs[index] != JPT_IRONWALL && _vs[index] != JPT_WATER && _vs[index] != JPT_MOVEDWATER && _vs[index] != JPT_ACID && _vs[index] != JPT_MOVEDACID) {
						_vs[index] = JPT_NOTHING;
					}

					break;
				case JPT_MOVEDSALT:
					if (rand()%20 == 0) {
						switch (rand()%4) {
							case 0:	index = above; break;
							case 1:	index = below; break;
							case 2:	index = first; break;
							case 3:	index = second; break;
						}

						if (_vs[index] == JPT_ICE) {
							_vs[index] = JPT_WATER;
						}
					}

					break;
				case JPT_MOVEDSALTWATER:
					//Saltwater separated by heat
					//	if (_vs[above] == FIRE || _vs[below] == FIRE || _vs[first] == FIRE || _vs[second] == FIRE || _vs[above] == STOVE || _vs[below] == STOVE || _vs[first] == STOVE || _vs[second] == STOVE)
					//	{
					//		_vs[same] = SALT;
					//		_vs[above] = STEAM;
					//	}
					if (rand()%40 == 0) { //Saltwater dissolves ice more _slowly than pure salt
						switch (rand()%4) {
							case 0:	index = above; break;
							case 1:	index = below; break;
							case 2:	index = first; break;
							case 3:	index = second; break;
						}

						if (_vs[index] == JPT_ICE) {
							_vs[index] = JPT_WATER;
						}
					}

					break;
				case JPT_MOVEDOIL:
					switch (rand()%4) {
						case 0:	index = above; break;
						case 1:	index = below; break;
						case 2:	index = first; break;
						case 3:	index = second; break;
					}

					if (_vs[index] == JPT_FIRE) {
						_vs[same] = JPT_FIRE;
					}

					break;
				default:
					break;
			}

			//Peform 'realism' logic?
			// When adding dynamics to this part please use the following structure:
			// If a particle A is ligther than particle B then add _vs[above] == B to the condition in case A (case MOVED_A)
			if (_implement_particle_swaps) {
				switch (type) {
					case JPT_MOVEDWATER:
						if (_vs[above] == JPT_SAND || _vs[above] == JPT_MUD || _vs[above] == JPT_SALTWATER && rand()%3 == 0) {
							_vs[same] = _vs[above];
							_vs[above] = type;

							return;
						}

						break;
					case JPT_MOVEDOIL:
						if (_vs[above] == JPT_WATER && rand()%3 == 0) {
							_vs[same] = _vs[above];
							_vs[above] = type;

							return;
						}

						break;
					case JPT_MOVEDSALTWATER:
						if (_vs[above] == JPT_DIRT || _vs[above] == JPT_MUD || _vs[above] == JPT_SAND && rand()%3 == 0) {
							_vs[same] = _vs[above];
							_vs[above] = type;

							return;
						}

						break;
					default:
						break;
				}
			}

			// The place below (x,y+1) is filled with something, then check (x+sign,y+1) and (x-sign,y+1).
			// We chose sign randomly to randomly check eigther left or right. This is for elements that fall _is_button_downward
			if (!IsFloating(type)) {
				int first_is_button_down = (x + sign) + ((y + 1)*size.x);
				int second_is_button_down = (x - sign) + ((y + 1)*size.x);

				if ( _vs[first_is_button_down] == JPT_NOTHING) {
					_vs[first_is_button_down] = type;
					_vs[same] = JPT_NOTHING;
				} else if ( _vs[second_is_button_down] == JPT_NOTHING) {
					_vs[second_is_button_down] = type;
					_vs[same] = JPT_NOTHING;
				} else if (_vs[first] == JPT_NOTHING) {
					_vs[first] = type;
					_vs[same] = JPT_NOTHING;
				} else if (_vs[second] == JPT_NOTHING) {
					_vs[second] = type;
					_vs[same] = JPT_NOTHING;
				}
			} else if (type == JPT_MOVEDSTEAM) {
				// Make steam move
				int firstup = (x + sign) + ((y - 1)*size.x);
				int secondup = (x - sign) + ((y - 1)*size.x);

				if ( _vs[firstup] == JPT_NOTHING) {
					_vs[firstup] = type;
					_vs[same] = JPT_NOTHING;
				} else if ( _vs[secondup] == JPT_NOTHING) {
					_vs[secondup] = type;
					_vs[same] = JPT_NOTHING;
				} else if (_vs[first] == JPT_NOTHING) {
					_vs[first] = type;
					_vs[same] = JPT_NOTHING;
				} else if (_vs[second] == JPT_NOTHING) {
					_vs[second] = type;
					_vs[same] = JPT_NOTHING;
				}
			}
		}

		//Drawing a filled circle at a given position with a given radius and a given partice type
		void DrawParticles(int xpos, int ypos, int radius, jparticle_type_t type)
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			for (int x=((xpos-radius-1) < 0)?0:(xpos-radius-1); x<=xpos+radius && x<size.x; x++) {
				for (int y=((ypos-radius-1) < 0)?0:(ypos-radius-1); y<=ypos+radius && y<size.y; y++) {
					if ((x - xpos)*(x - xpos) + (y - ypos)*(y - ypos) <= radius*radius) {
						_vs[x + (size.x*y)] = type;
					}
				}
			}
		}

		void DrawLine(int newx, int newy, int _old_x, int _old_y)
		{
			if (newx == _old_x && newy == _old_y) {
				DrawParticles(newx,newy,_pen_size,_current_particle);
			} else {
				float step = 1.0f / ((abs(newx-_old_x)>abs(newy-_old_y)) ? abs(newx-_old_x) : abs(newy-_old_y));

				for (float a = 0; a < 1; a+=step) {
					DrawParticles(a*newx+(1-a)*_old_x,a*newy+(1-a)*_old_y,_pen_size,_current_particle); 
				}
			}
		}

		void DoRandomLines(jparticle_type_t type)
		{
      jcanvas::jpoint_t<int>
        size = GetSize();
			jparticle_type_t 
        tmp = _current_particle;

			_current_particle = type;

			for (int i = 0; i < 20; i++) {
				int x1 = rand() % size.x;
				int x2 = rand() % size.x;

				DrawLine(x1, 0, x2, size.y);
			}

			for (int i = 0; i < 20; i++) {
				int y1 = rand() % size.y;
				int y2 = rand() % size.y;

				DrawLine(0, y1, size.x, y2);
			}

			_current_particle = tmp;
		}

		inline void UpdateVirtualPixel(int x, int y)
		{
      jcanvas::jpoint_t<int>
        size = GetSize();
			jparticle_type_t 
        same = _vs[x + (size.x*y)];

			if (same != JPT_NOTHING) {
				if (IsStillborn(same)) {
					StillbornParticleLogic(x,y,same);
				} else {
					if (rand() >= RAND_MAX / 13 && same % 2 == 0) {
						MoveParticle(x,y,same); //THe rand condition makes the particles fall unevenly
					}
				}
			}
		}

		// Updating the particle system (virtual screen) pixel by pixel
		inline void UpdateVirtualScreen()
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			for (int y =0; y<size.y-DASHBOARD_SIZE; y++) {
				// Due to biasing when iterating through the scanline from left to right,
				// we now chose our direction randomly per scanline.
				if (rand() % 2 == 0) {
					for (int x=size.x-2; x--;) {
						UpdateVirtualPixel(x,y);
					}
				} else {
					for (int x=1; x<size.x-1; x++) {
						UpdateVirtualPixel(x,y);
					}
				}
			}
		}

		//Cearing the particle system
		void Clear()
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			for (int w=0; w<size.x ; w++) {
				for (int h=0; h<size.y; h++) {
					_vs[w + (size.x*h)] = JPT_NOTHING;
				}
			}
		}

		void DrawRect(jcanvas::Graphics *g, jcanvas::jrect_t<int> bounds, uint32_t color)
		{
			g->SetColor(color);
			g->DrawRectangle(bounds);
		}

		void FillRect(jcanvas::Graphics *g, jcanvas::jrect_t<int> bounds, uint32_t color)
		{
			g->SetColor(color);
			g->FillRectangle(bounds);
		}

		void InitButtons()
		{
			// set up water emit button
			jcanvas::jrect_t<int> wateroutput;
			wateroutput.point = {
        0*(BUTTON_SIZE + BUTTON_GAP) + BUTTON_GAP, _upper_row_y
      };
			wateroutput.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t warect;
			warect.rect = wateroutput;
			warect.particleType = JPT_WATER;
			_buttons[0] = warect;

			// set up sand sand button
			jcanvas::jrect_t<int> sandoutput;
			sandoutput.point = {
        1*(BUTTON_SIZE + BUTTON_GAP) + BUTTON_GAP, _upper_row_y
      };
			sandoutput.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t sanrect;
			sanrect.rect = sandoutput;
			sanrect.particleType = JPT_SAND;
			_buttons[1] = sanrect;

			// set up salt emit button
			jcanvas::jrect_t<int> saltoutput;
			saltoutput.point = {
        2*(BUTTON_SIZE + BUTTON_GAP) + BUTTON_GAP, _upper_row_y
      };
			saltoutput.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t sarect;
			sarect.rect = saltoutput;
			sarect.particleType = JPT_SALT;
			_buttons[2] = sarect;

			// set up oil button
			jcanvas::jrect_t<int> oiloutput;
			oiloutput.point = {
        3*(BUTTON_SIZE + BUTTON_GAP) + BUTTON_GAP, _upper_row_y
      };
			oiloutput.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t oirect;
			oirect.rect = oiloutput;
			oirect.particleType = JPT_OIL;
			_buttons[3] = oirect;

			// set up fire button
			jcanvas::jrect_t<int> fire;
			fire.point = {
        4*(BUTTON_SIZE + BUTTON_GAP) + BUTTON_GAP, _upper_row_y
      };
			fire.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t firect;
			firect.particleType = JPT_FIRE;
			firect.rect = fire;
			_buttons[4] = firect;

			// set up acid button
			jcanvas::jrect_t<int> acid;
			acid.point = {
        5*(BUTTON_SIZE + BUTTON_GAP) + BUTTON_GAP, _upper_row_y
      };
			acid.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t acrect;
			acrect.particleType = JPT_ACID;
			acrect.rect = acid;
			_buttons[5] = acrect;

			// set up dirt emit button
			jcanvas::jrect_t<int> dirtoutput;
			dirtoutput.point = {
        6*(BUTTON_SIZE + BUTTON_GAP) + BUTTON_GAP, _upper_row_y
      };
			dirtoutput.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t direct;
			direct.rect = dirtoutput;
			direct.particleType = JPT_DIRT;
			_buttons[6] = direct;

			// set up spout water
			jcanvas::jrect_t<int> spwater;
			spwater.point = {
        7*(BUTTON_SIZE + BUTTON_GAP) + 2*BUTTON_GAP + BUTTON_GAP, _middle_row_y
      };
			spwater.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t spwrect;
			spwrect.particleType = JPT_WATERSPOUT;
			spwrect.rect = spwater;
			_buttons[7] = spwrect;

			// set up spout sand
			jcanvas::jrect_t<int> spdirt;
			spdirt.point = {
        8*(BUTTON_SIZE + BUTTON_GAP) + 2*BUTTON_GAP + BUTTON_GAP, _middle_row_y
      };
			spdirt.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t spdrect;
			spdrect.particleType = JPT_SANDSPOUT;
			spdrect.rect = spdirt;
			_buttons[8] = spdrect;

			// set up spout salt
			jcanvas::jrect_t<int> spsalt;
			spsalt.point = {
        9*(BUTTON_SIZE + BUTTON_GAP) + 2*BUTTON_GAP + BUTTON_GAP, _middle_row_y
      };
			spsalt.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t spsrect;
			spsrect.particleType = JPT_SALTSPOUT;
			spsrect.rect = spsalt;
			_buttons[9] = spsrect;

			// set up spout oil
			jcanvas::jrect_t<int> spoil;
			spoil.point = {
        10*(BUTTON_SIZE + BUTTON_GAP) + 2*BUTTON_GAP + BUTTON_GAP, _middle_row_y
      };
			spoil.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t sporect;
			sporect.particleType = JPT_OILSPOUT;
			sporect.rect = spoil;
			_buttons[10] = sporect;

			// set up wall button
			jcanvas::jrect_t<int> wall;
			wall.point = {
        11*(BUTTON_SIZE + BUTTON_GAP) + 4*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			wall.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t walrect;
			walrect.particleType = JPT_WALL;
			walrect.rect = wall;
			_buttons[11] = walrect;

			// set up torch button
			jcanvas::jrect_t<int> torch;
			torch.point = {
        12*(BUTTON_SIZE + BUTTON_GAP) + 4*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			torch.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t torect;
			torect.particleType = JPT_TORCH;
			torect.rect = torch;
			_buttons[12] = torect;

			// set up stove button
			jcanvas::jrect_t<int> stove;
			stove.point = {
        13*(BUTTON_SIZE + BUTTON_GAP) + 4*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			stove.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t storect;
			storect.particleType = JPT_STOVE;
			storect.rect = stove;
			_buttons[13] = storect;

			// set up plant button
			jcanvas::jrect_t<int> plant;
			plant.point = {
        14*(BUTTON_SIZE + BUTTON_GAP) + 4*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			plant.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t prect;
			prect.particleType = JPT_PLANT;
			prect.rect = plant;
			_buttons[14] = prect;

			// ice
			jcanvas::jrect_t<int> ice;
			ice.point = {
        15*(BUTTON_SIZE + BUTTON_GAP) + 4*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			ice.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t irect;
			irect.particleType = JPT_ICE;
			irect.rect = ice;
			_buttons[15] = irect;

			// ironwall
			jcanvas::jrect_t<int> iw;
			iw.point = {
        16*(BUTTON_SIZE + BUTTON_GAP) + 4*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			iw.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t iwrect;
			iwrect.particleType = JPT_IRONWALL;
			iwrect.rect = iw;
			_buttons[16] = iwrect;

			// void
			jcanvas::jrect_t<int> voidele;
			voidele.point = {
        17*(BUTTON_SIZE + BUTTON_GAP) + 4*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			voidele.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t voidelerect;
			voidelerect.particleType = JPT_VOID;
			voidelerect.rect = voidele;
			_buttons[17] = voidelerect;

			// eraser
			jcanvas::jrect_t<int> eraser;
			eraser.point = {
        18*(BUTTON_SIZE + BUTTON_GAP) + 6*BUTTON_GAP + BUTTON_GAP, _lower_row_y
      };
			eraser.size = {
        BUTTON_SIZE, BUTTON_SIZE
      };
			jbutton_rect_t eraserrect;
			eraserrect.particleType = JPT_NOTHING;
			eraserrect.rect = eraser;
			_buttons[18] = eraserrect;
		}

		// Initializing the screen
		void init()
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			initColors();

			_scene = {
        .point = {
          .x = 0,
			    .y = 0
        },
        .size = {
			    .x = size.x,
			    .y = size.y - DASHBOARD_SIZE
        }
      };

			//set up dashboard
			jcanvas::jrect_t<int> dashboard ;

			dashboard = {
        .point = {
          .x = 0,
			    .y = size.y - DASHBOARD_SIZE
        },
        .size = {
			    .x = size.x,
			    .y = DASHBOARD_SIZE
        }
      };

			InitButtons();
		}

		inline void CheckGuiInteraction(int _mb_x, int _mb_y)
		{
			for (int i = BUTTON_COUNT; i--;) {
				jbutton_rect_t r = _buttons[i];

				if (_mb_x > r.rect.point.x && _mb_x <= r.rect.point.x + r.rect.size.x && _mb_y > r.rect.point.y && _mb_y <= r.rect.point.y + r.rect.size.y) {
					_current_particle = r.particleType;
				}
			}
		}

		std::string GetParticleName(jparticle_type_t t)
		{
			if (t == JPT_NOTHING) {
				return "EMPTY";
			} else if (t == JPT_WALL) {
				return "WALL";
			} else if (t == JPT_IRONWALL) {
				return "IRON WALL";
			} else if (t == JPT_TORCH) {
				return "TORCH";
			} else if (t == JPT_STOVE) {
				return "STOVE";
			} else if (t == JPT_ICE) {
				return "ICE";
			} else if (t == JPT_RUST) {
				return "RUST";
			} else if (t == JPT_EMBER) {
				return "EMBER";
			} else if (t == JPT_PLANT) {
				return "PLANT";
			} else if (t == JPT_VOID) {
				return "VOID";
			} else if (t == JPT_WATERSPOUT) {
				return "WATER SPOUT";
			} else if (t == JPT_SANDSPOUT) {
				return "SAND SPOUT";
			} else if (t == JPT_SALTSPOUT) {
				return "SALT SPOUT";
			} else if (t == JPT_OILSPOUT) {
				return "OIL SPOUT";
			} else if (t == JPT_WATER) {
				return "WATER";
			} else if (t == JPT_MOVEDWATER) {
				return "MOVED WATER";
			} else if (t == JPT_DIRT) {
				return "DIRT";
			} else if (t == JPT_MOVEDDIRT) {
				return "MOVED DIRT";
			} else if (t == JPT_SALT) {
				return "SALT";
			} else if (t == JPT_MOVEDSALT) {
				return "MOVED SALT";
			} else if (t == JPT_OIL) {
				return "OIL";
			} else if (t == JPT_MOVEDOIL) {
				return "MOVED OIL";
			} else if (t == JPT_SAND) {
				return "SAND";
			} else if (t == JPT_MOVEDSAND) {
				return "MOVED SAND";
			} else if (t == JPT_SALTWATER) {
				return "SALT WATER";
			} else if (t == JPT_MOVEDSALTWATER) {
				return "MOVED SALT WATER";
			} else if (t == JPT_MUD) {
				return "MUD";
			} else if (t == JPT_MOVEDMUD) {
				return "MOVED MUD";
			} else if (t == JPT_ACID) {
				return "ACID";
			} else if (t == JPT_MOVEDACID) {
				return "MOVED ACID";
			} else if (t == JPT_STEAM) {
				return "STEAM";
			} else if (t == JPT_MOVEDSTEAM) {
				return "MOVED STEAM";
			} else if (t == JPT_FIRE) {
				return "FIRE";
			} else if (t == JPT_MOVEDFIRE) {
				return "MOVED FIRE";
			} else if (t == JPT_ELEC) {
				return "ELECTRICITY";
			} else if (t == JPT_MOVEDELEC) {
				return "MOVED ELECTRICITY";
			}

			return "";
		}

		void drawSelection(jcanvas::Graphics *g)
		{
      jcanvas::jpoint_t<int>
        size = GetSize();

			for (int i=BUTTON_COUNT; i--;) {
				jbutton_rect_t button = _buttons[i];

				if (_current_particle == button.particleType) {
					DrawRect(g, button.rect, 0xff000000);

					g->DrawString(GetParticleName(_current_particle), {0, button.rect.point.y, size.x - BUTTON_GAP, button.rect.size.y}, jcanvas::JHA_RIGHT);
				}
			}
		}

		void drawCursor(jcanvas::Graphics *g, int x, int y)
		{
			jcanvas::jrect_t<int> 
        partHorizontal = { 
          x + 1, y + 1, 4, 1 
        },
        partVertical = { 
          x + 1, y + 1, 1, 4 
        };

			FillRect(g, partHorizontal, 0xffff00ff);
			FillRect(g, partVertical, 0xffff00ff);
		}

		void drawPenSize(jcanvas::Graphics *g)
		{
      jcanvas::jpoint_t<int>
        wsize = GetSize();
			jcanvas::jrect_t<int> 
        rect = { wsize.x - BUTTON_SIZE, wsize.y - BUTTON_SIZE - 1, 0, 0 };

			switch (_pen_size) {
				case 1:
					rect.size = {1, 1};
					break;
				case 2:
					rect.size = {2, 2};
					break;
				case 4:
					rect.size = {3, 3};
					break;
				case 8:
					rect.size = {5, 5};
					break;
				case 16:
					rect.size = {7, 7};
					break;
				case 32:
					rect.size = {9, 9};
					break;
				default:
					break;
			}

			FillRect(g, rect, 0xff000000);
		}

		virtual bool KeyPressed(jcanvas::KeyEvent *event) 
		{
			if (jcanvas::Window::KeyPressed(event) == true) {
				return true;
			}

			jcanvas::jkeyevent_symbol_t s = event->GetSymbol();

			if (s == jcanvas::JKS_ENTER) {
				Clear();
			} else if (s == jcanvas::JKS_CURSOR_LEFT) {
				for (int i = BUTTON_COUNT; i--;) {
					if (_current_particle == _buttons[i].particleType) {
						if (i > 0) {
							_current_particle = _buttons[i-1].particleType;
						} else {
							_current_particle = _buttons[BUTTON_COUNT-1].particleType;
						}

						break;
					}
				}
			} else if (s == jcanvas::JKS_CURSOR_RIGHT) {
				for (int i = BUTTON_COUNT; i--;) {
					if (_current_particle == _buttons[i].particleType) {
						if (i < BUTTON_COUNT - 1) {
							_current_particle = _buttons[i+1].particleType;
						} else {
							_current_particle = _buttons[0].particleType;
						}

						break;
					}
				}
			} else if (s == jcanvas::JKS_CURSOR_UP) { // decrease pen size
				_pen_size *= 2;
				
				if (_pen_size > 32) {
					_pen_size = 32;
				}
			} else if (s == jcanvas::JKS_CURSOR_DOWN) { // increase pen size
				_pen_size /= 2;

				if (_pen_size < 1) {
					_pen_size = 1;
				}
			} else if (s == jcanvas::JKS_SPACE) {
				_current_particle = JPT_NOTHING;
			} else if (s == jcanvas::JKS_TAB) {
				_oil_density -= 0.05f;
				_salt_density -= 0.05f;
				_water_density -= 0.05f;
				_sand_density -= 0.05f;

				if (_oil_density < 0.05f) {
					_oil_density = 0.05f;
				}

				if (_salt_density < 0.05f) {
					_salt_density = 0.05f;
				}

				if (_water_density < 0.05f) {
					_water_density = 0.05f;
				}

				if (_sand_density < 0.05f) {
					_sand_density = 0.05f;
				}
			} else if (s == jcanvas::JKS_BACKSPACE) {
				_oil_density += 0.05f;
				_salt_density += 0.05f;
				_water_density += 0.05f;
				_sand_density += 0.05f;

				if (_oil_density > 1.0f) {
					_oil_density = 1.0f;
				}

				if (_salt_density > 1.0f) {
					_salt_density = 1.0f;
				}

				if (_water_density > 1.0f) {
					_water_density = 1.0f;
				}

				if (_sand_density > 1.0f) {
					_sand_density = 1.0f;
				}
			}

      jcanvas::jpoint_t<int>
        size = GetSize();
			jcanvas::jkeyevent_modifiers_t 
        m = event->GetModifiers();

			if (m & jcanvas::JKM_CONTROL) { 
				_mb_x = _old_x;
				_mb_y = _old_y;

				if (_old_y < (size.y-DASHBOARD_SIZE)) {
					DrawLine(_old_x+_speed_x,_old_y+_speed_y,_old_x,_old_y);
				}

				_is_button_down = true;
			} else if (m & jcanvas::JKM_ALTGR) { 
				_slow = true;
			} else if (m & jcanvas::JKM_SHIFT) { 
				_emit_oil ^= true;
				_emit_salt ^= true;
				_emit_water ^= true;
				_emit_sand ^= true;
			}

			if (s == jcanvas::JKS_0) { // eraser
				_current_particle = JPT_NOTHING;
			} else if (s == jcanvas::JKS_1) { // wall
				_current_particle = JPT_WALL;
			} else if (s == jcanvas::JKS_2) { // water
				_current_particle = JPT_WATER;
			} else if (s == jcanvas::JKS_3) { // dirty
				_current_particle = JPT_SAND;
			} else if (s == jcanvas::JKS_4) { // salt
				_current_particle = JPT_SALT;
			} else if (s == jcanvas::JKS_5) { // oil
				_current_particle = JPT_OIL;
			} else if (s == jcanvas::JKS_6) { // acid
				_current_particle = JPT_ACID;
			} else if (s == jcanvas::JKS_7) { // fire
				_current_particle = JPT_FIRE;
			} else if (s == jcanvas::JKS_8) { // electricity
				_current_particle = JPT_ELEC;
			} else if (s == jcanvas::JKS_9) { // torch
				_current_particle = JPT_TORCH;
			} else if (s == jcanvas::JKS_F1) { // mud
				_current_particle = JPT_MUD;
			} else if (s == jcanvas::JKS_F2) { // saltwater
				_current_particle = JPT_SALTWATER;
			} else if (s == jcanvas::JKS_F3) { // steam
				_current_particle = JPT_STEAM;
			} else if (s == jcanvas::JKS_F4) { // ice
				_current_particle = JPT_ICE;
			} else if (s == jcanvas::JKS_DELETE) { // clear screen
				Clear();
			} else if (s == jcanvas::JKS_v) { // enable or disable oil emitter
				_emit_oil ^= true;
			} else if (s == jcanvas::JKS_r) { // increase oil emitter density
				_oil_density += 0.05f;

				if (_oil_density > 1.0f) {
					_oil_density = 1.0f;
				}
			} else if (s == jcanvas::JKS_f) { // decrease oil emitter density
				_oil_density -= 0.05f;

				if (_oil_density < 0.05f) {
					_oil_density = 0.05f;
				}
			} else if (s == jcanvas::JKS_c) { // enable or disable salt emitter
				_emit_salt ^= true;
			} else if (s == jcanvas::JKS_e) { // increase salt emitter density
				_salt_density += 0.05f;

				if (_salt_density > 1.0f) {
					_salt_density = 1.0f;
				}
			} else if (s == jcanvas::JKS_d) { // decrease salt emitter density
				_salt_density -= 0.05f;

				if (_salt_density < 0.05f) {
					_salt_density = 0.05f;
				}
			} else if (s == jcanvas::JKS_z) { // enable or disable water emitter
				_emit_water ^= true;
			} else if (s == jcanvas::JKS_q) { // increase water emitter density
				_water_density += 0.05f;

				if (_water_density > 1.0f) {
					_water_density = 1.0f;
				}
			} else if (s == jcanvas::JKS_a) { // decrease water emitter density
				_water_density -= 0.05f;

				if (_water_density < 0.05f) {
					_water_density = 0.05f;
				}
			} else if (s == jcanvas::JKS_x) { // enable or disable dirt emitter
				_emit_sand ^= true;
			} else if (s == jcanvas::JKS_w) { // increase dirt emitter density
				_sand_density += 0.05f;

				if (_sand_density > 1.0f) {
					_sand_density = 1.0f;
				}
			} else if (s == jcanvas::JKS_s) { // decrease dirt emitter density
				_sand_density -= 0.05f;

				if (_sand_density < 0.05f) {
					_sand_density = 0.05f;
				}
			} else if (s == jcanvas::JKS_t) { // draw a bunch of random lines
				DoRandomLines(JPT_WALL);
			} else if (s == jcanvas::JKS_y) { // erase a bunch of random lines
				DoRandomLines(JPT_NOTHING);
			} else if (s == jcanvas::JKS_o) { // enable or disable particle swaps
				_implement_particle_swaps ^= true;
			}

			return true;
		}

		virtual bool KeyReleased(jcanvas::KeyEvent *event) 
		{
			if (jcanvas::Window::KeyReleased(event) == true) {
				return true;
			}

			jcanvas::jkeyevent_modifiers_t m = event->GetModifiers();

			if ((m & jcanvas::JKM_CONTROL) == 0) { 
				_mb_x = 0;
				_mb_y = 0;
				_is_button_down = false;
			} else if ((m & jcanvas::JKM_ALT) == 0) { 
				_slow = false;
			}

			return true;
		}

		virtual bool MousePressed(jcanvas::MouseEvent *event)
		{
			if (jcanvas::Window::MousePressed(event) == true) {
				return true;
			}

      jcanvas::jpoint_t<int>
        location = event->GetLocation();
      jcanvas::jpoint_t<int>
        size = GetSize();

			_old_x = location.x;
			_old_y = location.y;
			_mb_x = location.x;
			_mb_y = location.y;

			if (_mb_x < (size.y-DASHBOARD_SIZE)) {
				DrawLine(_mb_x, _mb_y, _old_x, _old_y);
			}

			_is_button_down = true;

			return true;
		}

		virtual bool MouseReleased(jcanvas::MouseEvent *event)
		{
			if (jcanvas::Window::MouseReleased(event) == true) {
				return true;
			}

      jcanvas::jpoint_t<int>
        location = event->GetLocation();
      jcanvas::jpoint_t<int>
        size = GetSize();

			if (_old_y < (size.y-DASHBOARD_SIZE)) {
				DrawLine(location.x, location.y, _old_x, _old_y);
			}

			_mb_x = 0;
			_mb_y = 0;
			_is_button_down = false;

			return true;
		}

		virtual bool MouseMoved(jcanvas::MouseEvent *event)
		{
			if (jcanvas::Window::MouseMoved(event) == true) {
				return true;
			}

      jcanvas::jpoint_t<int>
        location = event->GetLocation();
      jcanvas::jpoint_t<int>
        size = GetSize();

			if (_is_button_down == true) {
				DrawLine(location.x, location.y, _old_x, _old_y);
			}

			_old_x = location.x;
			_old_y = location.y;

			if (_mb_y > size.y-DASHBOARD_SIZE) {
				CheckGuiInteraction(_mb_x, _mb_y);
			}

			return true;
		}

		virtual void Paint(jcanvas::Graphics *g) 
		{
			jcanvas::Window::Paint(g);

      jcanvas::jpoint_t<int>
        size = GetSize();

			if (_slow) {
				if (_speed_x > 0) {
					_old_x += 1;
				} else if (_speed_x < 0) {
					_old_x += -1;
				}

				if (_speed_y > 0) {
					_old_y += 1;
				} else if (_speed_y < 0) {
					_old_y += -1;
				}
			} else {
				_old_x += _speed_x;
				_old_y += _speed_y;
			}

			if (_old_x < 0) {
				_old_x = 0;
			} else if (_old_x > size.x) {
				_old_x = size.x;
			}

			if (_old_y < 0) {
				_old_y = 0;
			} else if (_old_y > size.y) {
				_old_y = size.y;
			}

			//To emit or not to emit
			if (_emit_water) {
				Emit((size.x/2 - ((size.x/6)*2)), 20, JPT_WATER, _water_density);
			}

			if (_emit_sand) {
				Emit((size.x/2 - (size.x/6)), 20, JPT_SAND, _sand_density);
			}

			if (_emit_salt) {
				Emit((size.x/2 + (size.x/6)), 20, JPT_SALT, _salt_density);
			}

			if (_emit_oil) {
				Emit((size.x/2 + ((size.x/6)*2)), 20, JPT_OIL, _oil_density);
			}

			//If the button is pressed (and no event has occured since last frame due
			// to the polling procedure, then draw at the position (enabeling 'dynamic emitters')
			if (_is_button_down == true) {
				DrawLine(_old_x, _old_y, _old_x, _old_y);
			}

			//Clear bottom line
			for (int i=0; i<size.x; i++) {
				_vs[i + ((size.y - DASHBOARD_SIZE - 1)*size.x)] = JPT_NOTHING;
			}

			//Clear top line
			for (int i=0; i<size.x; i++) {
				_vs[i+((0)*size.x)] = JPT_NOTHING;
			}

			// Update the virtual screen (performing particle logic)
			UpdateVirtualScreen();

			// Map the virtual screen to the real screen
			_particle_count = 0;

			for (int y=size.y-DASHBOARD_SIZE; y--;) {
				for (int x=size.x; x--;) {
					int index = x + (size.x*y);
					jparticle_type_t same = _vs[index];

					if (same != JPT_NOTHING) {
						if (IsStillborn(same)) {
							g->SetRGB(colors[same], {x, y});
						} else {
							_particle_count++;
							if (same % 2 == 1) { // Moved 
								g->SetRGB(colors[(same-1)], {x, y});
								_vs[index] = (jparticle_type_t)(same-1); // Set it to not moved
							} else { // Not moved
								g->SetRGB(colors[same], {x, y});
							}
						}
					}
				}
			}

			// Update dashboard
			jcanvas::jrect_t<int> dashboard;

			dashboard = {
        .point = {
          .x = 0,
			    .y = size.y - DASHBOARD_SIZE
        },
        .size = {
			    .x = size.x,
			    .y = DASHBOARD_SIZE
        }
      };

			FillRect(g, dashboard, 0xff9b9b9b);

			FillRect(g, _buttons[0].rect, colors[JPT_WATER]);
			FillRect(g, _buttons[1].rect, colors[JPT_SAND]);
			FillRect(g, _buttons[2].rect, colors[JPT_SALT]);
			FillRect(g, _buttons[3].rect, colors[JPT_OIL]);
			FillRect(g, _buttons[4].rect, colors[JPT_FIRE]);
			FillRect(g, _buttons[5].rect, colors[JPT_ACID]);
			FillRect(g, _buttons[6].rect, colors[JPT_DIRT]);
			FillRect(g, _buttons[7].rect, colors[JPT_WATERSPOUT]);
			FillRect(g, _buttons[8].rect, colors[JPT_SANDSPOUT]);
			FillRect(g, _buttons[9].rect, colors[JPT_SALTSPOUT]);
			FillRect(g, _buttons[10].rect, colors[JPT_OILSPOUT]);
			FillRect(g, _buttons[11].rect, colors[JPT_WALL]);
			FillRect(g, _buttons[12].rect, colors[JPT_TORCH]);
			FillRect(g, _buttons[13].rect, colors[JPT_STOVE]);
			FillRect(g, _buttons[14].rect, colors[JPT_PLANT]);
			FillRect(g, _buttons[15].rect, colors[JPT_ICE]);
			FillRect(g, _buttons[16].rect, colors[JPT_IRONWALL]);
			FillRect(g, _buttons[17].rect, colors[JPT_VOID]);
			FillRect(g, _buttons[18].rect, 0);

			drawSelection(g);
			drawPenSize(g);
			drawCursor(g, _old_x, _old_y);
		}

		virtual void ShowApp() 
    {
      do {
        Repaint();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      } while (IsVisible() == true);
    }

};

int main(int argc, char **argv)
{
	jcanvas::Application::Init(argc, argv);

	Screen app;

	srand(time(NULL));

	app.SetTitle("Ball Drop");
	app.SetVisible(true);
  app.Exec();

	jcanvas::Application::Loop();

	return 0;
}
