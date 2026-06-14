#include<cmath>
#include<Eigen/Core>
#include<Eigen/Dense>
#include<iostream>

using namespace std;

int main() {

	Eigen::Vector3f p(2.0f, 1.0f, 1.0f), res;
	Eigen::Matrix3f m1, m2;
	double angle = EIGEN_PI / 4.0;

	m1 << cos(angle), -sin(angle), 0,
		  sin(angle), cos(angle), 0,
		  0, 0, 1;

    m2 << 1, 0, 1,
    	  0, 1, 2,
    	  0, 0, 1;
    
    res = m2 * m1 * p;
    cout << res << endl;

	return 0;
}
