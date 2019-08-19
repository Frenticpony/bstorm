#include <bstorm/thecl.hpp>

#include <d3dx9.h>

namespace bstorm
{
	
	ECLPattern::ECLPattern() :
		pat_counter(0),
		pat_complete(false) {}
	ECLPattern::~ECLPattern()
	{
	}
	void ECLPattern::Apply(MoveModeECL* eclMode)
	{
	}

	ECLPattern_SPUP::ECLPattern_SPUP(bool _isWait, int _time, float _accel) :
		pat_isWait(_isWait),
		pat_time(_time),
		pat_accel(_accel){}
	void ECLPattern_SPUP::Apply(MoveModeECL* eclMode)
	{
		if (pat_counter == 0)
		{
			eclMode->SetAcceleration(pat_accel);
			pat_counter++;
		}
		else if (pat_counter < pat_time)
		{
			pat_counter++;
		}

		if (pat_counter == pat_time)
		{
			eclMode->SetAcceleration(0);
			pat_complete = true;
		}
	}

	ECLPattern_ANGVEL::ECLPattern_ANGVEL(bool _isWait, int _time, float _angvel) :
		pat_isWait(_isWait),
		pat_time(_time),
		pat_angvel(_angvel) {}
	void ECLPattern_ANGVEL::Apply(MoveModeECL* eclMode)
	{
		if (pat_counter == 0)
		{
			eclMode->SetAngularVelocity(pat_angvel);
			pat_counter++;
		}
		else if (pat_counter < pat_time)
		{
			pat_counter++;
		}

		if (pat_counter == pat_time)
		{
			eclMode->SetAngularVelocity(0);
			pat_complete = true;
		}
	}

	ECLDefinition::ECLDefinition() :
		etamaStyle(0),
		x(0),
		y(0),
		speed(1),
		maxSpeed(999),
		minSpeed(0),
		angle(0),
		graphic(0),
		delay(0),
		count(1),
		layer(1) {}
	ECLDefinition::~ECLDefinition() {}
	void ECLDefinition::SetInitProperties(int _style, float _x, float _y, float _speed, float _angle, int _graphic, int _delay, int _count, int _layer)
	{
		etamaStyle = _style;
		x = _x;
		y = _y,
		speed = _speed;
		angle = _angle;
		graphic = _graphic;
		delay = _delay;
		count = _count;
		layer = _layer;
	}
	std::list<std::shared_ptr<ECLPattern>> ECLDefinition::GetPatternList()
	{
		return pat_list;
	}
	void ECLDefinition::AddPattern(std::shared_ptr<ECLPattern> newPat)
	{
		pat_list.push_back(newPat);
	}

	ECLStorage::ECLStorage(const std::shared_ptr<Package>& package) :
		Obj(package)
	{
		for (int i = 0; i < 9; i++)
		{
			ecl_list.push_back(std::make_shared<ECLDefinition>());
		}
	}
	ECLStorage::~ECLStorage(){}
	void ECLStorage::ECL_InitDefinition() {}
	std::shared_ptr<ECLDefinition> ECLStorage::ECL_GetDefinition(int _index)
	{
		auto it = ecl_list.begin();
		if (_index > 0)
		{
			std::advance(it, _index);
		}
		auto def = *it;
		return def;
	}
	void ECLStorage::ECL_SetMoveObject(int _obj) { ecl_move_obj = _obj; }
	int ECLStorage::ECL_GetMoveObject() const { return ecl_move_obj; }

	MoveModeECL::MoveModeECL(){}
	void MoveModeECL::Move(float & x, float & y)
	{
		//EX_WAIT
		patApplied = false;
		auto pattern_counter = ecl_data.begin();
		std::advance(pattern_counter, pattern_count);
		if (pattern_counter != ecl_data.end())
		{
			while (!patApplied)
			{
				if (pattern_counter != ecl_data.end()) { patApplied = true; }
				auto pat = *pattern_counter;
				if (pat->IsPatternComplete() == false)
				{
					if (pat->IsWait() == true)
					{
						PushToNoWait(pat);
						pattern_counter = ecl_data.erase(pattern_counter);
						++pattern_counter;
						++pattern_count;
					}
					else
					{
						pat->Apply(this);
						patApplied = true;
					}
				}
				else
				{
					//pattern_counter = ecl_data.erase(pattern_counter);
					++pattern_counter;
					++pattern_count;
				}
			}
		}

		//EX_NOWAIT
		auto pattern_counter_n = ecl_noWaitData.begin();
		while (pattern_counter_n != ecl_noWaitData.end())
		{
			auto pat_n = *pattern_counter_n;
			if (pat_n->IsPatternComplete() == false)
			{
				pat_n->Apply(this);
			}
			else
			{
				pattern_counter_n = ecl_noWaitData.erase(pattern_counter_n);
			}
			++pattern_counter_n;
		}

		mv_speed += mv_acceleration;
		if (mv_acceleration != 0)
		{
			if (mv_speed > mv_maxSpeed)
			{
				mv_speed = mv_maxSpeed;
			}
			else if (mv_speed < mv_minSpeed)
			{
				mv_speed = mv_minSpeed;
			}
		}
		mv_angle += mv_angularVelocity;
		float rad = D3DXToRadian(mv_angle);
		float dx = mv_speed * cos(rad);
		float dy = mv_speed * sin(rad);
		x += dx;
		y += dy;
	}
	void MoveModeECL::SetData(const std::list<std::shared_ptr<ECLPattern>>& loc_data)
	{
		std::list<std::shared_ptr<ECLPattern>> pattern;

		auto pattern_counter_n = loc_data.begin();
		while (pattern_counter_n != loc_data.end())
		{
			auto cur_pat = *pattern_counter_n;
			auto copy_pat = cur_pat->clone();
			pattern.push_back(copy_pat);
			++pattern_counter_n;
		}
		ecl_data = pattern;
	}
	void MoveModeECL::PushToNoWait(const std::shared_ptr<ECLPattern>& loc_data)
	{
		ecl_noWaitData.push_back(loc_data);
	}
}