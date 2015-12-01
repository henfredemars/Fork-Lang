#include <iostream>
#include <cmath>
#include <cstdint>

int64_t mod(int64_t x, int64_t y) {
	int64_t m = x/y;
	return x-m*y;
}

double distance(double x0, double y0, double x1, double y1) {
	double dx = x0 - x1;
	double dy = y0 - y1;
	double result = sqrt(dx*dx + dy*dy);
	return result;
}

void init_coords(double* coords, int64_t numberOfPoints, double delta) {
	if(numberOfPoints == 0) {
		return;
	}
	numberOfPoints = numberOfPoints - 1;
	coords[numberOfPoints] = delta*numberOfPoints;
	init_coords(coords, numberOfPoints, delta);
	return;
}

void compute_distances_from_center_sub(double* distances, double xcoord, double* ycoords,int64_t numberOfPointsY,int64_t offset) {
	if (numberOfPointsY==0) {
		return;
	}
	numberOfPointsY = numberOfPointsY-1;
	distances[offset] = distance(0.5,0.5,xcoord,ycoords[numberOfPointsY]);
	offset = offset+1;
	compute_distances_from_center_sub(distances,xcoord,ycoords,numberOfPointsY,offset);
	return;
}

void compute_distances_from_center(double* distances,double* xcoords,double* ycoords,int64_t numberOfPointsX,int64_t numberOfPointsY,int64_t callcount) {
	if (numberOfPointsX==0) {
		return;
	}
	numberOfPointsX = numberOfPointsX-1;
	int64_t offset = numberOfPointsY*callcount;
	compute_distances_from_center_sub(distances,xcoords[numberOfPointsX],ycoords,numberOfPointsY,offset);
	callcount = callcount+1;
	compute_distances_from_center(distances,xcoords,ycoords,numberOfPointsX,numberOfPointsY,callcount);
	return;
}

int64_t count_less_than_one_half(double* distances,int64_t start,int64_t end) {
	if (start > end) {
	  return 0;
	}
	double val = distances[start];
	if (val < 0.5) {
	  return count_less_than_one_half(distances,start+1,end)+1;
	} else {
	  return count_less_than_one_half(distances,start+1,end);
	}
	return 0;
}

int main() {
	//config
	int64_t numberOfPointsX = 1200;
	int64_t numberOfPointsY = 1200;
	int64_t totalNumberOfPoints = numberOfPointsX*numberOfPointsY;
	double deltaX = 1.0/numberOfPointsX;
	double deltaY = 1.0/numberOfPointsY;

	//make coord arrays
	double* xcoords = (double*)calloc(numberOfPointsX, 8);
	double* ycoords = (double*)calloc(numberOfPointsY, 8);

	//init coord arrays
	init_coords(xcoords, numberOfPointsX, deltaX);
	init_coords(ycoords, numberOfPointsY, deltaY);

	//create array to hold distances
	double* distances = (double*)calloc(totalNumberOfPoints, 8);

	//compute distances
	compute_distances_from_center(distances,xcoords,ycoords,numberOfPointsX,
		numberOfPointsY,0);
	compute_distances_from_center(distances,xcoords,ycoords,numberOfPointsX,
		numberOfPointsY,0);
	compute_distances_from_center(distances,xcoords,ycoords,numberOfPointsX,
		numberOfPointsY,0);
	compute_distances_from_center(distances,xcoords,ycoords,numberOfPointsX,
		numberOfPointsY,0);

	//count number of points inside the unit circle
	int64_t count = count_less_than_one_half(distances, 0, totalNumberOfPoints - 1);
	std::cout << count << std::endl;

	//pi is roughly...
	std::cout << 4/(totalNumberOfPoints/(count*1.0)) << std::endl;

	//free coord arrays
	delete xcoords;
	delete ycoords;
	delete distances;

	return 0;
}