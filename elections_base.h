#ifndef _ELECTIONS_BASE
#define _ELECTIONS_BASE
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/array.hpp>
#include <numeric>
#include <algorithm>
#include <deque>
#include <cv.h>
#include "cell.h"
#include "agitation_works.h"


struct Elections
{

public:
	Elections(CvSize outputImageSize):
	  m_field(m_size,m_size),
	  m_tempField(m_size,m_size),
	  m_linkingField(m_size, m_size),
	  m_agitationField(m_size,m_size),
	  m_dt(0.1),
	  m_linkCoefficient(0.4),
	  m_decreasePotential(0.5)
	{
		m_base = cvCreateImage(cvSize(m_size,m_size),IPL_DEPTH_8U,4);
		m_output = cvCreateImage(outputImageSize, IPL_DEPTH_8U,4);
		srand(GetTickCount());
		for(int y = 0; y < m_linkingField.size1(); y++)
		{
			for(int x =0; x < m_linkingField.size2(); x++)
			{
				m_linkingField(y,x) = m_linkCoefficient;
			}
		}
		m_colors.resize(Elections::m_numParties);
		CvScalar colors[4] =  { cvScalar(0,1.0,0), cvScalar(0,0,1.0), cvScalar(0.8,0.8,0.8),cvScalar(0.7,0.3,0.7)};
		for(int i =0; i < 4; i++)
		{
			m_colors[i] = colors[i];
		}
		
	}
	~Elections()
	{
		cvReleaseImage(&m_base);
		cvReleaseImage(&m_output);
	}
   
    void init()
	{
		printf("init \n");
		for(int y = 0; y < (int)m_field.size1(); y++)
		{
			for(int x = 0; x < (int)m_field.size2(); x++)
			{
				m_field(y,x) = assignInitialWeight(y,x);
				std::fill(m_agitationField(y,x).begin(), m_agitationField(y,x).end(),0);
			}
		}
		m_agitationDeque.clear();
		m_step = 0;
	}

	void setAgigatationWorks(const AgitationWorks& ag)
	{
		m_agitationWorks = ag;
	}

	void processingStep()
	{
		printf("processing step %d \n", m_step);
		m_step++;
		processAgitationField(m_step);
		propagate();
		renorm();
	}

	void prepareImage()
	{
		fillImage(m_base);
		cvResize(m_base, m_output);
	
	}
	void loadDiffusionMap(IplImage* image)
	{
		IplImage* temp = cvCreateImage(cvSize(image->width, image->height),IPL_DEPTH_8U,4);
		cvCvtColor(image, temp,CV_RGB2RGBA);
		cvResize(temp,m_base);
		IplImage* grayTemp = cvCreateImage(cvSize(m_base->width, m_base->height),IPL_DEPTH_8U,1);
		cvCvtColor(m_base, grayTemp, CV_RGBA2GRAY);
	   	for(int y = 0; y < grayTemp->height; y++)
		{
			unsigned char* row = reinterpret_cast<unsigned char*>(grayTemp->imageData+ y * grayTemp->widthStep);
			
			for(int x = 0; x < grayTemp->width; x++)
			{
				m_linkingField(y,x) = 1.0 - ((double)row[x])/255;
				if(x > grayTemp->width*3/4)
				{
					x =x;
					double c  =m_linkingField(y,x);
					c  = c;
				}
				
				
				if(m_linkingField(y,x) < 0.7)
				{
					 y = y;
				}
			}
		}
		cvReleaseImage(&temp);
		cvReleaseImage(&grayTemp);
		visualizeDispersionMap(m_base);
		cvResize(m_base, m_output);
	}


	IplImage* outputImage()
	{
		return m_output;
	}
public: //drawning
	void fillImage(IplImage* image)
	{
		
		for(int y = 0; y < image->height; y++)
		{
			char* row = image->imageData+ y * image->widthStep;
			
			for(int x = 0; x < image->width; x++)
			{
				CvScalar s = getColor(x,y);
				char* element = row + x * image->nChannels;
				for(int i = 0; i < image->nChannels; i++)
				{
					element[i] = s.val[i] * 255;
				}
				
			}
		}
	}

	void drawHistogramm(IplImage* image, int channel, int range, double maximum, CvScalar scalar)
	{
		std::vector<double> hist(range);
		int validCount = 0;
		for(int y = 0; y < m_field.size1(); y++)
		{
			for(int x =0; x < m_field.size2(); x++)
			{
				if(m_linkingField(y,x) < std::numeric_limits<double>::epsilon())
					continue;
				validCount++;
				
				
				double v = m_field(y,x)[channel];
				int r = (int)(range * v);
				r = std::min(r,range-1);
				hist[r] ++;
			}
		}
		for(int i = 0; i < hist.size(); i++)
		{
			hist[i] /=validCount;
		}
		for(int i = 0; i < range-1; i++)
		{
			cvLine(image, cvPoint(i * image->width/(range-1),image->width - (int)(image->width * hist[i]/maximum)),
				cvPoint((i +1) * image->width/(range-1), image->width - (int)(image->width * hist[i+1]/maximum)),
				scalar);
		}
	}

private:
	static const int m_numParties = 4;
	static const int m_size = 300;
	static const int m_randomRange = 100;
	static const int m_maxNumOfAgitCenters = 10;
private:
	typedef Cell<m_numParties> Cell;
	typedef boost::numeric::ublas::matrix<Cell> Field;
	typedef boost::numeric::ublas::matrix<double> TransitionCoeffs;
	typedef std::vector<double> Histogramm;
	typedef std::deque<AgitationRecord> AgitationDeque;
private: // main functions

