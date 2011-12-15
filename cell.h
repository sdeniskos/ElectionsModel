/*!
* \author    denisko
* \date      14.12.2011 10:23:20
*/

#ifndef _ELECTIONS_CELL_H_
#define _ELECTIONS_CELL_H_

// ================================================================== INCLUDE
#include <boost/array.hpp>
// ===========================================================================
template<int numparties>
class Cell: public boost::array<double, numparties>
{
public:
	Cell operator + (const Cell& a)
	{
		Cell result;
		for(int i = 0, size = (int)this->size(); i < size; i++)
		{
			result[i] = (*this)[i] + a[i];
		}
		return result;
	}
	const Cell& operator += (const Cell& a)
	{
		for(int i = 0, size = (int)this->size(); i < size; i++)
		{
			(*this)[i]  += a[i];
		}
		return *this;
	}
	const Cell& operator *= (const Cell& a)
	{
		for(int i = 0, size = (int)this->size(); i < size; i++)
		{
			(*this)[i]  *= a[i];
		}
		return *this;
	}

	Cell operator*(const double& c)
	{
		Cell result;
		for(int i = 0, size = (int)this->size(); i < size; i++)
		{
			result[i] = (*this)[i]*c;
		}
		return result;
	}
	Cell operator/(const double& c)
	{
		Cell result;
		for(int i = 0, size = (int)this->size(); i < size; i++)
		{
			result[i] = (*this)[i]/c;
		}
		return result;
	}
	Cell operator - (const Cell& a)
	{
		Cell result;
		for(int i = 0, size = (int)this->size(); i < size; i++)
		{
			result[i] = (*this)[i] - a[i];
		}
		return result;
	}
	double sum() const
	{
		return std::accumulate(begin(),end(),0.0);
	}
	double min() const
	{
		return *std::min_element(begin(),end());
	}
public:	
	static double norm(const Cell& cell)
	{
		return cell.sum();
	}
	static double correlation(const Cell& a, const Cell& b)
	{
		Cell temp;
		for(int i = 0; i < temp.size(); i++)
		{
			temp[i] = fabs(a[i] - b[i])/numparties;
		}
		return 1.0 - std::min(temp.sum(),1.0);
	}
};

#endif //_ELECTIONS_CELL_H_
