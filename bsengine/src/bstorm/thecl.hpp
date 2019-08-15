#pragma once

#include <memory>
#include <list>

#include <bstorm/obj.hpp>
#include <bstorm/logger.hpp>

namespace bstorm
{
	class Obj;
	class Package;
	class MoveModeECL;

	class ECLPattern
	{
	public:
		ECLPattern();
		virtual ~ECLPattern();
		virtual void Apply(MoveModeECL* eclMove);
		virtual bool IsWait() const { return pat_isWait; }
		void TickCounter() { pat_counter--; }
		int GetCounter() const { return pat_counter; }
		bool IsPatternComplete() const { return pat_complete; }
		int pat_counter;
		bool pat_complete;
		bool pat_isWait;
	};

	class ECLPattern_Initialize : public ECLPattern
	{
	public:
		ECLPattern_Initialize(float _speed, float _angle);
		void Apply(MoveModeECL* eclMove);
		bool pat_isWait;
		//int source obj
		//int etamaStyle
		//float x
		//float y
		float pat_speed;
		float pat_angle;
		//int graphic
		//int delay
		//float radius
		//int count
		//int layer
		//float xofs
		//float yofs
		//int blend
		//int filter
		//float layer_angle_mod
		//float sub1
		//float sub2
		//float sub3
		//float sub4
	};
	//class ECLPattern_QUICKDECELERATE
	//class ECLPattern_EFFON
	//class ECLPattern_GRAVITY
	class ECLPattern_SPUP : public ECLPattern
	{
	public:
		ECLPattern_SPUP(bool _isWait, int _time, float _accel);
		void Apply(MoveModeECL* eclMove);
		bool IsWait() const override { return pat_isWait; }
		bool pat_isWait;
		int pat_time;
		float pat_accel;
	};
	class ECLPattern_ANGVEL : public ECLPattern
	{
	public:
		ECLPattern_ANGVEL(bool _isWait, int _time, float _angvel);
		void Apply(MoveModeECL* eclMove);
		bool IsWait() const override { return pat_isWait; }
		bool pat_isWait;
		int pat_time;
		float pat_angvel;
	};
	//class ECLPattern_ANGMOD_RELATIVE
	//class ECLPattern_ANGMOD_ABSOLUTE
	//class ECLPattern_ANGMOD_AIMED
	//class ECLPattern_REFLECT
	//class ECLPattern_ORBIT
	//class ECLPattern_ETON_BORDER
	//class ECLPattern_ETON_FRAME
	//class ECLPattern_ETON_DELETE
	//class ECLPattern_DELETE
	//class ECLPattern_DELETE_WITH_OBJ

	//class ECLPattern_ARMOR
	//class ECLPattern_NOTOUT
	//class ECLPattern_BLEND
	//class ECLPattern_GRAPHIC_INST
	//class ECLPattern_GRAPHIC_EFON
	//class ECLPattern_SE
	//class ECLPattern_WAIT
	//class ECLPattern_LOOP

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
		void PushToNoWait(const std::shared_ptr<ECLPattern>& loc_data);
		std::list<std::shared_ptr<ECLPattern>> ecl_data;
		std::list<std::shared_ptr<ECLPattern>> ecl_noWaitData;
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

	class ECLStorage : public Obj
	{
	public:
		ECLStorage(const std::shared_ptr<Package>& package);
		~ECLStorage();
		void ECL_InitPattern();
		void ECL_AddPattern();
		void ECL_FireFromObj();
		//void ECL_FireFromPosition();
		//void ECL_SetProperties();
		//void ECL_SetSpeed();
		//void ECL_SetAngle();
		//void ECL_SetGraphic();
		//void ECL_SetDelay();
		//void ECL_SetRadius();
		//void ECL_SetCount();
		//void ECL_SetLayer();
		//void ECL_SetMultishot();
		//void ECL_SetXOffset();
		//void ECL_SetYOffset();
		//void ECL_SetOffset();
		//void ECL_SetBlend();
		//void ECL_SetFilter();
		//void ECL_SetLayerAngleDifference();
		//void ECL_SetSubData();
		std::list<std::list<std::shared_ptr<ECLPattern>>> ecl_list;
		void CreateTestList(std::list<std::shared_ptr<ECLPattern>> testList);
	};
}