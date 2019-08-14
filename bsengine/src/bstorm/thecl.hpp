#pragma once

#include <memory>
#include <list>

#include <bstorm/logger.hpp>

namespace bstorm
{
	class MoveModeECL;

	class ECLPattern
	{
	public:
		ECLPattern();
		virtual ~ECLPattern();
		virtual void Apply(MoveModeECL* eclMove);
		void TickCounter() { pat_counter--; }
		int GetCounter() const { return pat_counter; }
		bool IsPatternComplete() const { return pat_complete; }
		int pat_counter;
		bool pat_complete;
	};

	class ECLPattern_Initialize : public ECLPattern
	{
	public:
		ECLPattern_Initialize(float _speed, float _angle);
		void Apply(MoveModeECL* eclMove);
		float pat_speed;
		float pat_angle;
	};
	class ECLPattern_SPUP : public ECLPattern
	{
	public:
		ECLPattern_SPUP(bool _isWait, int _time, float _accel);
		void Apply(MoveModeECL* eclMove);
		bool pat_isWait;
		int pat_time;
		float pat_accel;
	};

	class MoveModeECL
	{
	public:
		MoveModeECL();
		void Move(float& x, float& y);
		void SetSpeed(float loc_speed) { mv_speed = loc_speed; };
		void SetAngle(float loc_angle) { mv_angle = loc_angle; };
		void SetMinSpeed(float loc_minSpeed) { mv_minSpeed = loc_minSpeed;};
		void SetMaxSpeed(float loc_maxSpeed) { mv_maxSpeed = loc_maxSpeed; };
		void SetAngularVelocity(float loc_angularVelocity) { mv_angularVelocity = loc_angularVelocity; };
		void SetAcceleration(float loc_acceleration) { mv_acceleration = loc_acceleration; };
		void SetData(const std::list<std::shared_ptr<ECLPattern>>& loc_data);
		std::list<std::shared_ptr<ECLPattern>> ecl_data;
		std::shared_ptr<ECLPattern> current_pattern;
		int pattern_counter;
	private:
		float mv_speed;
		float mv_angle;
		float mv_minSpeed;
		float mv_maxSpeed;
		float mv_angularVelocity;
		float mv_acceleration;
	};

	class ECLStorage
	{
	public:
		std::list<std::list<std::shared_ptr<ECLPattern>>> ecl_list;
		void CreateTestList(std::list<std::shared_ptr<ECLPattern>> testList);
	};
}