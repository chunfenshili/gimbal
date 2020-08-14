
#include  "kalman.h"

/*	
	Q:����������Q���󣬶�̬��Ӧ��죬�����ȶ��Ա仵
	R:����������R���󣬶�̬��Ӧ�����������ȶ��Ա��	
*/

// һά  model Xn+1 =  Xn + w*dt	
void KalmanInit(tKalman* Kalman,float q,float r)
{
  
  Kalman->fQ = q;
  Kalman->fR = r;
  Kalman->fPLast = q;
  
}
float  KalmanFilter(tKalman* Kalman,float mX,float U)
{
  double fPNew = Kalman->fPLast+ Kalman->fQ;//P(K-1|K) = F(K-1)*P(k|k-1)F(K-1)'
  double K = fPNew /(fPNew +Kalman->fR);
  double X =  Kalman->fXLast+  U;
  double eX = X+K*(mX-X);
  Kalman->fXLast = eX;
  Kalman->fPLast = (1.f-K)*fPNew;
  return X;


}



