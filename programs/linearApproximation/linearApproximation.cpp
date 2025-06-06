//============================================================================
// Name        : linear approximation
// Author      : Vetrov D.N.
// Version     :
// Copyright   : 
// Description : Алгоритм расчёта линейной аппроксимации и коэффициента кореляции
// Buid        : g++ linearApproximation.cpp -o build
// Using       : ./build
//============================================================================

#include <iostream>
#include <cmath>
using namespace std;

// Постоянные параметры
float dataX[21] = {351, 362, 373, 389, 407, 425, 441, 454, 464, 477, 500, 515, 532, 549, 567, 578, 592, 608, 627, 646, 666};
float dataY[21] = {-1000, -900, -800, -700, -600, -500, -400, -300, -200, -100, 0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

int main() {
    float number_b = 0;
    float number_g = 0;
    float number_R = 0;
    int lenArray = sizeof(dataX)/sizeof(dataX[0]);
    
    float sum_x = 0;
    float sum_y = 0;
    float sum_xy = 0;
    float sum_xx = 0;
    for (int i=0;i<lenArray;i++) {
        sum_x = sum_x + dataX[i];
        sum_y = sum_y + dataY[i];
        sum_xy = sum_xy + dataX[i]*dataY[i];
        sum_xx = sum_xx + dataX[i]*dataX[i];
    }
    number_b = (lenArray*sum_xy-sum_x*sum_y)/(lenArray*sum_xx-sum_x*sum_x);
    number_g = (sum_xx*sum_y-sum_x*sum_xy)/(lenArray*sum_xx-sum_x*sum_x);
    
    float sum_xxyy = 0;
    float sum_sxx = 0;
    float sum_syy = 0;
    for (int i=0;i<lenArray;i++) {
        sum_xxyy = sum_xxyy + (dataX[i]-sum_x/lenArray)*(dataY[i]-sum_y/lenArray);
        sum_sxx = sum_sxx + (dataX[i]-sum_x/lenArray)*(dataX[i]-sum_x/lenArray);
        sum_syy = sum_syy + (dataY[i]-sum_y/lenArray)*(dataY[i]-sum_y/lenArray);
    }
    number_R = sum_xxyy/(sqrt(sum_sxx)*sqrt(sum_syy));
          
    cout << endl;
    cout << "n  X  Y"<< endl;
    for (int i=0;i<lenArray;i++) {
        cout << i+1 << ") " << dataX[i] << "    " << dataY[i] << ","  << endl;
    }
    
    cout << endl;
    cout << " Y(X) = bX + g" << "," << endl;
    cout << " b = " << number_b << "," << endl;
    cout << " g = " << number_g << "," << endl;
    cout << " R = " << number_R << "." << endl;
    
    return 0;
}
