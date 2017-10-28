#include <string>
#include <iostream>
#include <fstream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

int main (int argc, char** argv) {
	pcl::PointCloud<pcl::PointXYZ> cloud;

	// Fill in the cloud data


	std::ifstream inFile(argv[1], std::ifstream::in);


	cloud.width    = 1;
	cloud.height   = 1;
	cloud.is_dense = false;

	std::string line;
	float theta;
	int h,r,i = 0;
	
	while(std::getline(inFile,line))
	{
		std::stringstream ss(line);
		cloud.points.resize(cloud.width * cloud.height);
		ss >> theta >> h >> r;
		cloud.points[i].x = r*cos(theta);
		cloud.points[i].y = r*sin(theta);
		cloud.points[i].z = h;
		cloud.width = (++i) + 1;
	}
	cloud.width -= 1;
	
	pcl::io::savePCDFileASCII ("test_pcd.pcd", cloud);
	std::cerr << "Saved " << cloud.points.size() << " data points to test_pcd.pcd." << std::endl;

	for (size_t i = 0; i < cloud.points.size (); ++i)
		std::cerr << "    " << cloud.points[i].x << " " << cloud.points[i].y << " " << cloud.points[i].z << std::endl;

	return (0);
}
