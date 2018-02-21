#include <HAL/Gamepad.pb.h>
#include <HAL/Gamepad/GamepadDevice.h>
#include <HAL/Car.pb.h>
#include <HAL/Car/CarDevice.h>
#include <thread>
//#include <signal.h>
#include <spirit/spirit.h>
#include <HAL/Posys/PosysDevice.h>
#include <iostream>
#include <fstream>
#include <cmath>


//hal::CarCommandMsg commandMSG;

//void GamepadCallback(hal::GamepadMsg& _msg) {
//  commandMSG.set_steering_angle(_msg.axes().data(0));
//  commandMSG.set_throttle_percent(_msg.axes().data(5)*20);
//}

double V(double th, double x, double y, double v, double p){
  return 0.190624486308*p + 1.96773363112*th*th + 3.63058416146*x*x + 3.52728977122*y*y + 1.97925944398*v*v - 1.19032956317*th*x + 0.139231847089*th*y + 0.199779394346*th*v - 0.0833258535883*x*y + 0.0545824013467*v*x + 1.75893785671*v*y;
}

double V_pure(double th, double x, double y, double v){
  return 1.96773363112*th*th + 3.63058416146*x*x + 3.52728977122*y*y + 1.97925944398*v*v - 1.19032956317*th*x + 0.139231847089*th*y + 0.199779394346*th*v - 0.0833258535883*x*y + 0.0545824013467*v*x + 1.75893785671*v*y;
}

double dV(double u_1, double u_2, double u_3, double th, double x, double y, double v, double p){
  double v_r = 2;
  double dth_r = 0;
  double v_t = v + v_r;
  double sin_th = (-0.1569*th*th*th+0.9977*th);
  double cos_th = (-0.4622*th*th+0.9959);
  double r = (2*1.96773363112*th -1.19032956317*x + 0.139231847089 *y + 0.199779394346*v)*(-2.9380*v_t*u_2 - dth_r*(1+u_3)) + 
     (2*3.63058416146*x - 1.19032956317*th -0.0833258535883*y + 0.0545824013467 *v)*(dth_r*(1+u_3)*y + v_t*-sin_th) + 
     (2*3.52728977122*y + 0.139231847089 *th -0.0833258535883*x + 1.75893785671*v)*(dth_r*(1+u_3)*(-x) + v_t*cos_th - v_r*(1+u_3)) + 
     (2*1.97925944398*v + 0.199779394346*th + 0.0545824013467 *x + 1.75893785671*y)*(u_1);
  double boundary = 1;
  double L = V(th, x, y, v, p);
  if( L > boundary )
    // r += 0.249999989812*(1+u_3);
    r += 0.190624486308*(1+u_3);
  return r;
}

double rem(double a, double b){
  double q = a / b;
  return a - ((int)q)*b;
}

double rem(double a, int b){
  double dec = a - (int) a;
  return ((int)a % b) + dec;
}

class Input{
  public: double u_1;
  public: double u_2;
  public: double u_3;
  public: int seg;
  public: Input(double u_1, double u_2, double u_3, int seg){
    this->u_1 = u_1;
    this->u_2 = u_2;
    this->u_3 = u_3;
    this->seg = seg;
  }
};