	void processAgitationField(int time)
	{
		Field& aggField = m_agitationField;
		while((!m_agitationDeque.empty()) && ( m_agitationDeque.begin()->time + 
			m_agitationDeque.begin()->period < time))
		{
			m_agitationDeque.pop_front();
		}
		if(m_agitationDeque.size() < m_maxNumOfAgitCenters)
		{
			addNewAgitationCenter(time);
		}
		for(int y = 0; y < (int)m_field.size1(); y++)
		{
			for(int x = 0; x < (int)m_field.size2(); x++)
			{
				std::fill(m_agitationField(y,x).begin(), m_agitationField(y,x).end(),0);
			}
		}


		std::for_each(m_agitationDeque.begin(), m_agitationDeque.end(),[&aggField](const AgitationRecord& rec)
		{
			
			
			
			for(int y = rec.center.y - rec.radius; y <= rec.center.y + rec.radius; y++)
			{
				for(int x = rec.center.x - rec.radius; x  <= rec.center.x + rec.radius; x++)
				{
					double dist = (x - rec.center.x) * (x - rec.center.x) + (y - rec.center.y) * (y - rec.center.y);
					if(dist > rec.radius * rec.radius)
						continue;
					if((std::min(x,y) < 0) || (x >= aggField.size2()) || (y >= aggField.size1()))
					{
						continue;
					}
					aggField(y,x)[rec.partNum] = rec.strength;
				}
			
			}
		});
	}


	void propagate()
	{
		for(int y = 0, ySize = m_field.size1(); y < ySize; y++)
		{
			for(int x = 0, xSize = m_field.size2(); x < xSize; x++)
			{
				Cell agitation = getAgitation(x,y,m_step);
				Cell neighborhood;
				neighborhood.assign(0);
				for(int yy = y-1; yy <= y + 1; yy++)
				{
					for(int xx = x-1; xx <= x+1; xx++)
					{
						if(std::min(yy,xx) < 0)
							continue;
						if((xx >= m_field.size2()) || (yy >= m_field.size1()))
							continue;
						neighborhood += (m_field(yy,xx) - m_field(y,x))*potential(m_field(yy,xx), m_field(y,x)) * 
							m_linkingField(y,x) * m_linkingField(yy,xx)*0.5; 
					}

				}
				m_tempField(y,x)  = m_field(y,x) +  (neighborhood  + agitation) * m_dt;
			}
		}
	}
	void renorm()
	{
		for(int y = 0; y < (int)m_field.size1(); y++)
		{
			for(int x = 0; x < (int)m_field.size2(); x++)
			{
				if(m_tempField(y,x).min() < 0) //value could be <0 due to Euler method inaccuracy
				{
					for(int i = 0; i < (int)m_tempField(y,x).size(); i++)
					{
						m_tempField(y,x)[i] = std::max<double>(m_tempField(y,x)[i],0.0); //force values to be >= 0
					}
				}
				m_field(y,x) = m_tempField(y,x)/std::max(std::numeric_limits<double>::epsilon(),Cell::norm(m_tempField(y,x)));
			}
		}
	}
	Cell getAgitation(int x, int y, int t) //random weight addon, should be changed to show real politic agitation, depending on the geography 
	{
		return m_agitationField(y,x);
	}
	Cell assignInitialWeight(int x, int y) //assign functions with random initial weights
	{
		Cell bates;
		for(int i = 0; i < m_numParties; i++)
		{
			bates[i] = rand()% m_randomRange  +1;
		}
		Cell result;
		result.assign(0.01);
		result[std::max_element(bates.begin(),bates.end())-bates.begin()] = 1.0;
		return result/Cell::norm(result);
	}
private: // service functions

	double potential(const Cell& a, const Cell& b) const
	{
		double delta= 1.0 - Cell::correlation(a,b);
		double neg = 1.0 / (delta*delta* m_decreasePotential + 1) - 0.5;
		double pos = exp(-delta * m_decreasePotential);
		return neg * pos;
	}
	 
    CvScalar getColor(int x, int y)
	{
		const Cell& cell = m_field(y,x);
		CvScalar result = {0};
		Cell out = cell;
		if(m_linkingField(y, x) < std::numeric_limits<double>::epsilon())
		{
			return cvScalar(1.0,1.0,1.0);
		}



		for(int i = 0;  i < m_numParties; i++)
		{
			for(int j  =0; j < 3; j++)
			{
				result.val[j] += m_colors[i].val[j] *cell[i];
			}
		}
		for(int j  =0; j < 3; j++)
		{
			result.val[j] = std::max(0.0,std::min(result.val[j],1.0));
		}
		return result;
	}
	
	void visualizeDispersionMap(IplImage* image)
	{
		for(int y = 0; y < image->height; y++)
		{
			char* row = image->imageData+ y * image->widthStep;
			
			for(int x = 0; x < image->width; x++)
			{
				double val = m_linkingField(y,x);
				char* element = row + x * image->nChannels;
				for(int i = 0; i < image->nChannels; i++)
				{
					element[i] = val * 255;
				}
				
			}
		}
	}
	void addNewAgitationCenter(int time)
	{
		AgitationRecord r = m_agitationWorks.generateRecord();
		r.center = cvPoint2D32f((double)(rand()% m_field.size2()), (double)(rand() % m_field.size1()));
		r.time = time;
		m_agitationDeque.push_back(r);
	}
private:

	IplImage*  m_output;
	IplImage* m_base;
	Field m_field;
	Field m_tempField;
	AgitationDeque m_agitationDeque;
	Field m_agitationField;
	TransitionCoeffs m_linkingField;
	std::vector<CvScalar> m_colors;
	int m_step;
	AgitationWorks m_agitationWorks;
private:
	const double m_dt;
	const double m_linkCoefficient;
	const double m_decreasePotential;
};






#endif 