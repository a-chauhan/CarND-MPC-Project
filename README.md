# CarND-Controls-MPC
Self-Driving Car Engineer Nanodegree Program

---

## Dependencies

* cmake >= 3.5
 * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1(mac, linux), 3.81(Windows)
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools]((https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)
* [uWebSockets](https://github.com/uWebSockets/uWebSockets)
  * Run either `install-mac.sh` or `install-ubuntu.sh`.
  * If you install from source, checkout to commit `e94b6e1`, i.e.
    ```
    git clone https://github.com/uWebSockets/uWebSockets
    cd uWebSockets
    git checkout e94b6e1
    ```
    Some function signatures have changed in v0.14.x. See [this PR](https://github.com/udacity/CarND-MPC-Project/pull/3) for more details.

* **Ipopt and CppAD:** Please refer to [this document](https://github.com/udacity/CarND-MPC-Project/blob/master/install_Ipopt_CppAD.md) for installation instructions.
* [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page). This is already part of the repo so you shouldn't have to worry about it.
* Simulator. You can download these from the [releases tab](https://github.com/udacity/self-driving-car-sim/releases).
* Not a dependency but read the [DATA.md](./DATA.md) for a description of the data sent back from the simulator.


## Basic Build Instructions

1. Clone this repo.
2. Make a build directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./mpc`.

## MPC
Model predictive controller to drive the vehicle

## Model
x - x car's position x
y - y car's position y
psi - orientation
v - velocity
cte - Cross Track Error
epsi - Psi Error

Two actuators are:
delta - steering angle
a - acceleration

## Equations:
x[t+1] = x[t] + v[t] * cos(psi[t]) * dt
y[t+1] = y[t] + v[t] * sin(psi[t]) * dt
psi[t+1] = psi[t] + v[t] / Lf * delta[t] * dt
v[t+1] = v[t] + a[t] * dt
cte[t+1] = f(x[t]) - y[t] + v[t] * sin(epsi[t]) * dt
epsi[t+1] = psi[t] - psides[t] + v[t] * delta[t] / Lf * dt

## Timestep Length and duration

N - the number of timesteps
dt - the time elapsed between actuations

N values are determined based on our use case, large values mean more horizon, will increase computation, and can change by the car's behaviour. It is to be noted that larger N does not mean, the car will follow the exact, but actually, it updates the path dynamically at every time stamp dt. Also it should not be small enough so that the car needs can see just few steps in the future. Also, latency will be increased if N is larger, not favourable.

dt - dt is the time after wich car will update its path, it should be good enough, not too large, not too small. Larger values mean a big differency between the predicted path points and the real path points because of several circumstances, forces. If it is too small, it will do unnecessary computations.

N * dt: the time horizon predict the path in the future with certain assumptions and condition. It will give us a time frame car can move being on the track.
Since Speed = distance / time, larger time horizon mean (N * dt) = distance/speed, car can travel at high speed with greater certainity about the trajectory points and will remain on the track.

## Polynomial and MPC:
data provided by simulator, first converted to the car system co-ordinates and then fitted into a polynomial of the 3rd degree.

## Latency handling
At slow speed it does not have that much effect, but at higher speeds the vehicle becomes a little bit unstable. I tried to tune the various parameters so that it does not overshoot the track. And giving more weight to the cte error rather than speed. So, car may slow down to avoid the track overshoot.

Also, to handle the latency, the recommed way is to predict the state in future, say 100 ms. Implementation is done by predicting the state at 100ms in future.

## Result
The car was able to complete the track.

## Link
youtube simulation video link: https://youtu.be/6U-wcRKylds