Input K(double th_t, double x_t, double y_t, double v_t, double p_t, int seg_prev, double u_1_prev, double u_2_prev, double u_3_prev){
  
  // double horizon = 1.5;
  // int seg = (int)(rem(p_t, horizon*4)/horizon);
  // if(seg != seg_prev)
  //   std::cout << "\n\n\nsegment " << seg << "\n\n\n" << std::endl;
  int seg = seg_prev;

  double p = p_t;//rem(p_t, horizon);

  double new_x = x_t;
  double new_y = y_t;
  double new_th = th_t;
  // if (seg == 1){
  //     new_x = y_t;
  //     new_y = -x_t;
  //     new_th = th_t-(SP_PI/2);
  // }
  // if (seg == 2){
  //     new_x = -x_t;
  //     new_y = -y_t;
  //     new_th = th_t+SP_PI;
  //     // std::cout << new_th << ":" << rem((new_th+SP_PI), (2*SP_PI))-SP_PI << std::endl;
  // }
  // if (seg == 3){
  //     new_x = -y_t;
  //     new_y = x_t;
  //     new_th = th_t+(SP_PI/2);
  // }
  new_th = rem((new_th+SP_PI), (2*SP_PI))-SP_PI;

  x_t = new_x;
  y_t = new_y;
  th_t = new_th;

  // std::cout << "state: " << th_t << ", " << x_t << ", " << y_t << ", " << v_t << ", " << p << std::endl;

  double dth_r = 0;
  double th_r = 0 - SP_PI/2;
  double v_r = 2;
  double x_r = -4 + 2*p;
  double y_r = 0;
  double th_d = th_t - th_r;
  double v_d  = v_t  - v_r;
  double x_d  = x_t  - x_r;
  double y_d  = y_t  - y_r;
  double th = th_d;
  double v = v_d;
  double x = cos(-th_r)*x_d-sin(-th_r)*y_d;
  double y = sin(-th_r)*x_d+cos(-th_r)*y_d;

  double L = V(th, x, y, v, p);
  std::cout << "L: " << L << std::endl;

  double sin_th = (-0.1569*th*th*th+0.9977*th);
  double cos_th = (-0.4622*th*th+0.9959);

  double boundary = 1;

  double  b_1 = (2*1.97925944398*v + 0.199779394346*th + 0.0545824013467 *x + 1.75893785671*y);
  double  b_2 = (2*1.96773363112*th -1.19032956317*x + 0.139231847089 *y + 0.199779394346*v)*(-2.9380*(v_t));
  double  b_3 = (2*1.96773363112*th -1.19032956317*x + 0.139231847089 *y + 0.199779394346*v)*(-2.9380*v_t*0 - dth_r*(1+0)) + 
     (2*3.63058416146*x - 1.19032956317*th -0.0833258535883*y + 0.0545824013467 *v)*(dth_r*(1+0)*y) + 
     (2*3.52728977122*y + 0.139231847089 *th -0.0833258535883*x + 1.75893785671*v)*(dth_r*(1+0)*(-x) - v_r*(1+0)) + 
     (2*1.97925944398*v + 0.199779394346*th + 0.0545824013467 *x + 1.75893785671*y)*(0);
  if (L > boundary){
    // b_3 += 0.249999989812;
    b_3 += 0.190624486308;
  }
  double a = (2*1.96773363112*th -1.19032956317*x + 0.139231847089 *y + 0.199779394346*v)*(-2.9380*v_t*0 - dth_r*(1+0)) + 
     (2*3.63058416146*x - 1.19032956317*th -0.0833258535883*y + 0.0545824013467 *v)*(dth_r*(1+0)*y + v_t*-sin_th) + 
     (2*3.52728977122*y + 0.139231847089 *th -0.0833258535883*x + 1.75893785671*v)*(dth_r*(1+0)*(-x) + v_t*cos_th - v_r*(1+0)) + 
     (2*1.97925944398*v + 0.199779394346*th + 0.0545824013467 *x + 1.75893785671*y)*(0);
  if (L > boundary){
    // a += 0.249999989812;
    a += 0.190624486308;
  }
  double beta = b_1*b_1 + b_2*b_2 + b_3*b_3;
  
  double u1_max = 4;
  double u1_min = -4;
  double u2_max = 1;
  double u2_min = -1;
  double u3_max = 9;
  double u3_min = -0.9;

  double beta_low = 0.1/2;
  double u_1 = 0;
  double u_2 = 0;
  double u_3 = 0;

  double dL = 0;
  double Lpure = V_pure(th, x, y, v);
  if (beta < beta_low){ // use default u if L is small
    u_1 = 0;
    u_2 = 0;
    u_3 = 0;
    dL = dV(u_1, u_2, u_3, th, x, y, v, p);
  }else if (beta < 2*beta_low && u_1_prev == 0 && u_2_prev == 0 && u_3_prev == 0){ // continue using default u
    u_1 = u_1_prev;
    u_2 = u_2_prev;
    u_3 = u_3_prev;
    dL = dV(u_1, u_2, u_3, th, x, y, v, p);
  }else if (Lpure < 0.5){
    u_1 = 0;
    u_2 = dth_r/(-2.9380*v_r);
    u_3 = 0;
    dL = dV(u_1, u_2, u_3, th, x, y, v, p);
  }else{
    double phi = 0;
    double dec_rate = 0;
    if (beta > beta_low){ // Sontage formula
      dec_rate = (0.5*sqrt(a*a+beta));
      phi = (dec_rate+a)/beta;
      u_1 = -b_1*phi;
      u_2 = -b_2*phi;
      u_3 = -b_3*phi;
      u_1 = (u_1>u1_max)?u1_max:((u_1<u1_min)?u1_min:u_1);
      u_2 = (u_2>u2_max)?u2_max:((u_2<u2_min)?u2_min:u_2);
      u_3 = (u_3>u3_max)?u3_max:((u_3<u3_min)?u3_min:u_3);
    }
    dL = dV(u_1, u_2, u_3, th, x, y, v, p);

    double thresh = -0.1*L;
    double coef = 1.1;
    while (dL > thresh){ // increase decrease rate size if L is not decreasing
      if(phi == 0 || beta < beta_low)
        break;
      dec_rate = coef*dec_rate;
      phi = (dec_rate + a)/beta;
      double u_1t = -b_1*phi; double u_2t = -b_2*phi; double u_3t = -b_3*phi;
      u_1t = (u_1t>u1_max)?u1_max:((u_1t<u1_min)?u1_min:u_1t);
      u_2t = (u_2t>u2_max)?u2_max:((u_2t<u2_min)?u2_min:u_2t);
      u_3t = (u_3t>u3_max)?u3_max:((u_3t<u3_min)?u3_min:u_3t);
      // std::cout << ";" << dec_rate << ", " << phi << ":" << u_1t << ", " << u_2t << ", " << u_3t << std::endl;
      double dL_t = dV(u_1t, u_2t, u_3t, th, x, y, v, p);
      if (dL_t > dL){
        if (L > boundary){
          std::cout << "error: dL increases... V:" << L << ", Beta:" << beta << std::endl;
        }
        break;
      }
      dL = dL_t; u_1 = u_1t; u_2 = u_2t; u_3 = u_3t;
      if ((u_1 == u1_min || u_1 == u1_max) && (u_2 == u2_min || u_2 == u2_max)  && (u_3 == u3_min || u_3 == u3_max)){
        break;
      }
    }
    // if (dL > thresh)
    //   std::cout << "dL :" << dL << std::endl;
  }
  return Input(u_1, u_2, u_3, seg);
}

