
# include "my_viewer.h"
# include "sphere.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>
# include <fstream>
# include <algorithm>
# include <vector>
# include <cmath>
# include <time.h>
# include <iostream>
# include <string>

#define M_PI 3.141592654f
#define INFINITE 1e8
#define MAX_RAY_DEPTH 5

using namespace std;

int outputCounter;
vector<Sphere> mySpheres;
double startTime;
double endTime;

MyViewer::MyViewer(int x, int y, int w, int h, const char* l) : WsViewer(x, y, w, h, l)
{
	_nbut = 0;
	outputCounter = 1;
	add_ui();
	build_scene();
}

void MyViewer::add_ui()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel("", UiPanel::HorizLeft);
	p->add(new UiButton("View", sp = new UiPanel()));
	p->add(new UiButton("Regenerate Spheres", EvRegenerate));
	p->add(new UiButton("Ray Trace Spheres", EvRayTrace));
	p->add(new UiButton("Create GIF", EvCreateGIF));
	p->add(new UiButton("Exit", EvExit)); p->top()->separate();
}

/*
float mix(float a, float b, float mix) {
	return b * mix + a * (1 - mix);
}
*/

GsVec multVecs(GsVec a, GsVec b) {

	return GsVec(a.x * b.x, a.y * b.y, a.z * b.z);

}

GsVec traceRay(GsVec rayOrigin, GsVec rayDirection, vector<Sphere> mySpheres, int depth) {

	// Find the object that's closest to the screen. (First object that the ray hits)

	float tnear = INFINITE;
	Sphere* hitSphere = NULL;

	// Finds intersection of closest sphere
	for (unsigned i = 0; i < mySpheres.size(); i++) {

		float t0 = INFINITE;
		float t1 = INFINITE;

		if (mySpheres[i].intersect(rayOrigin, rayDirection, t0, t1)) {

			if (t0 < tnear) {
				tnear = t0;
				hitSphere = &mySpheres[i];
			}

		}

	}

	// No intersection, return default color (background)
	if (!hitSphere) {
		return GsVec(0.8f, 0.8f, 0.8f);
	}

	GsVec surfaceColor(0.0f, 0.0f, 0.0f); // Color of intersected point on surface

	GsVec phit = rayOrigin + rayDirection*tnear; // Intersected point
	GsVec nhit = phit - hitSphere->center; // Normal of intersected point
	nhit.normalize();

	/*

	GsVec2 tex;
	tex.x = (1 + atan2(nhit.z, nhit.x) / M_PI) * 0.5f;
	tex.y = acosf(nhit.y) / M_PI;

	float scale = 6.0f;
	float pattern = (float)((fmodf(tex.x * scale, 1.0f) > 0.5f) ^ (fmodf(tex.y * scale, 1.0f) > 0.5f));
	return max(0.0f, dot(nhit, -rayDirection)) * mix(hitSphere->surfaceColor, hitSphere->surfaceColor * 0.8f, pattern);

	*/

	float bias = (float)1e-4;
	bool inside = false; // Check if we're inside the object

	// If normal and ray direction are in the same direction, then we are inside the sphere.
	// (Normal isn't facing us, so the point isn't facing us)
	// Toggle inside bool and negate normal

	if (dot(rayDirection, nhit) > 0) {
		nhit = -nhit;
		inside = true;
	}

	// Transparency & Reflection Resursive Rays

	if ((hitSphere->transparency > 0 || hitSphere->reflection > 0) && depth < MAX_RAY_DEPTH) {
	
		// Used for some mixing effect

		// Change return bg color to 2.0f to use the fresnel effect
		// Also make light source 3.0f to use the fresnel effect

		// float facingratio = dot(-rayDirection, nhit);
		// float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1f);

		// Also use this for surface color:
		// surfaceColor = multVecs((reflection * fresneleffect + refraction * (1 - fresneleffect) * hitSphere->transparency), hitSphere->surfaceColor);
	

		GsVec reflection(0.0f, 0.0f, 0.0f); // Used for reflection

		if (hitSphere->reflection) {

			GsVec reflectionDirection = rayDirection - nhit * 2 * dot(rayDirection, nhit);
			reflectionDirection.normalize();
			reflection = traceRay(phit + nhit * bias, reflectionDirection, mySpheres, depth + 1);

		}
		

		GsVec refraction(0.0f, 0.0f, 0.0f); // Used for transparency

		if (hitSphere->transparency) {

			float ior = 1.1f;

			float eta;

			if (inside) {
				eta = ior;
			}
			else {
				eta = 1 / ior;
			}

			float cosi = dot(-nhit, rayDirection);
			float k = 1 - eta * eta * (1 - cosi * cosi);
			GsVec refractionDirection = rayDirection * eta + nhit * (eta * cosi - sqrt(k));
			refractionDirection.normalize();
			refraction = traceRay(phit - nhit * bias, refractionDirection, mySpheres, depth + 1);

		}


		surfaceColor = multVecs((reflection + refraction * hitSphere->transparency), hitSphere->surfaceColor);
	
	}

	// No Transparency or Reflection. No Recursive Rays needed

	else {

		for (unsigned i = 0; i < mySpheres.size(); i++) {

			// Find light source. Only light spheres have emission colors

			if (mySpheres[i].emissionColor.x > 0) {

				GsVec transmission(1.0f, 1.0f, 1.0f);
				GsVec lightDirection = mySpheres[i].center - phit;
				lightDirection.normalize();

				for (unsigned j = 0; j < mySpheres.size(); j++) {
					// Check for spheres in the way of the light source
					if (i != j) {

						float t0;
						float t1;
						if (mySpheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) {
							transmission = GsVec(0.0f, 0.0f, 0.0f);
							break;
						}

					}

				}

				surfaceColor += multVecs(multVecs(hitSphere->surfaceColor, transmission), max(float(0), dot(nhit, lightDirection)) * mySpheres[i].emissionColor);

			}

		}

	}

	return surfaceColor + hitSphere->emissionColor;

}

