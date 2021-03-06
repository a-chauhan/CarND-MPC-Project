#include <math.h>
#include <uWS/uWS.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "Eigen-3.3/Eigen/Core"
#include "Eigen-3.3/Eigen/QR"
#include "helpers.h"
#include "json.hpp"
#include "MPC.h"

// for convenience
using nlohmann::json;
using std::string;
using std::vector;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

int main() {
  uWS::Hub h;

  // MPC is initialized here!
  MPC mpc;

  h.onMessage([&mpc](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length,
                     uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    string sdata = string(data).substr(0, length);
    std::cout << sdata << std::endl;
    if (sdata.size() > 2 && sdata[0] == '4' && sdata[1] == '2') {
      string s = hasData(sdata);
      if (s != "") {
        auto j = json::parse(s);
        string event = j[0].get<string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          vector<double> ptsx = j[1]["ptsx"];
          vector<double> ptsy = j[1]["ptsy"];
          double px = j[1]["x"];
          double py = j[1]["y"];
          double psi = j[1]["psi"];
          double v = j[1]["speed"];

          /**
           * Steering angle and throttle calculation using MPC.
           * Both are in between [-1, 1].
           */
          double steer_value = j[1]["steering_angle"];
          double throttle_value = j[1]["throttle"];	

          Eigen::VectorXd xvals(ptsx.size());
          Eigen::VectorXd yvals(ptsy.size());

          for(int i = 0; i < ptsx.size(); i++) {
            double dx = ptsx[i] - px;
            double dy = ptsy[i] - py;

            xvals[i] = dx * cos(psi) + dy * sin(psi);
            yvals[i] = -dx * sin(psi) + dy * cos(psi);
          }

          auto coeffs = polyfit(xvals, yvals, 3);

          double cte = polyeval(coeffs, 0) - 0;
          double epsi = 0 - atan(coeffs[1]);

          const double latency_dt = 0.1;
          const double Lf = 2.67;

          // px = v * cos(psi) * latency_dt;
          // py = v * sin(psi) * latency_dt;
          // psi = v * delta / Lf * latency_dt;
          // v = v + a * latency_dt;
          // cte = cte + v * sin(epsi) * latency_dt;
          // epsi = epsi  - v * delta / Lf * latency_dt;

          double x_lat_adjust = 0, y_lat_adjust = 0,  psi_lat_adjust = 0, v_lat_adjust = v, cte_lat_adjust = cte, epsi_lat_adjust = epsi;	  
	        x_lat_adjust += v * cos(0) * latency_dt;
	        y_lat_adjust += v * sin(0) * latency_dt;
	        psi_lat_adjust += - v/Lf * steer_value * latency_dt;
	        v_lat_adjust += throttle_value * latency_dt;	    
	        cte_lat_adjust +=   v * sin(epsi_lat_adjust) * latency_dt;
	        epsi_lat_adjust += - v * steer_value / Lf * latency_dt;	 

          // Add everything to the state
          Eigen::VectorXd state(6);
          state << x_lat_adjust, y_lat_adjust, psi_lat_adjust, v_lat_adjust, cte_lat_adjust, epsi_lat_adjust;

          // Eigen::VectorXd state(6);
          // state << 0, 0, 0, v, cte, epsi;
          auto vars = mpc.Solve(state, coeffs);
          steer_value = vars[0] / deg2rad(25);
          throttle_value = vars[1];

          json msgJson;
          // NOTE: Remember to divide by deg2rad(25) before you send the 
          //   steering value back. Otherwise the values will be in between 
          //   [-deg2rad(25), deg2rad(25] instead of [-1, 1].
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = throttle_value;

          // Display the MPC predicted trajectory 
          vector<double> mpc_x_vals;
          vector<double> mpc_y_vals;

          /**
           * Add (x,y) points to list here, points are in reference to 
           *   the vehicle's coordinate system the points in the simulator are 
           *   connected by a Green line
           */

          for (int i = 2; i < vars.size(); i += 2) {
            mpc_x_vals.push_back(vars[i]);
            mpc_y_vals.push_back(vars[i+1]);
          }

          msgJson["mpc_x"] = mpc_x_vals;
          msgJson["mpc_y"] = mpc_y_vals;

          // Display the waypoints/reference line
          vector<double> next_x_vals;
          vector<double> next_y_vals;

          /**
           * TODO: add (x,y) points to list here, points are in reference to 
           *   the vehicle's coordinate system the points in the simulator are 
           *   connected by a Yellow line
           */

          for (int i = 0; i < xvals.size() - 1; i++) {
            next_x_vals.push_back(xvals[i]);
            next_y_vals.push_back(yvals[i]);
          }

          msgJson["next_x"] = next_x_vals;
          msgJson["next_y"] = next_y_vals;


          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl;
          // Latency
          // The purpose is to mimic real driving conditions where
          //   the car does actuate the commands instantly.
          //
          // Feel free to play around with this value but should be to drive
          //   around the track with 100ms latency.
          //
          // NOTE: REMEMBER TO SET THIS TO 100 MILLISECONDS BEFORE SUBMITTING.
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          // std::this_thread::sleep_for(std::chrono::milliseconds(50));
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }  // end "telemetry" if
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }  // end websocket if
  }); // end h.onMessage

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                         char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  
  h.run();
}