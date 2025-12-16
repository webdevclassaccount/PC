#pragma OPENCL EXTENSION cl_khr_fp64 : enable

typedef struct
{
    double x, y;
    double vx, vy;
    double mass;
} Body;

__kernel void compute_forces(
    __global Body *bodies,
    __global double *fx_out,
    __global double *fy_out,
    const int num_bodies,
    const double G)
{
    int i = get_global_id(0);
    
    if (i >= num_bodies) return;
    
    double fx = 0.0;
    double fy = 0.0;
    
    for (int j = 0; j < num_bodies; j++)
    {
        if (i != j)
        {
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;
            double distance = sqrt(dx * dx + dy * dy);
            
            if (distance > 0.0)
            {
                double force_magnitude = G * bodies[i].mass * bodies[j].mass / (distance * distance);
                fx += force_magnitude * dx / distance;
                fy += force_magnitude * dy / distance;
            }
        }
    }
    
    fx_out[i] = fx;
    fy_out[i] = fy;
}

__kernel void update_velocities( __global Body *bodies, __global double *fx, __global double *fy, const int num_bodies, const double dt)
{
    int i = get_global_id(0);
    
    if (i >= num_bodies) return;
    
    bodies[i].vx += fx[i] / bodies[i].mass * dt;
    bodies[i].vy += fy[i] / bodies[i].mass * dt;
}

__kernel void update_positions( __global Body *bodies, const int num_bodies, const double dt)
{
    int i = get_global_id(0);
    if (i >= num_bodies) return;
    
    bodies[i].x += bodies[i].vx * dt;
    bodies[i].y += bodies[i].vy * dt;
}
