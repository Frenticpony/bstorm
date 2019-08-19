#pragma once

#include <memory>
#include <list>

#include <bstorm/obj.hpp>
#include <bstorm/logger.hpp>

namespace bstorm
{
	class Obj;
	class ObjMove;
	class Package;
	class MoveModeECL;

	class ECLPattern
	{
	public:
		ECLPattern();
		virtual ~ECLPattern();
		virtual void Apply(MoveModeECL* eclMove);
		virtual std::shared_ptr<ECLPattern> clone() const
		{
			return std::shared_ptr<ECLPattern>(new ECLPattern(*this));
		}
		virtual bool IsWait() const { return pat_isWait; }
		void TickCounter() { pat_counter--; }
		int GetCounter() const { return pat_counter; }
		bool IsPatternComplete() const { return pat_complete; }
		int pat_counter;
		bool pat_complete;
		bool pat_isWait;
	};

	//class ECLPattern_QUICKDECELERATE
	//class ECLPattern_EFFON
	//class ECLPattern_GRAVITY
	class ECLPattern_SPUP : public ECLPattern
	{
	public:
		ECLPattern_SPUP(bool _isWait, int _time, float _accel);
		void Apply(MoveModeECL* eclMove);
		std::shared_ptr<ECLPattern> clone() const
		{
			return std::shared_ptr<ECLPattern_SPUP>(new ECLPattern_SPUP(*this));
		}
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
		std::shared_ptr<ECLPattern> clone() const
		{
			return std::shared_ptr<ECLPattern_ANGVEL>(new ECLPattern_ANGVEL(*this));
		}
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

	class ECLDefinition
	{
	public:
		ECLDefinition();
		~ECLDefinition();
		std::list<std::shared_ptr<ECLPattern>> pat_list;
		void SetPatternList(std::list<std::shared_ptr<ECLPattern>> _pat_list) { pat_list = _pat_list; };
		std::list<std::shared_ptr<ECLPattern>> GetPatternList();
		void AddPattern(std::shared_ptr<ECLPattern> newPat);

		void SetInitProperties(int _style, float _x, float _y, float _speed, float _angle, int _graphic, int _delay, int _count, int _layer);

		void SetInitStyle(int _style) { etamaStyle = _style; };
		void SetInitX(float _x) { x = _x; };
		void SetInitY(float _y) { y = _y; };
		void SetInitSpeed(float _speed) { speed = _speed; };
		void SetInitMaxSpeed(float _maxSpeed) { maxSpeed = _maxSpeed; };
		void SetInitMinSpeed(float _minSpeed) { minSpeed = _minSpeed; };
		void SetInitAngle(float _angle) { angle = _angle; };
		void SetInitGraphic(int _graphic) { graphic = _graphic; };
		void SetInitDelay(int _delay) { delay = _delay; };
		void SetInitCount(int _count) { count = _count; };
		void SetInitLayer(int _layer) { layer = _layer; };

		int GetInitStyle() { return etamaStyle; };
		float GetInitX() { return x; };
		float GetInitY() { return y; };
		float GetInitSpeed() { return speed; };
		float GetInitMaxSpeed() { return maxSpeed; };
		float GetInitMinSpeed() { return minSpeed; };
		float GetInitAngle() { return angle; };
		int GetInitGraphic() { return graphic; };
		int GetInitDelay() { return delay; };
		int GetInitCount() { return count; };
		int GetInitLayer() { return layer; };

		int etamaStyle;
		float x;
		float y;
		float speed;
		float maxSpeed;
		float minSpeed;
		float angle;
		int graphic;
		int delay;
		float radius;
		int count;
		int layer;
		float xofs;
		float yofs;
		int blend;
		int filter;
		float layer_angle_mod;
		float sub1;
		float sub2;
		float sub3;
		float sub4;
	};

	class ECLStorage : public Obj
	{
	public:
		ECLStorage(const std::shared_ptr<Package>& package);
		~ECLStorage();
		void ECL_InitDefinition();
		std::shared_ptr<ECLDefinition> ECL_GetDefinition(int _index);
		void ECL_SetMoveObject(int _obj);
		int ECL_GetMoveObject() const;
		int ecl_move_obj;
		std::list<std::shared_ptr<ECLDefinition>> ecl_list;
	};

	class MoveModeECL
	{
	public:
		MoveModeECL();
		void Move(float& x, float& y);
		void SetSpeed(float loc_speed) { mv_speed = loc_speed; };
		void SetAngle(float loc_angle) { mv_angle = loc_angle; };
		void SetMinSpeed(float loc_minSpeed) { mv_minSpeed = loc_minSpeed; };
		void SetMaxSpeed(float loc_maxSpeed) { mv_maxSpeed = loc_maxSpeed; };
		void SetAngularVelocity(float loc_angularVelocity) { mv_angularVelocity = loc_angularVelocity; };
		void SetAcceleration(float loc_acceleration) { mv_acceleration = loc_acceleration; };
		void SetData(const std::list<std::shared_ptr<ECLPattern>>& loc_data);
		void PushToNoWait(const std::shared_ptr<ECLPattern>& loc_data);
		std::list<std::shared_ptr<ECLPattern>> ecl_data;
		std::list<std::shared_ptr<ECLPattern>> ecl_noWaitData;
		std::shared_ptr<ECLPattern> current_pattern;
		int pattern_count = 0;
		bool patApplied;
	private:
		float mv_speed;
		float mv_angle;
		float mv_minSpeed;
		float mv_maxSpeed;
		float mv_angularVelocity;
		float mv_acceleration;
	};
}