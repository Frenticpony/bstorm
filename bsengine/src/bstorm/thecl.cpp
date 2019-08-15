#include <bstorm/thecl.hpp>

#include <d3dx9.h>

namespace bstorm
{
	ECLPattern::ECLPattern() :
		pat_counter(0),
		pat_complete(false) {}
	ECLPattern::~ECLPattern() {}
	void ECLPattern::Apply(MoveModeECL* eclMode)
	{
	}

	ECLPattern_Initialize::ECLPattern_Initialize(float _speed, float _angle) :
		pat_speed(_speed),
		pat_angle(_angle){}
	void ECLPattern_Initialize::Apply(MoveModeECL* eclMode)
	{
		eclMode->SetSpeed(pat_speed);
		eclMode->SetAngle(pat_angle);
		eclMode->SetMinSpeed(0.0f);
		eclMode->SetMaxSpeed(999.0f);
		pat_complete = true;
	}

	ECLPattern_SPUP::ECLPattern_SPUP(bool _isWait, int _time, float _accel) :
		pat_isWait(_isWait),
		pat_time(_time),
		pat_accel(_accel){}
	void ECLPattern_SPUP::Apply(MoveModeECL* eclMode)
	{
		if (pat_counter == 0)
		{
			Logger::Write(Log(LogLevel::LV_USER).Msg("APPLY ACCEL"));
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
			Logger::Write(Log(LogLevel::LV_USER).Msg("APPLY ANGVEL"));
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

	MoveModeECL::MoveModeECL() : 
		mv_speed(0),
		mv_angle(0),
		mv_minSpeed(0),
		mv_maxSpeed(0),
		mv_angularVelocity(0),
		mv_acceleration(0),
		pattern_counter(0){}
	void MoveModeECL::Move(float & x, float & y)
	{
		//EX_WAIT
		auto pattern_counter = ecl_data.begin();
		if (pattern_counter != ecl_data.end())
		{
			auto pat = *pattern_counter;
			if (pat->IsPatternComplete() == false)
			{
				if (pat->IsWait() == true)
				{
					PushToNoWait(pat);
					pattern_counter = ecl_data.erase(pattern_counter);
					++pattern_counter;
				}
				else
				{
					pat->Apply(this);
				}
			}
			else
			{
				pattern_counter = ecl_data.erase(pattern_counter);
				++pattern_counter;
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
		ecl_data = loc_data;
	}
	void MoveModeECL::PushToNoWait(const std::shared_ptr<ECLPattern>& loc_data)
	{
		ecl_noWaitData.push_back(loc_data);
	}
	
	ECLStorage::ECLStorage(const std::shared_ptr<Package>& package) :
		Obj(package)
	{
	}

	ECLStorage::~ECLStorage()
	{
	}

	void ECLStorage::CreateTestList(std::list<std::shared_ptr<ECLPattern>> testList)
	{
	}
}