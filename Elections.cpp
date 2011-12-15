// Elections.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "elections_base.h"
#include <highgui.h>




int _tmain(int argc, _TCHAR* argv[])
{
	static const int baseNumParties = 4;
	
	Elections elections(cvSize(500,500));
	elections.loadDiffusionMap(cvLoadImage("c:\\work\\diffusion_map.bmp"));
	cvShowImage("mainWindow",elections.outputImage());
	cvWaitKey(0);
	elections.init();
	
	int f[baseNumParties] = {10,40,20,120};
	AgitationWorks::StrengthBorders s[baseNumParties] = {AgitationWorks::StrengthBorders(0,0.1), AgitationWorks::StrengthBorders(0,0.15), AgitationWorks::StrengthBorders(0,0.17),
	AgitationWorks::StrengthBorders(0,0.05)};
	double rs[baseNumParties] = {5, 7, 8, 17};
	int ps[baseNumParties] = {10,12, 30,12};
	std::vector<int> freqs(f, f+ baseNumParties);
	std::vector<AgitationWorks::StrengthBorders > strengths(s, s+ baseNumParties);
	std::vector<double> radiuses(rs,rs+ baseNumParties);
	std::vector<int> periods(ps,ps+ baseNumParties);
	AgitationWorks agit;
	agit.addBorders(freqs,strengths, radiuses, periods);
	elections.setAgigatationWorks(agit);



	for(int i = 0; i < 5000; i++)
	{
		for(int j = 0; j < 1; j++)
		{
			elections.processingStep();
		}
		elections.prepareImage();
		cvShowImage("mainWindow",elections.outputImage());
		cvWaitKey(2);
	}

	//cv::namedWindow("win");
	cvNamedWindow("mainWindow");
	CvScalar colors[4] = {cvScalar(255,0,255), cvScalar(0,255,0), cvScalar(0,0,255), cvScalar(255,255,0)};
	IplImage* image = cvCreateImage(cvSize(400,400),IPL_DEPTH_8U,3);
	cvFillImage(image,0);
	for(int i = 0; i < 4; i++)
	{
		elections.drawHistogramm(image,i,100,1.0,colors[i]);
	}
	cvShowImage("mainWindow",image);
	cvWaitKey(0);
	cvReleaseImage(&image);
	return 0;
}

