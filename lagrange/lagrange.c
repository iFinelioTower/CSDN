#include <algorithm.h>

double Lagrange(double x[], double y[], int n, double t)
{
   
	double s;
	double result = 0.0;

	if (n < 1) {
		goto exit;
	} else if (n == 1) {
	    result = y[0];
		goto exit;
	} else if (n == 2) {
		result = (y[0] * (t - x[1]) - y[1] *( t - x[0])) / (x[0] - x[1]);
		goto exit;
	} else {		
		int i, j, k, m;
		double s;
		
		i = 0;
		while ((x[i] < t) && (i < n))
			i = i + 1;

		k = i - 4;
		if (k < 0) 
			k = 0;

		m = i + 3;
		if (m > n - 1) 
			m = n - 1;

		for (i = k; i <= m; i++) {
			s = 1.0;
			for (j = k; j <= m; j++) {
				if (j != i) {
					s = s * (t - x[j]) / (x[i] - x[j]);
				}
			}

			result = result + (s * y[i]);
		}
	}

exit:
	return result;	
}
