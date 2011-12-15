/*!
* \author    denisko
* \date      14.12.2011 10:33:20
*/


#ifndef _ELECTIONS_AGITATION_WORKS_H_
#define _ELECTIONS_AGITATION_WORKS_H_
// ===========================================================================
#include <cv.h>
// ===========================================================================
struct AgitationRecord
{
	int partNum;
	double strength;
	CvPoint2D32f center;
	double radius;
	int period;
	int time;
};

struct AgitationWorks
{
	typedef std::pair<double,double> StrengthBorders;
	typedef std::vector<int> IntVector;
	typedef std::vector<double> DoublesVector;
	typedef std::vector<StrengthBorders> StrengthBordersVector;
	AgitationWorks()
	{
		;
	}
	void addBorders(IntVector& indexBorders, StrengthBordersVector& strengthDistribution, 
		DoublesVector& radiusDistribution, IntVector& periodsDistribution)
	{
		m_indexBorders = indexBorders;
		m_indexRange = 0;
		for(int i = 0; i < m_indexBorders.size(); i++)
		{
			m_indexBorders[i] += m_indexRange;
			m_indexRange = m_indexBorders[i];
		}


		m_strengthDistribution = strengthDistribution;
		m_radiusDistribution = radiusDistribution;
		m_periodsDistribution = periodsDistribution;
	}
	
	AgitationRecord generateRecord()
	{
		AgitationRecord result;
		result.partNum = getPartNum();
		result.period = getPeriod(result.partNum);
		result.radius = getRadius(result.partNum);
		result.strength = getStrength(result.partNum);
		return result;
	}
private: //service functions
	int getPartNum() const
	{
		int index = rand() % m_indexRange;
		int result = 0;
		for(int i = 0; i < m_indexBorders.size(); i++,result++)
		{
			if(index < m_indexBorders[i])
				break;
		}
		return result;
	}
	int getPeriod(int partNum) const
	{
		return rand()%m_periodsDistribution[partNum];
	}
	int getRadius(int partNum) const
	{
		return m_radiusDistribution[partNum];
	}
	double getStrength(int partNum) const
	{
		static const int range = 100;
		int value = rand()%range;
		return m_strengthDistribution[partNum].first + (m_strengthDistribution[partNum].second - 
			m_strengthDistribution[partNum].first)*value/range;
	}

private:
	std::vector<int> m_indexBorders;
	int m_indexRange;
	std::vector<StrengthBorders> m_strengthDistribution;
	std::vector<int> m_periodsDistribution;
	std::vector<double> m_radiusDistribution;
};


#endif //_ELECTIONS_AGITATION_WORKS_H_
