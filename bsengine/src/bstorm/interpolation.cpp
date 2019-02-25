/*
*
* TERMS OF USE - EASING EQUATIONS
*
* Open source under the BSD License.
*
* Copyright © 2001 Robert Penner
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of
* conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list
* of conditions and the following disclaimer in the documentation and/or other materials
* provided with the distribution.
*
* Neither the name of the author nor the names of contributors may be used to endorse
* or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <bstorm/interpolation.hpp>

#include <algorithm>

namespace bstorm
{
	float GetInterpolateResult(int ipType, float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ipResult = 0.0f;
		switch (ipType)
		{
			case 0:  return easeLinear(ipStart, ipEnd, ipFrame, ipTime);     //IP_LINEAR_1X
			case 1:  return easeInQuad(ipStart, ipEnd, ipFrame, ipTime);     //IP_ACCEL_2X
			case 2:  return easeInCubic(ipStart, ipEnd, ipFrame, ipTime);    //IP_ACCEL_3X
			case 3:  return easeInQuart(ipStart, ipEnd, ipFrame, ipTime);    //IP_ACCEL_4X
			case 4:  return easeInQuint(ipStart, ipEnd, ipFrame, ipTime);    //IP_ACCEL_5X
			case 5:  return easeOutQuad(ipStart, ipEnd, ipFrame, ipTime);    //IP_DECEL_2X
			case 6:  return easeOutCubic(ipStart, ipEnd, ipFrame, ipTime);   //IP_DECEL_3X
			case 7:  return easeOutQuart(ipStart, ipEnd, ipFrame, ipTime);   //IP_DECEL_4X
			case 8:  return easeOutQuint(ipStart, ipEnd, ipFrame, ipTime);   //IP_DECEL_5X
			case 9:  return easeInOutQuad(ipStart, ipEnd, ipFrame, ipTime);  //IP_SMOOTH_2X
			case 10: return easeInOutCubic(ipStart, ipEnd, ipFrame, ipTime); //IP_SMOOTH_3X
			case 11: return easeInOutQuart(ipStart, ipEnd, ipFrame, ipTime); //IP_SMOOTH_4X
			case 12: return easeInOutQuint(ipStart, ipEnd, ipFrame, ipTime); //IP_SMOOTH_5X
			default: return easeLinear(ipStart, ipEnd, ipFrame, ipTime);
		}
	}

	//Linear
	float easeLinear(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / (float)ipTime;
		return (ipEnd * ip_v) + (ipStart * (1.0f - ip_v));
	}
	
	//Acceleration X2
	float easeInQuad(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / (float)ipTime;
		float ip_v_d = ipEnd - ipStart;
		return ip_v_d * (ip_v)* ip_v + ipStart;
	}

	//Acceleration X3
	float easeInCubic(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / (float)ipTime;
		float ip_v_d = ipEnd - ipStart;
		return ip_v_d * ip_v * ip_v * ip_v + ipStart;
	}

	//Acceleration X4
	float easeInQuart(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / (float)ipTime;
		float ip_v_d = ipEnd - ipStart;
		return ip_v_d * ip_v * ip_v * ip_v * ip_v + ipStart;
	}

	//Acceleration X5
	float easeInQuint(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / (float)ipTime;
		float ip_v_d = ipEnd - ipStart;
		return ip_v_d * ip_v * ip_v * ip_v * ip_v * ip_v + ipStart;
	}

	//Deceleration X2
	float easeOutQuad(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / (float)ipTime;
		float ip_v_d = ipEnd - ipStart;
		return -ip_v_d * (ip_v) * (ip_v - 2.0f) + ipStart;
	}

	//Deceleration X3
	float easeOutCubic(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = (ipFrame / (float)ipTime) - 1.0f;
		float ip_v_d = ipEnd - ipStart;
		return ip_v_d * (ip_v * ip_v * ip_v + 1.0f) + ipStart;
	}

	//Deceleration X4
	float easeOutQuart(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = (ipFrame / (float)ipTime) - 1.0f;
		float ip_v_d = ipEnd - ipStart;
		return -ip_v_d * (ip_v * ip_v * ip_v * ip_v - 1.0f) + ipStart;
	}

	//Deceleration X5
	float easeOutQuint(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = (ipFrame / (float)ipTime) - 1.0f;
		float ip_v_d = ipEnd - ipStart;
		return ip_v_d * (ip_v * ip_v * ip_v * ip_v * ip_v + 1.0f) + ipStart;
	}

	//Smoothstep X2
	float easeInOutQuad(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / ((float)ipTime / 2.0f);
		float ip_v_d = ipEnd - ipStart;
		if (ip_v < 1.0f)
		{
			return ip_v_d / 2.0f * ip_v * ip_v + ipStart;
		}
		ip_v--;
		return -ip_v_d / 2.0f * (ip_v * (ip_v - 2.0f) - 1.0f) + ipStart;
	}

	//Smoothstep X3
	float easeInOutCubic(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / ((float)ipTime / 2.0f);
		float ip_v_d = ipEnd - ipStart;
		if (ip_v < 1.0f)
		{
			return ip_v_d / 2.0f * ip_v * ip_v * ip_v + ipStart;
		}
		ip_v -= 2.0f;
		return ip_v_d / 2.0f * (ip_v * ip_v * ip_v + 2.0f) + ipStart;
	}

	//Smoothstep X4
	float easeInOutQuart(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / ((float)ipTime / 2.0f);
		float ip_v_d = ipEnd - ipStart;
		if (ip_v < 1.0f)
		{
			return ip_v_d / 2.0f * ip_v * ip_v * ip_v * ip_v + ipStart;
		}
		ip_v -= 2.0f;
		return -ip_v_d / 2.0f * (ip_v * ip_v * ip_v * ip_v - 2.0f) + ipStart;
	}

	//Smoothstep X5
	float easeInOutQuint(float ipStart, float ipEnd, int ipFrame, int ipTime)
	{
		float ip_v = ipFrame / ((float)ipTime / 2.0f);
		float ip_v_d = ipEnd - ipStart;
		if (ip_v < 1.0f)
		{
			return ip_v_d / 2.0f * ip_v * ip_v * ip_v * ip_v * ip_v + ipStart;
		}
		ip_v -= 2.0f;
		return ip_v_d / 2.0f * (ip_v * ip_v * ip_v * ip_v * ip_v + 2.0f) + ipStart;
	}
}

