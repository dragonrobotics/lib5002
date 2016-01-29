__device__ __constant__ double dragForceCoefficient = 0;
__device__ __constant__ double ballMass = 0;
__device__ __constant__ double ballRadius = 0;
__device__ __constant__ double ballR3 = (ballRadius * ballRadius * ballRadius);
__device__ __constant__ double pi = ;
__device__ __constant__ double piSquared = pi*pi;

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

__device__ __constant__ cuda_physDeriv emptyDeriv;

__device__ void dragAcc(cuda_physState* inState, cuda_physDeriv* out) {
  double2 outAcc;
  outAcc.x = ((inState->vel.x * inState->vel.x) * dragForceCoefficient) / mass;
  outAcc.y = ((inState->vel.y * inState->vel.y) * dragForceCoefficient) / mass;
  
  out->acc.x -= (out->acc.x > 0) ? outAcc.x : -outAcc.x;
  out->acc.y -= (out->acc.y > 0) ? outAcc.y : -outAcc.y;
}

__device__ void spinLiftAcc(cuda_physState* inState, cuda_physDeriv* out) {
  double2 outAcc;
  outAcc.x = (inState->vel.x * airDensity * (inState->rotVel / 360.0) * ballR3 * piSquared * 16) / 3.0;
  outAcc.y = (inState->vel.y * airDensity * (inState->rotVel / 360.0) * ballR3 * piSquared * 16) / 3.0;
  
  out->acc.x += outAcc.x / ballMass;
  out->cc.y += outAcc.y / ballMass;
}

__device__ void gravAcc(cuda_physDeriv* out) {
  out->acc.y -= 9.81;
}

__device__ void derive(cuda_physState* inState, cuda_physDeriv *inDeriv, cuda_physDeriv* out, double delta) {
  cuda_physState tmp;
  tmp.pos.x = inState->pos.x + (inDeriv->vel.x * delta);
  tmp.pos.y = inState->pos.y + (inDeriv->vel.y * delta);
  
  tmp.vel.x = inState->vel.x + (inDeriv->acc.x * delta);
  tmp.vel.y = inState->vel.y + (inDeriv->acc.y * delta);
  
  tmp.rot = inState->rot + (inDeriv->rot * delta);
  tmp.rotVel = inState->rotVel + (inDeriv->rotVel * delta);
  
  out = new cuda_physDeriv();
  
  out->vel = tmp.vel;
  out->rotVel  = tmp.rotVel;
  
  dragAcc(&tmp, out);
  spinLiftAcc(&tmp, out);
  gravAcc(out);
}

__global__ void integrate(cuda_physState* in, cuda_physState* out, double delta) {
  cuda_physDeriv* a,b,c,d;
  
  derive(in, &emptyDeriv, a, delta);
  derive(in, a, b, delta/2.0);
  derive(in, b, c, delta/2.0);
  derive(in, c, d, delta);
  
  double2 finPos;
  double2 finVel;
  
  finPos.x = (a->vel.x + 2.0*(b->vel.x + c->vel.x) + d->vel.x) / 6.0;
  finPos.y = (a->vel.y + 2.0*(b->vel.y + c->vel.y) + d->vel.y) / 6.0;
  
  finVel.x = (a->acc.x + 2.0*(b->acc.x + c->acc.x) + d->acc.x) / 6.0;
  finVel.y = (a->acc.y + 2.0*(b->acc.y + c->acc.y) + d->acc.y) / 6.0;
  
  double finRot = (a->rotVel + 2.0*(b->rotVel + c->rotVel) + d->rotVel) / 6.0;
  double finRotVel = (a->rotAcc + 2.0*(b->rotAcc + c->rotAcc) + d->rotAcc) / 6.0;
  
  out->pos.x = in->pos.x + (finPos.x * delta);
  out->pos.y = in->pos.y + (finPos.y * delta);
  
  out->vel.x = in->vel.x + (finVel.x * delta);
  out->vel.y = in->vel.y + (finVel.y * delta);
  
  out->rot = in->rot + (finRot * delta);
  out->rotVel = in->rotVel + (finRotVel * delta);
}