int main(int argc, char** argv) {
  // connect to a gamepad
//  hal::Gamepad gamepad("gamepad:/");
//  gamepad.RegisterGamepadDataCallback(&GamepadCallback);

  // Initialize commands
//  commandMSG.set_steering_angle(0);
//  commandMSG.set_throttle_percent(0);

  // Create world setting object
  spSettings settings_obj;
  settings_obj.SetGuiType(spGuiType::GUI_PANGOSCENEGRAPH);
  settings_obj.SetPhysicsEngineType(spPhyEngineType::PHY_BULLET);

  // create the world object
  spirit spworld(settings_obj);

  // create a car with default values at car_param object
  spObjectHandle car_handle = spworld.objects_.CreateVehicle(spworld.car_param);
  spworld.gui_.AddObject(spworld.objects_.GetObject(car_handle));
  spAWSDCar& car = (spAWSDCar&) spworld.objects_.GetObject(car_handle);

  // create a flat ground with a box object
  spPose gnd_pose_ = spPose::Identity();
  gnd_pose_.translate(spTranslation(0,0,-0.5));
  spObjectHandle gnd_handle = spworld.objects_.CreateBox(gnd_pose_,spBoxSize(50,50,1),0,spColor(0,1,0));
  spworld.gui_.AddObject(spworld.objects_.GetObject(gnd_handle));

  // set friction coefficent of ground
  ((spBox&)spworld.objects_.GetObject(gnd_handle)).SetFriction(1);


  double x0 = -1;
  double y0 = -1;
  double th0 = -1.6268;
  double v0 = 1.43;
  double p0 = 0;

  double torque0 = 0; // range: -50, 50
  double turn0 = 1.2667; //SP_PI_QUART; // range: -pi/4, pi/4
  double dp0 = 0;

  std::ifstream in_file;
  in_file.open ("temp_files/sim_input.txt");
  in_file >> th0 >> x0 >> y0 >> v0 >> torque0 >> turn0;
  in_file.close();

  double v0_x = -v0*sin(th0);
  double v0_y = v0*cos(th0);

  // std::cout << "state: " << th0 << ", " << x0 << ", " << y0 << ", " << v0_x << ", " << v0_y << ", " << torque0 << ", " << turn0 << std::endl;
  
// int first_time = 1;
while(1){
  spState initState;
  initState.pose = spPose::Identity();
  initState.pose.translate(spTranslation(x0,y0,0.06));
  Eigen::AngleAxisd initRot(th0,Eigen::Vector3d::UnitZ());
  initState.pose.rotate(initRot);
  initState.linvel = spVector3(v0_x, v0_y, 0);

  car.SetState(initState);


  double p_t = p0;
  double u_1_prev = torque0;
  double u_2_prev = turn0;
  double u_3_prev = dp0;

  double tau = 0.01;

  int seg_prev = 0;
  while(p_t < 4) {
//    car.SetEngineMaxVel(commandMSG.throttle_percent());
//    car.SetFrontSteeringAngle(commandMSG.steering_angle());

    // set some constant values for engine velocity and front steering angle
    // Input
    
    Eigen::Matrix3d rotmat = car.GetPose().rotation();
    double x_t = -car.GetState().pose.translation()[1];
    double y_t = car.GetState().pose.translation()[0];
    double th_t = std::atan2(rotmat(1,0),rotmat(0,0)) + 2*SP_PI + 1*SP_PI/2;
    double v_t = car.GetState().linvel.norm();
    

    
    Input feedback = K(th_t, x_t, y_t, v_t, p_t, seg_prev, u_1_prev, u_2_prev, u_3_prev);

    double u_1 = feedback.u_1;
    double u_2 = feedback.u_2;
    double u_3 = feedback.u_3;
    int seg = feedback.seg;

    u_1_prev = u_1;
    u_2_prev = u_2;
    u_3_prev = u_3;
    seg_prev = seg;
    
    std::cout << "log:" << x_t << "\t,\t" << y_t << "\t,\t" << th_t << "\t,\t" << v_t << "\t,\t" << p_t << std::endl;
    std::cout << "inp:" << u_1 << "\t,\t" << u_2 << "\t,\t" << u_3 << std::endl;

    double torque = (u_1-0.3124)/13908;
    torque += 0.0002;
    double turn = atan(u_2);
    
    car.SetEngineMaxVel(100);
    car.SetEngineTorque(torque);
    car.SetFrontSteeringAngle(turn);
    // std::cout << "u: " << u_1 << ", " << u_2 << std::endl;
    // std::cout << "control: " << torque << ", " << turn << std::endl;
    // std::cout << "radius: " << sqrt(x_t*x_t + y_t*y_t) << std::endl;
    spworld.objects_.StepPhySimulation(tau);
    p_t += (1+u_3_prev)*tau;

    spworld.gui_.Iterate(spworld.objects_);
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(tau*1000*(1))));
  }
}
  return 0;
}