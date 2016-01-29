__device__ __constant__ dragForceCoefficient = 0;
__device__ __constant__ ballMass = 0;
__device__ __constant__ ballRadius = 0;
__device__ __constant__ ballR3 = (ballRadius * ballRadius * ballRadius);
__device__ __constant__ pi = ;
__device__ __constant__ piSquared = pi*pi;

typedef struct {
  double2 pos;
  double2 vel;
  
  double rot;
  double rotVel;
} cuda_physState;

typedef struct {
  double2 vel;
  double2 acc;
  
  double rotVel;
  double rotAcc;
} cuda_physDeriv;

__device__ dragAcc(cuda_physState* inState, cuda_physDeriv* out) {
  double2 outAcc;
  outAcc.x = ((inState.vel.x * inState.vel.x) * dragForceCoefficient) / mass;
  outAcc.y = ((inState.vel.y * inState.vel.y) * dragForceCoefficient) / mass;
  
  out->acc.x -= (out->acc.x > 0) ? outAcc.x : -outAcc.x;
  out->acc.y -= (out->acc.y > 0) ? outAcc.y : -outAcc.y;
}

__device__ spinLiftAcc(cuda_physState* inState, cuda_physDeriv* out) {
  double2 outAcc;
  outAcc.x = (inState.vel.x * airDensity * (inState.rotVel / 360.0) * ballR3 * piSquared * 16) / 3.0;
  outAcc.y = (inState.vel.y * airDensity * (inState.rotVel / 360.0) * ballR3 * piSquared * 16) / 3.0;
  
  out.acc.x += outAcc.x / ballMass;
  out.acc.y += outAcc.y / ballMass;
}

__device__ gravAcc(cuda_physDeriv* out) {
  out.acc.y -= 9.81;
}

