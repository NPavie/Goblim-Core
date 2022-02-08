/**
 * @author     G.Gilet, Guillaume.Gilet@unilim.fr
 * @date       January 1, 2013
 * @version    1.0 (alpha)
 */



#ifndef _EFFECT_
#define _EFFECT_
#include <string>
/**
 * @brief      Abstract base to define global effect such as post-processing, composition of frames, intermediate computations...
 *
 * @details    TO DO : Usage scenario
 *
 */
class Effect
{
	public:
	 /**
     * @brief      Creates a new Effect.
     */
		Effect();
 	/**
     * @brief      Creates a new Effect under name className.
     * @param className the name of the effect
     */
		Effect(std::string name,std::string className = "Unknown");
 	/**
     * @brief      Destroy the effect.
     */

		~Effect();

 /**
     * @brief Apply the effect. See ...
     */
		virtual void apply();

 	/**
     * @brief  Return the name of the effect
     * @return the name of the effect
     */
		const std::string getName();

		bool valid;

	protected:
		std::string m_Name;
		std::string m_ClassName;

};




#endif