void rayTraceRender(vector<Sphere>& mySpheres) {

	float width = 1200;
	float height = 900;

	vector<GsVec> pixels((int)(width*height));
	float fov = 60;
	float aspectratio = width/float(height);
	float angle = tan(M_PI * 0.5f * fov / 180.0f);

	

	// For each pixel, trace a ray. Total pixels = width*height

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			float xPixel = (2 * ((j + 0.5f) / width) - 1) * angle * aspectratio;
			float yPixel = (1 - 2 * ((i + 0.5f) / height)) * angle;

			GsVec rayDirection(xPixel, yPixel, -1); // No z-coord
			rayDirection.normalize();

			GsVec initialRay(0.0f, 0.0f, 0.0f);
			pixels[(int)(width*i) + j] = traceRay(initialRay, rayDirection, mySpheres, 0);

		}
	}


	// Output Image as a PPM file

	string outputFileName = "./RayTraceTest" + to_string(outputCounter) + ".ppm";

	ofstream ofs(outputFileName, std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (int i = 0; i < width * height; i++) {
		ofs << (unsigned char)(min(float(1), pixels[i].x) * 255) <<
			(unsigned char)(min(float(1), pixels[i].y) * 255) <<
			(unsigned char)(min(float(1), pixels[i].z) * 255);
	}
	ofs.close();

	outputCounter++;

}

void MyViewer::build_scene ()
{
	rootg()->remove_all();
	mySpheres.clear();

	time_t* t = new time_t;

	srand((int)_time64(t));
	
	// cout << rand() % 100;

	mySpheres.push_back(Sphere(GsVec(0.0f, -10004.0f, -20.0f), 10000, GsVec(0.40f, 0.40f, 0.40f), GsVec(0.0f, 0.0f, 0.0f), 0.0f, 0.0f));
	/*
	mySpheres.push_back(Sphere(GsVec(0.0f, 0.0f, -20.0f), 4, GsVec(1.00f, 0.32f, 0.36f), GsVec(0.0f, 0.0f, 0.0f), 1.0f, 0.5f));
	mySpheres.push_back(Sphere(GsVec(5.0f, -1.0f, -15.0f), 2, GsVec(0.90f, 0.76f, 0.46f), GsVec(0.0f, 0.0f, 0.0f), 1.0f, 0.0f));
	mySpheres.push_back(Sphere(GsVec(5.0f, 0.0f, -25.0f), 3, GsVec(0.65f, 0.77f, 0.97f), GsVec(0.0f, 0.0f, 0.0f), 1.0f, 0.0f));
	mySpheres.push_back(Sphere(GsVec(-5.5f, 0.0f, -15.0f), 3, GsVec(0.90f, 0.90f, 0.90f), GsVec(0.0f, 0.0f, 0.0f), 1.0f, 0.0f));
	*/

	GsVec randPosition;
	float randRadius = 0.0f;
	GsVec randSurfaceColor;
	GsVec randEmissionColor = GsVec(0.0f, 0.0f, 0.0f);
	float randReflect = 1.0f;
	float randTransparent = 0.5f;

	for (int i = 0; i < 7; i++) {

		randPosition = GsVec(-20.0f + (float)(rand()) / ((float)(RAND_MAX / (20.0f - (-20.0f)))), -1.0f + (float)(rand()) / ((float)(RAND_MAX / (1.0f - (-1.0f)))), -35.0f + (float)(rand()) / ((float)(RAND_MAX / (-15.0f - (-35.0f)))));
		randRadius = (float)(rand() % 4) + 1;
		randSurfaceColor = GsVec((float)(rand()) / ((float)(RAND_MAX)), (float)(rand()) / ((float)(RAND_MAX)), (float)(rand()) / ((float)(RAND_MAX)));

		//cout << randSurfaceColor.x << " " << randSurfaceColor.y << " " << randSurfaceColor.z << endl;

		mySpheres.push_back(Sphere(randPosition, randRadius, randSurfaceColor, randEmissionColor, randReflect, randTransparent));

	}

	Sphere lightSphere(GsVec(0.0f, 20.0f, -40.0f), 3.0f, GsVec(0.0f, 0.0f, 0.0f), GsVec(1.0f, 1.0f, 1.0f), 0.0f, 0.0f);

	mySpheres.push_back(lightSphere);

	SnPrimitive* p;

	for (unsigned i = 1; i < mySpheres.size(); i++) {
		p = new SnPrimitive(GsPrimitive::Sphere, mySpheres[i].radius);
		p->prim().material.diffuse = GsColor(mySpheres[i].surfaceColor.x, mySpheres[i].surfaceColor.y, mySpheres[i].surfaceColor.z);
		add_model(p, mySpheres[i].center);
	}

	
}

void MyViewer::add_model(SnShape* s, GsVec p)
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation(p);
	manip->initial_mat(m);

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	rootg()->add(manip);
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	
	case EvRegenerate: build_scene(); return 1;
	case EvRayTrace:

		startTime = gs_time(); 
		rayTraceRender(mySpheres);
		endTime = gs_time();
		cout << "Runtime: " << endTime - startTime << endl; 
		return 1;

	case EvCreateGIF: 
		
		for (int i = 0; i < 40; i++) {

			rayTraceRender(mySpheres);
			mySpheres[mySpheres.size() - 1].center.roty(2 * M_PI / 40.0f);

		}

		return 1;

	case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
