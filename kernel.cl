typedef struct {
	float3 position;		
	float3 view;			
	float3 up;			
	float2 resolution;	
	float2 fov;		
	float aperature;
	float focal;
} camera_t;

typedef struct {
	float3 origin;
	float3 dir;
} ray_t;

typedef struct {
	float radius;
	float3 position;
	float3 color;
	float3 emission;
} shape_t;

union Colour {
	float c;
	uchar4 components;
};

constant int SAMPLES = 12;
constant float EPSILON = 0.00005f;

static float get_random(unsigned int *s0,
						unsigned int *s1)
{
	*s0 = 36969 * ((*s0) & 65535) + ((*s0) >> 16);
	*s1 = 18000 * ((*s1) & 65535) + ((*s1) >> 16);
	unsigned int i = ((*s0) << 16) + (*s1);
	union {
		float f;
		unsigned int ui;
	} res;
	res.ui = (i & 0x007fffff) | 0x40000000;
	return (res.f - 2.0f) / 2.0f;
}

ray_t createRay(const int x_coord,
				const int y_coord,
				const int width,
				const int height,
				__constant camera_t* cam,
				const int* seed0,
				const int* seed1) {
	float3 rcamv = cam->view; rcamv = normalize(rcamv);
	float3 rendercamup = cam->up; rendercamup = normalize(rendercamup);
	float3 hAxis = cross(rcamv, rendercamup); hAxis = normalize(hAxis);
	float3 vAxis = cross(hAxis, rcamv); vAxis = normalize(vAxis);
	float3 middle = cam->position + rcamv;
	float3 horizontal = hAxis * tan(cam->fov.x * 0.5f * (M_PI_F / 180)); 
	float3 vertical   =  vAxis * tan(cam->fov.y * -0.5f * (M_PI_F / 180)); 
	unsigned int x = x_coord;
	unsigned int y = y_coord;
	int pixelx = x_coord; 
	int pixely = height - y_coord - 1;
	float sx = (float)pixelx / (width - 1.0f);
	float sy = (float)pixely / (height - 1.0f);
	float3 pointOnPlaneOneUnitAwayFromEye = middle + (horizontal * ((2 * sx) - 1)) + (vertical * ((2 * sy) - 1));
	float3 pointOnImagePlane = cam->position + ((pointOnPlaneOneUnitAwayFromEye - cam->position) * cam->focal);
	float3 aperturePoint;
	if (cam->aperature > 0.00001f) { 

		float random1 = get_random(seed0, seed1);
		float random2 = get_random(seed1, seed0);

		float angle = 2 * M_PI_F * random1;
		float distance = cam->aperature * sqrt(random2);
		float apertureX = cos(angle) * distance;
		float apertureY = sin(angle) * distance;

		aperturePoint = cam->position + (hAxis * apertureX) + (vAxis * apertureY);
	}
	else aperturePoint = cam->position;
	float3 apertureToImagePlane = pointOnImagePlane - aperturePoint; apertureToImagePlane = normalize(apertureToImagePlane); 
	ray_t ray;
	ray.origin = aperturePoint;
	ray.dir = apertureToImagePlane; 
	return ray;
}


float intersect_sphere(const shape_t* sphere,
					   const ray_t* ray)
{
	float3 rayToCenter = sphere->position - ray->origin;
	float b = dot(rayToCenter, ray->dir);
	float c = dot(rayToCenter, rayToCenter) - sphere->radius*sphere->radius;
	float disc = b * b - c;
	if (disc < 0.0f) return 0.0f;
	else disc = sqrt(disc);
	if ((b - disc) > EPSILON) return b - disc;
	if ((b + disc) > EPSILON) return b + disc;
	return 0.0f;
}

bool intersect_scene(__constant shape_t* spheres,
					 const ray_t* ray,
					 float* t,
					 int* sphere_id,
					 const int sphere_count)
{
	float max = 1e20f;
	*t = max;
	for (int i=0; i < sphere_count; i++)  {
		shape_t sphere = spheres[i];
		float hitdistance = intersect_sphere(&sphere, ray);
		if (hitdistance != 0.0f && hitdistance < *t) {
			*t = hitdistance;
			*sphere_id = i;
		}
	}
	return *t < max;
}

float3 path(__constant shape_t* spheres,
			const ray_t* camray,
			const int sphere_count,
			const int* seed0,
			const int* seed1)
{
	ray_t ray = *camray;
	float3 accum_color = (float3)(0.0f, 0.0f, 0.0f);
	float3 mask = (float3)(1.0f, 1.0f, 1.0f);
	int randSeed0 = seed0;
	int randSeed1 = seed1;
	for (int bounces = 0; bounces < 8; bounces++){
		float t;
		int hitsphere_id = 0;
		if (!intersect_scene(spheres, &ray, &t, &hitsphere_id, sphere_count))
			return accum_color += mask * (float3)(0.15f, 0.15f, 0.25f);
		shape_t hitsphere = spheres[hitsphere_id];
		float3 hitpoint = ray.origin + ray.dir * t;
		float3 normal = normalize(hitpoint - hitsphere.position);
		float3 normal_facing = dot(normal, ray.dir) < 0.0f ? normal : normal * (-1.0f);
		float rand1 = get_random(randSeed0, randSeed1) * 2.0f * M_PI_F;
		float rand2 = get_random(randSeed1, randSeed0);
		float rand2s = sqrt(rand2);
		float3 w = normal_facing;
		float3 axis = fabs(w.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
		float3 u = normalize(cross(axis, w));
		float3 v = cross(w, u);
		float3 newdir = normalize(u * cos(rand1)*rand2s + v*sin(rand1)*rand2s + w*sqrt(1.0f - rand2));
		ray.origin = hitpoint + normal_facing * EPSILON;
		ray.dir = newdir;
		accum_color += mask * hitsphere.emission;
		mask *= hitsphere.color;
		mask *= dot(newdir, normal_facing);
	}
	return accum_color;
}

__kernel void render(__constant shape_t* spheres,
					 const int width,
					 const int height,
					 const int sphere_count,
					 __global float3* out,
					 const int framenumber,
					 __constant const camera_t* cam,
					 float random0,
					 float random1,
					 __global float3* accumbuffer,
					 const int hashedframenumber)
{
	unsigned int id = get_global_id(0);
	unsigned int x_coord = id % width;
	unsigned int y_coord = id / width;
	unsigned int seed0 = x_coord * framenumber % 1000 + (random0 * 100);
	unsigned int seed1 = y_coord * framenumber % 1000 + (random1 * 100);
	float3 finalcolor = (float3)(0.0f, 0.0f, 0.0f);
	float invSamples = 1.0f / SAMPLES;
	for (unsigned int i = 0; i < SAMPLES; i++){
		ray_t camray = createRay(x_coord, y_coord, width, height, cam, &seed0, &seed1);
		finalcolor += path(spheres, &camray, sphere_count, &seed0, &seed1) * invSamples;
	}
	accumbuffer[id] += finalcolor;
	float3 tempcolor = accumbuffer[id] / framenumber;
	tempcolor = (float3)(
		clamp(tempcolor.x, 0.0f, 1.0f),
		clamp(tempcolor.y, 0.0f, 1.0f), 
		clamp(tempcolor.z, 0.0f, 1.0f));
	union Colour fcolor;
	fcolor.components = (uchar4)(
		(unsigned char)(tempcolor.x * 255),
		(unsigned char)(tempcolor.y * 255),
		(unsigned char)(tempcolor.z * 255),
		1);
	out[id] = (float3)(x_coord, y_coord, fcolor.c);
}