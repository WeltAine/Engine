#pragma once

//步进时间，或者叫时间长度，总之用来记录经过了多长时间
namespace Ayin {


	struct Timestep {

	public:
		Timestep(float time) 
			: m_TimeDuration(time)
		{}

		/// <summary>
		/// Timestep -> float转换，默认转换为秒（目前的成员来说，它其实就是一个float）
		/// </summary>
		operator float() const { return m_TimeDuration; }//? 占用一次隐式转换么
		//x 当然你可以选择更费劲的方法，全局域写两个operator +()，（写两个是为了对调参数，保证支持交换律，这也是不在Timestep中这么写的原因）

		float GetSeconds() const { return m_TimeDuration; };
		float GetMilliseconds() const { return m_TimeDuration * 1000.0f; };

	private:

		float m_TimeDuration;

	};

}