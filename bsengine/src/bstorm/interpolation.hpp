#pragma once

namespace bstorm
{
	float GetInterpolateResult(int ipType, float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeLinear(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInQuad(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInCubic(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInQuart(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInQuint(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeOutQuad(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeOutCubic(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeOutQuart(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeOutQuint(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInOutQuad(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInOutCubic(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInOutQuart(float ipStart, float ipEnd, int ipFrame, int ipTime);
	float easeInOutQuint(float ipStart, float ipEnd, int ipFrame, int ipTime);
}